#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
using namespace sf;
using namespace std;

// ---------------- CONSTANTS ----------------
const int SCREEN_X = 1136;
const int SCREEN_Y = 896;
const int CELL_SIZE = 64;
const int LEVEL_HEIGHT = 14;
const int LEVEL_WIDTH = 18;

// ---------------- FUNCTION DECLARATIONS ----------------
void load_level1(char** lvl);
void load_level2(char** lvl);
void display_level(RenderWindow& window, char** lvl, Sprite& bgSprite, Sprite& blockSprite);
void apply_gravity(char** lvl, float& player_x, float& player_y, float& velocityY, bool& onGround,
                   float gravity, float terminal_Velocity, int PlayerHeight, int PlayerWidth);
void show_game_over(RenderWindow& window, Font& font);
void show_level_complete(RenderWindow& window, Font& font);

// ============================================================
int main() {
    RenderWindow window(VideoMode(SCREEN_X, SCREEN_Y), "Tumble-POP", Style::Close);
    window.setFramerateLimit(60);

    // ---------------- VARIABLES ----------------
    float player_x = 500, player_y = 150;
    float velocityY = 0;
    float speed = 5;
    const float jumpStrength = -20;
    const float gravity = 1.0f;
    const float terminal_Velocity = 20;
    bool onGround = false;
    int PlayerHeight = 102, PlayerWidth = 96;
    int health = 3, score = 0, level = 1;
    bool gameOver = false, levelComplete = false;
    float speedMultiplier = 1.0f;
    float vacuumPower = 1.0f;

    // ---------------- FONT ----------------
    Font font;
    if (!font.loadFromFile("Data/font.ttf"))
        cout << "⚠️ font.ttf missing\n";

    Text infoText("", font, 30);

    infoText.setFillColor(Color::White);
    infoText.setPosition(20, 20);

    // ---------------- MENU SCREEN ----------------
    Text title("TUMBLE-POP", font, 80);
    title.setFillColor(Color::Yellow);
    title.setPosition(250, 200);

    Text subtitle("Press 1 for Yellow (Fast) | Press 2 for Green (Strong Vacuum)", font, 30);
    subtitle.setFillColor(Color::White);
    subtitle.setPosition(120, 400);

    bool characterSelected = false;
    while (window.isOpen() && !characterSelected) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();
            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Num1) {
                    speedMultiplier = 1.5f;
                    vacuumPower = 1.0f;
                    characterSelected = true;
                }
                if (e.key.code == Keyboard::Num2) {
                    speedMultiplier = 1.0f;
                    vacuumPower = 1.2f;
                    characterSelected = true;
                }
            }
        }
        window.clear(Color::Black);
        window.draw(title);
        window.draw(subtitle);
        window.display();
    }

    if (!characterSelected) return 0;

    // ---------------- LOAD ASSETS ----------------
    Texture bgTex, blockTex, playerTex, ghostTex;
    bgTex.loadFromFile("Data/bg.png");
    blockTex.loadFromFile("Data/block1.png");
    playerTex.loadFromFile("Data/player.png");
    if (!ghostTex.loadFromFile("Data/ghost.png")) ghostTex = playerTex;

    Sprite bgSprite(bgTex), blockSprite(blockTex), playerSprite(playerTex), ghostSprite(ghostTex);
    playerSprite.setScale(3,3);
    ghostSprite.setScale(3,3);

    // Music & Sound
    Music music;
    if (music.openFromFile("Data/mus.ogg")) {
        music.setLoop(true);
        music.setVolume(30);
        music.play();
    }

    SoundBuffer suckBuf, shootBuf, hitBuf;
    Sound suckSound, shootSound, hitSound;
    if (suckBuf.loadFromFile("Data/suck.wav")) suckSound.setBuffer(suckBuf);
    if (shootBuf.loadFromFile("Data/shoot.wav")) shootSound.setBuffer(shootBuf);
    if (hitBuf.loadFromFile("Data/hit.wav")) hitSound.setBuffer(hitBuf);

    // Level
    char** lvl = new char*[LEVEL_HEIGHT];
    for (int i=0;i<LEVEL_HEIGHT;i++) lvl[i]=new char[LEVEL_WIDTH];
    load_level1(lvl);

    // Enemies
    const int NUM_GHOSTS = 6;
    float ghost_x[NUM_GHOSTS] = {200,600,800,300,900,500};
    float ghost_y[NUM_GHOSTS] = {700,700,700,400,400,500};
    int ghost_dir[NUM_GHOSTS] = {1,-1,1,-1,1,-1};
    bool ghost_alive[NUM_GHOSTS] = {true,true,true,true,true,true};

    // Vacuum
    int capturedEnemies = 0;
    bool ghost_captured[NUM_GHOSTS] = {false};
    const int maxCapacity = 3;

    // Projectiles
    bool projectile_active[NUM_GHOSTS] = {false};
    float projectile_x[NUM_GHOSTS];
    float projectile_y[NUM_GHOSTS];
    int projectile_dir[NUM_GHOSTS];

    // ---------------- MAIN LOOP ----------------
    while (window.isOpen()) {
        bool pressSpace = false, pressQ = false, pressE = false;

        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Escape)
                    window.close();
                if (e.key.code == Keyboard::Space)
                    pressSpace = true;
                if (e.key.code == Keyboard::Q)
                    pressQ = true;
                if (e.key.code == Keyboard::E)
                    pressE = true;
                if (e.key.code == Keyboard::Up && onGround) {
                    velocityY = jumpStrength;
                    onGround = false;
                }
            }
        }

        if (!gameOver && !levelComplete) {
            // MOVEMENT
            if (Keyboard::isKeyPressed(Keyboard::Left)) player_x -= speed * speedMultiplier;
            if (Keyboard::isKeyPressed(Keyboard::Right)) player_x += speed * speedMultiplier;

            // GRAVITY
            apply_gravity(lvl, player_x, player_y, velocityY, onGround, gravity, terminal_Velocity, PlayerHeight, PlayerWidth);
            playerSprite.setPosition(player_x, player_y);

            // ENEMY BEHAVIOR
            for (int i=0;i<NUM_GHOSTS;i++) {
                if (!ghost_alive[i]) continue;
                ghost_x[i]+=ghost_dir[i]*2;
                char next = lvl[(int)ghost_y[i]/CELL_SIZE][(int)(ghost_x[i]+48*ghost_dir[i])/CELL_SIZE];
                if (next=='#') ghost_dir[i]*=-1;
                float dx=fabs((player_x+48)-(ghost_x[i]+48));
                float dy=fabs((player_y+48)-(ghost_y[i]+48));
                if (dx<64 && dy<80) {
                    health--;
                    ghost_alive[i]=false;
                    if (hitSound.getBuffer()) hitSound.play();
                    if (health<=0) gameOver=true;
                }
            }

            // VACUUM
            if (pressSpace) {
                for (int i=0;i<NUM_GHOSTS;i++) {
                    if (ghost_alive[i] && capturedEnemies<maxCapacity) {
                        float dx=fabs((player_x+48)-(ghost_x[i]+48));
                        float dy=fabs((player_y+48)-(ghost_y[i]+48));
                        if (dx<150*vacuumPower && dy<150*vacuumPower) {
                            ghost_alive[i]=false; ghost_captured[i]=true;
                            capturedEnemies++; score+=50;
                            if (suckSound.getBuffer()) suckSound.play();
                        }
                    }
                }
            }

            // SHOOT ONE
            if (pressQ && capturedEnemies>0) {
                capturedEnemies--;
                for (int i=0;i<NUM_GHOSTS;i++) {
                    if (ghost_captured[i]) {
                        ghost_captured[i]=false;
                        projectile_active[i]=true;
                        projectile_x[i]=player_x+PlayerWidth/2;
                        projectile_y[i]=player_y+PlayerHeight/2;
                        projectile_dir[i]=1;
                        if (shootSound.getBuffer()) shootSound.play();
                        break;
                    }
                }
            }

            // BURST ALL
            if (pressE && capturedEnemies>0) {
                for (int i=0;i<NUM_GHOSTS;i++) {
                    if (ghost_captured[i]) {
                        ghost_captured[i]=false;
                        projectile_active[i]=true;
                        projectile_x[i]=player_x+PlayerWidth/2;
                        projectile_y[i]=player_y+PlayerHeight/2;
                        projectile_dir[i]=1;
                    }
                }
                capturedEnemies=0;
                score+=300;
                if (shootSound.getBuffer()) shootSound.play();
            }

            // PROJECTILES
            for (int i=0;i<NUM_GHOSTS;i++) {
                if (projectile_active[i]) {
                    projectile_x[i]+=10*projectile_dir[i];
                    if (projectile_x[i]>SCREEN_X) projectile_active[i]=false;
                }
            }

            // LEVEL TRANSITION
            bool allDead=true;
            for (int i=0;i<NUM_GHOSTS;i++) if (ghost_alive[i]) allDead=false;

            if (allDead && level==1) {
                levelComplete=true;
                show_level_complete(window, font);
                load_level2(lvl);
                level=2;
                for (int i=0;i<NUM_GHOSTS;i++){
                    ghost_alive[i]=true;
                    ghost_x[i]=200+100*i;
                    ghost_y[i]=500-30*i;
                }
                player_x=400; player_y=200;
                levelComplete=false;
            }
            else if (allDead && level==2) {
                levelComplete=true;
                show_level_complete(window, font);
                gameOver=true;
            }
        }

        // RENDER
        window.clear();
        display_level(window, lvl, bgSprite, blockSprite);
        window.draw(playerSprite);

        for (int i=0;i<NUM_GHOSTS;i++) {
            if (ghost_alive[i]) {
                ghostSprite.setPosition(ghost_x[i], ghost_y[i]);
                window.draw(ghostSprite);
            }
            if (projectile_active[i]) {
                ghostSprite.setPosition(projectile_x[i], projectile_y[i]);
                window.draw(ghostSprite);
            }
        }

        infoText.setString("Level: "+to_string(level)+"  Score: "+to_string(score)+"  Health: "+to_string(health));
        window.draw(infoText);
        if (gameOver) show_game_over(window, font);
        window.display();
    }

    for (int i=0;i<LEVEL_HEIGHT;i++) delete[] lvl[i];
    delete[] lvl;
    return 0;
}

// ============================================================
//                  HELPER FUNCTIONS
// ============================================================
void display_level(RenderWindow& window, char** lvl, Sprite& bgSprite, Sprite& blockSprite) {
    window.draw(bgSprite);
    for (int i=0;i<LEVEL_HEIGHT;i++)
        for (int j=0;j<LEVEL_WIDTH;j++)
            if (lvl[i][j]=='#') {
                blockSprite.setPosition(j*CELL_SIZE, i*CELL_SIZE);
                window.draw(blockSprite);
            }
}

void load_level1(char** lvl) {
    for (int i=0;i<LEVEL_HEIGHT;i++)
        for (int j=0;j<LEVEL_WIDTH;j++)
            lvl[i][j]=' ';
    for (int j=0;j<LEVEL_WIDTH;j++)
        lvl[13][j]='#';
    lvl[7][7]='#'; lvl[7][8]='#'; lvl[7][9]='#';
}

void load_level2(char** lvl) {
    for (int i=0;i<LEVEL_HEIGHT;i++)
        for (int j=0;j<LEVEL_WIDTH;j++)
            lvl[i][j]=' ';
    for (int j=0;j<LEVEL_WIDTH;j++)
        lvl[13][j]='#';
    lvl[10][5]='#'; lvl[9][6]='#'; lvl[8][7]='#'; lvl[7][8]='#';
}

void apply_gravity(char** lvl, float& player_x, float& player_y, float& velocityY, bool& onGround,
                   float gravity, float terminal_Velocity, int PlayerHeight, int PlayerWidth) {
    float offset_y = player_y;
    offset_y += velocityY;
    int cellY = (int)(offset_y + PlayerHeight) / CELL_SIZE;
    int leftX = (int)(player_x) / CELL_SIZE;
    int rightX = (int)(player_x + PlayerWidth) / CELL_SIZE;
    char bottom_left  = lvl[cellY][leftX];
    char bottom_right = lvl[cellY][rightX];
    if (bottom_left=='#' || bottom_right=='#') {
        onGround=true; velocityY=0;
    } else {
        onGround=false;
        player_y=offset_y;
        velocityY+=gravity;
        if (velocityY>=terminal_Velocity) velocityY=terminal_Velocity;
    }
}

void show_game_over(RenderWindow& window, Font& font) {
    Text over("GAME OVER!\nPress ESC to Exit", font, 70);
    over.setFillColor(Color::Red);
    over.setPosition(250, 400);
    window.clear(Color::Black);
    window.draw(over);
    window.display();
    sf::sleep(seconds(3));
}

void show_level_complete(RenderWindow& window, Font& font) {
    Text comp("LEVEL COMPLETE!", font, 60);
    comp.setFillColor(Color::Green);
    comp.setPosition(300, 400);
    window.clear(Color::Black);
    window.draw(comp);
    window.display();
    sf::sleep(seconds(2));
}

