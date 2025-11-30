// butt saab
// collision detection closes the window
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

using namespace sf;
using namespace std;

int screen_x = 1200;
int screen_y = 950;

// function to display animation frame based on player state
void updatePlayerAnimation(Sprite &PlayerSprite, int facing, int isMoving, bool isDead, bool onGround,
                           Texture &idleTex, Texture walkTex[], Texture &jumpTex, Texture deadTex[],
                           int &animFrame, int &deadAnimFrame, int &animCounter, int &deadAnimCounter, int animSpeed, int deadAnimSpeed)
{
    int texW = 0;
    int texH = 0;

    if (!isDead)
    {
        // Reset death animation if we somehow come back to life
        deadAnimFrame = 0;

        if (!onGround) // Priority 1: Jumping
        {
            PlayerSprite.setTexture(jumpTex, true);
            texW = jumpTex.getSize().x;
            texH = jumpTex.getSize().y;
        }
        else if (isMoving == 1) // Priority 2: Walking
        {
            animCounter++;
            if (animCounter >= animSpeed)
            {
                animCounter = 0;
                animFrame++;
                if (animFrame >= 6)
                    animFrame = 0;
            }

            PlayerSprite.setTexture(walkTex[animFrame], true);
            texW = walkTex[animFrame].getSize().x;
            texH = walkTex[animFrame].getSize().y;
        }
        else // Priority 3: Idle
        {
            animFrame = 0;
            PlayerSprite.setTexture(idleTex, true);
            texW = idleTex.getSize().x;
            texH = idleTex.getSize().y;
        }

        // --- FLIP LOGIC ---
        if (facing == 1) // Facing LEFT (Flipped)
        {
            PlayerSprite.setTextureRect(IntRect(texW, 0, -texW, texH));//-ve width flips the sprite horizontally
        }
        else // Facing RIGHT (Normal)
        {
            PlayerSprite.setTextureRect(IntRect(0, 0, texW, texH));
        }
    }
    else if (isDead)
    {
        deadAnimCounter++;
        if (deadAnimCounter >= deadAnimSpeed)
        {
            deadAnimCounter = 0;
            if (deadAnimFrame < 7)
            {
                deadAnimFrame++;
            }
        }

        PlayerSprite.setTexture(deadTex[deadAnimFrame], true);

        texW = deadTex[deadAnimFrame].getSize().x;
        texH = deadTex[deadAnimFrame].getSize().y;

        if (facing == 1)
            PlayerSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
        else
            PlayerSprite.setTextureRect(IntRect(0, 0, texW, texH));
    }
}

// player box overlaps enemy box then player will be killed
bool collisionDetection(RenderWindow &window, float playerX, float playerY, float enemyX, float enemyY, float playerW, float playerH, float enemyW, float enemyH, bool &isDead)
{
    if ((playerX < enemyX + enemyW) && (playerX + playerW > enemyX) && (playerY < enemyY + enemyH) && (playerY + playerH > enemyY))
    {
        // cout << "Collision Detected";
        isDead = true;
        return true;
    }
    return false;
}

// Safety check to prevent crashing if player goes off screen
char get_tile(char **lvl, int row, int col, int height, int width)
{
    if (row < 0 || row >= height || col < 0 || col >= width)
    {
        return ' '; // Outside the map is empty air
    }
    return lvl[row][col];
}

void display_level(RenderWindow &window, char **lvl, Texture &bgTex, Sprite &bgSprite, Texture &blockTexture, Sprite &blockSprite, const int height, const int width, const int cell_size)
{
    window.draw(bgSprite);

    for (int i = 0; i < height; i += 1)
    {
        for (int j = 0; j < width; j += 1)
        {
            // Draw walls '#' AND platforms '-'
            if (lvl[i][j] == '#' || lvl[i][j] == '-')
            {
                blockSprite.setPosition(j * cell_size, i * cell_size);
                window.draw(blockSprite);
            }
        }
    }
}

void player_gravity(char **lvl, float &offset_y, float &velocityY, bool &onGround, const float &gravity, float &terminal_Velocity, float &player_x, float &player_y, const int cell_size, int &Pheight, int &Pwidth, int height, int width, float dt)
{

    float new_y = player_y + velocityY * dt;

    // --- 1. Ceiling Collision (Moving Up) ---
    if (velocityY < 0)
    {
        char top_left = get_tile(lvl, (int)new_y / cell_size, (int)player_x / cell_size, height, width);
        char top_right = get_tile(lvl, (int)new_y / cell_size, (int)(player_x + Pwidth) / cell_size, height, width);

        // Only stop if hitting a SOLID WALL '#'
        if (top_left == '#' || top_right == '#')
        {
            velocityY = 0;
            new_y = ((int)new_y / cell_size + 1) * cell_size;
        }
    }

    // --- 2. Floor Collision (Moving Down) ---
    int feet_row = (int)(new_y + Pheight) / cell_size;
    int feet_col_left = (int)(player_x) / cell_size;
    int feet_col_right = (int)(player_x + Pwidth) / cell_size;

    bool landed = false;

    if (velocityY >= 0)
    {
        char block_left = get_tile(lvl, feet_row, feet_col_left, height, width);
        char block_right = get_tile(lvl, feet_row, feet_col_right, height, width);

        // TYPE A: Solid Wall '#' (Always land)
        if (block_left == '#' || block_right == '#')
        {
            landed = true;
        }
        // TYPE B: Platform '-' (Land only if crossing the top edge)
        else if (block_left == '-' || block_right == '-')
        {
            float block_top_pixel = feet_row * cell_size;
            const float tolerance = 4.0f;

            // Check if we are crossing the line downwards or standing on it
            if ((player_y + Pheight <= block_top_pixel + tolerance) && (new_y + Pheight >= block_top_pixel))
            {
                landed = true;
            }
        }
    }

    if (landed)
    {
        onGround = true;
        velocityY = 0;
        player_y = (feet_row * cell_size) - Pheight;
    }
    else
    {
        onGround = false;
        player_y = new_y;

        velocityY += gravity * dt; // this is dt. To smoothen out things
        if (velocityY >= terminal_Velocity)
            velocityY = terminal_Velocity;
    }
}

void playerCollision_x(char **lvl, float &player_x, float player_y,
                       const float &speed, const int cell_size, const int Pheight,
                       const int Pwidth, int height, int width, float dt, int &isMoving, int &facing)
{
    float offsetX_right = player_x + speed * dt;
    float offsetX_left = player_x - speed * dt;

    // --- Right Movement ---
    if (Keyboard::isKeyPressed(Keyboard::Right))
    {
        isMoving = 1;
        facing = 1;

        char top_right = get_tile(lvl, (int)player_y / cell_size, (int)(offsetX_right + Pwidth) / cell_size, height, width);
        char mid_right = get_tile(lvl, (int)(player_y + Pheight / 2) / cell_size, (int)(offsetX_right + Pwidth) / cell_size, height, width);
        char bot_right = get_tile(lvl, (int)(player_y + Pheight - 5) / cell_size, (int)(offsetX_right + Pwidth) / cell_size, height, width);

        // Only stop if solid wall '#'
        if (top_right == '#' || mid_right == '#' || bot_right == '#')
        {
            player_x = ((int)(offsetX_right + Pwidth) / cell_size) * cell_size - Pwidth - 1;
        }
        else
        {
            player_x += speed * dt;
        }
    }

    // --- Left Movement ---
    if (Keyboard::isKeyPressed(Keyboard::Left))
    {
        isMoving = 1;
        facing = 0;

        char top_left = get_tile(lvl, (int)player_y / cell_size, (int)(offsetX_left) / cell_size, height, width);
        char mid_left = get_tile(lvl, (int)(player_y + Pheight / 2) / cell_size, (int)(offsetX_left) / cell_size, height, width);
        char bot_left = get_tile(lvl, (int)(player_y + Pheight - 5) / cell_size, (int)(offsetX_left) / cell_size, height, width);

        if (top_left == '#' || mid_left == '#' || bot_left == '#')
        {
            player_x = ((int)(offsetX_left) / cell_size + 1) * cell_size;
        }
        else
        {
            player_x -= speed * dt;
        }
    }
}

    void addScore(int &playerScore, int &comboStreak, float &comboTimer, int points, 
              bool isDefeat, int &multiKillCount, float &multiKillTimer, const float dt)
{
    int finalPoints = points;
    
    // Apply combo multiplier for defeats
    if (isDefeat)
    {
        if (comboStreak >= 5)
            finalPoints = (int)(points * 2.0f);
        else if (comboStreak >= 3)
            finalPoints = (int)(points * 1.5f);
    }
    
    playerScore += finalPoints;
    
    // Reset combo timer
    if (isDefeat)
    {
        comboStreak++;
        comboTimer = 0.0f;
    }
}

void checkMultiKill(int &multiKillCount, float &multiKillTimer, int &playerScore)
{
    if (multiKillCount >= 3)
        playerScore += 500; // Multi-Kill (3+ enemies)
    else if (multiKillCount == 2)
        playerScore += 200; // Multi-Kill (2 enemies)
    
    multiKillCount = 0;
    multiKillTimer = 0.0f;
}

// --- SUCTION LOGIC FUNCTION ---
void handleVacuum(RenderWindow &window, Sprite &vacSprite, 
                  Texture &texHorz, Texture &texVert, 
                  float player_x, float player_y, 
                  int PlayerWidth, int PlayerHeight, int vacDir, bool isActive, 
                  float* enemiesX, float* enemiesY, int& enemyCount, 
                  int* inventory, int& capturedCount, int maxCap, int enemyType,
                  float &flickerTimer, bool &showSprite, float dt,
                  bool* enemyIsCaught,
                  bool drawOnly,float vacuumPower,
                  int& playerScore, int& comboStreak, float& comboTimer,
                  int& multiKillCount, float& multiKillTimer,
                  bool hasRangeBoost) 
{
    if (!isActive) return;
    if (capturedCount >= maxCap) return;

    // --- 1. FLICKER ANIMATION ---
    if (drawOnly)
    {
        flickerTimer += dt;
        if (flickerTimer >= 0.05f) 
        {
            showSprite = !showSprite;
            flickerTimer = 0;
        }
    }

    // --- 2. CALCULATE CENTERS & DIMENSIONS ---
    int pX = (int)player_x;
    int pY = (int)player_y;
    int pCenterX = pX + PlayerWidth / 2;
    int pCenterY = pY + PlayerHeight / 2;

  float rangeMultiplier = hasRangeBoost ? 1.5f : 1.0f; // 1.5x wider if boosted
int beamReach = 72 * vacuumPower * rangeMultiplier;
int beamThick = 30 * vacuumPower * rangeMultiplier;

    // Hitbox Variable
    IntRect vacHitbox;

    // --- 3. DIRECTION LOGIC ---

    if (vacDir == 0) // RIGHT
    {
        vacSprite.setTexture(texHorz, true);
        // Position: Exact Right Edge (No overlap). Center Vertically.
        vacSprite.setPosition(pX + PlayerWidth, pCenterY - beamThick/2); 
        vacSprite.setTextureRect(IntRect(0, 0, beamReach, beamThick));

        vacHitbox = IntRect(pX + PlayerWidth, pCenterY - beamThick/2, beamReach * 2, beamThick * 2);
    }
    else if (vacDir == 1) // LEFT
    {
        vacSprite.setTexture(texHorz, true);
        // Position: Exact Left Edge minus beam length. 
        vacSprite.setPosition(pX - (beamReach * 2), pCenterY - beamThick/2); 
        // Draw backwards (Visual only)
        vacSprite.setTextureRect(IntRect(beamReach, 0, -beamReach, beamThick)); 

        // Hitbox starts to the left of the player
        vacHitbox = IntRect(pX - (beamReach * 2), pCenterY - beamThick/2, beamReach * 2, beamThick * 2);
    }
    else if (vacDir == 2) // UP
    {
        vacSprite.setTexture(texVert, true);
        // X: Center Horizontal. Y: Exact Top Edge minus beam length.
        vacSprite.setPosition(pCenterX - beamThick/2, pY - (beamReach * 2));
        vacSprite.setTextureRect(IntRect(0, 0, beamThick, beamReach));

        vacHitbox = IntRect(pCenterX - beamThick/2, pY - (beamReach * 2), beamThick * 2, beamReach * 2);
    }
    else if (vacDir == 3) // DOWN
    {
        vacSprite.setTexture(texVert, true);
        // X: Center Horizontal. Y: Exact Bottom Edge.
        vacSprite.setPosition(pCenterX - beamThick/2, pY + PlayerHeight);
        vacSprite.setTextureRect(IntRect(0, 0, beamThick, beamReach));

        vacHitbox = IntRect(pCenterX - beamThick/2, pY + PlayerHeight, beamThick * 2, beamReach * 2);
    }

    // --- 4. DRAWING PASS ---
    if (drawOnly)
    {
        if (showSprite)
        {
            window.draw(vacSprite);
        }
        return; // Exit after drawing
    }

    // --- 5. LOGIC PASS (Only if drawOnly is FALSE) ---
    for (int i = 0; i < enemyCount; i++)
    {
        IntRect enemyRect((int)enemiesX[i], (int)enemiesY[i], 60, 60);

        if (vacHitbox.intersects(enemyRect))
        {
            // Mark enemy as caught so collision detection doesn't kill player
            enemyIsCaught[i] = true;

            // PULL LOGIC (RESTORED TO ORIGINAL SPEED)
            float pullSpeed = 250.0f * vacuumPower * dt;  // Pull speed affected by power
            
            if (enemiesX[i] < pCenterX) enemiesX[i] += pullSpeed;
            else enemiesX[i] -= pullSpeed;
            
            if (enemiesY[i] < pCenterY) enemiesY[i] += pullSpeed;
            else enemiesY[i] -= pullSpeed;

            // CAPTURE CHECK (RESTORED TO ORIGINAL DISTANCE)
            float dx = (float)(pCenterX - (enemiesX[i] + 30));
            float dy = (float)(pCenterY - (enemiesY[i] + 30));
            float dist = sqrt(dx*dx + dy*dy);

            if (dist < 50.0f * vacuumPower) 
{
    inventory[capturedCount] = enemyType;
    capturedCount++;
    
    // Award capture points ONLY during logic pass
    if (!drawOnly)
    {
        int capturePoints = 0;
        if (enemyType == 1) capturePoints = 50;  // Ghost
        else if (enemyType == 2) capturePoints = 75; // Skeleton
        
        addScore(playerScore, comboStreak, comboTimer, capturePoints, 
                 false, multiKillCount, multiKillTimer, dt);
    }
    
    // Remove enemy from array
    enemiesX[i] = enemiesX[enemyCount - 1];
    enemiesY[i] = enemiesY[enemyCount - 1];
    enemyCount--;
    i--;
}
        }
    }
}

 void spawnPowerup(float* powerupsX, float* powerupsY, int* powerupType, 
                  bool* powerupActive, float* powerupAnimTimer,
                  int& powerupCount, int maxPowerups, char** lvl, 
                  int mapWidth, int mapHeight, int cellSize)
{
    if (powerupCount >= maxPowerups) return;
    
    // Random powerup type (1-4)
    int type = (rand() % 4) + 1;
    
    // Try to find a valid platform position
    int attempts = 0;
    int randomCol, randomRow;
    bool foundValidSpot = false;
    
    while (attempts < 50 && !foundValidSpot)
    {
        // Random position (avoid edges)
        randomCol = 2 + (rand() % (mapWidth - 4));
        randomRow = 2 + (rand() % (mapHeight - 3));
        
        // Check if there's a platform or wall BELOW this position
        char tileBelow = lvl[randomRow + 1][randomCol];
        // Check if current position is empty
        char currentTile = lvl[randomRow][randomCol];
        
        // Valid spot if: current position is empty AND there's a platform/wall below
        if (currentTile == ' ' && (tileBelow == '-' || tileBelow == '#'))
        {
            foundValidSpot = true;
        }
        
        attempts++;
    }
    
    if (!foundValidSpot)
    {
        cout << "Could not find valid powerup spawn location\n";
        return;
    }
    
    powerupsX[powerupCount] = randomCol * cellSize;
    powerupsY[powerupCount] = randomRow * cellSize;
    powerupType[powerupCount] = type;
    powerupActive[powerupCount] = true;
    powerupAnimTimer[powerupCount] = 0.0f;
    
    cout << "Spawned powerup type " << type << " at (" << randomCol << ", " << randomRow << ")\n";
    
    powerupCount++;
} 


int main()
{
    bool playagain = true;
    bool firstrun=true;
    while (playagain)
    {
        
        // Score System Variables
         int playerScore = 0;
          int comboStreak = 0;
         float comboTimer = 0.0f;
         const float comboTimeout = 3.0f; // 3 seconds to maintain combo
          bool levelNoDamage = true;
           float levelTimer = 0.0f;
            int multiKillCount = 0;
          float multiKillTimer = 0.0f;
       const float multiKillWindow = 1.0f; // 1 second window for multi-kills
       
       
        const float dt = 0.018f; // dt to smooth everything 0.018
        srand(time(0));          //  Initialize random seed for skeleton jump timing
        RenderWindow window(VideoMode(screen_x, screen_y), "Tumble-POP", Style::Close | Style::Resize);

        const int cell_size = 64;
        const int height = 14;
        const int width = 20; // 18
        char **lvl;
        
        
        
        Texture gameOverBGTexture;
        Sprite gameOverBGSprite;

        float speedMultiplier = 1.0f;
        float vacuumPower = 1.0f;

        Texture PlayerTexture;
        Sprite PlayerSprite;

        int PlayerWidth, PlayerHeight;
        float scale = 2.8f;

        int facing = 1;
        int isMoving = 0;
        Texture idleTex, walkTex[6], jumpTex, deadTex[8];

        int animFrame = 0, animCounter = 0, animSpeed = 5,
            deadAnimCounter = 0, deadAnimFrame = 0, deadAnimSpeed = 60;
        bool isDead = false;
        bool restartGame = false;
       
        
        // ------------------------------------
        // INTRO SCREEN (BEFORE CHARACTER MENU)
        // ------------------------------------

       if(firstrun)// so the intro screen runs only once
       {
        Texture introTex;
        Sprite introSprite;

        if (!introTex.loadFromFile("Data/intro.png"))
            cout << "intro.png missing!\n";

        introSprite.setTexture(introTex);
        introSprite.setPosition(0, 0); 
        introSprite.setScale(1.8, 1.8);
        // Press Enter to Start text
        Font fontIntro;
        fontIntro.loadFromFile("Data/font.ttf");

        Text startText("PRESS ENTER TO START", fontIntro, 50);
        startText.setFillColor(Color::White);
        startText.setPosition(300, 800); 

        bool startGame = false;

        while (window.isOpen() && !startGame)
        {
            Event e;
            while (window.pollEvent(e))
            {
                if (e.type == Event::Closed)
                    window.close();

                if (e.type == Event::KeyPressed && e.key.code == Keyboard::Enter)
                    startGame = true;
            }

            window.clear();
            window.draw(introSprite);
            window.draw(startText);
            window.display();
        }
        firstrun=false;
        }

        // font
        Font font;
        if (!font.loadFromFile("Data/font.ttf"))
            cout << " font.ttf missing\n";

        Text infoText("", font, 30);

        infoText.setFillColor(Color::White);
        infoText.setPosition(20, 20);

        // game over screen text
        Text gameOverText("GAME OVER!!", font, 120);
        gameOverText.setFillColor(Color::Red);
        gameOverText.setPosition(330, 300);  //350,300

        Text livesRemainingText("", font, 40);
        livesRemainingText.setFillColor(Color::White);
        livesRemainingText.setPosition(470, 450); // 400,500

        Text restartText("Press ENTER to continue...", font, 50);
        restartText.setFillColor(Color::Yellow);
        restartText.setPosition(400, 520); //300,650
        
         Text escText("Press ESC to Exit", font, 45);
        escText.setFillColor(Color::Magenta);
        escText.setPosition(470, 600); //300,650

        // menu screen

        Texture menuBGTexture;
        Sprite menuBGSprite;

        if (!menuBGTexture.loadFromFile("Data/menuBG.png")) 
            cout << "Menu background missing!\n";

        menuBGSprite.setTexture(menuBGTexture);
        
        if (!gameOverBGTexture.loadFromFile("Data/gameover.png"))
            cout << "Game Over background missing!\n";

        gameOverBGSprite.setTexture(gameOverBGTexture);

        // Scale to fill the window
        gameOverBGSprite.setScale(
            float(screen_x) / gameOverBGTexture.getSize().x,
            float(screen_y) / gameOverBGTexture.getSize().y);

        // make image fill the whole window
        menuBGSprite.setScale(
            float(screen_x) / menuBGTexture.getSize().x,
            float(screen_y) / menuBGTexture.getSize().y);
            
         // Score Display
        Text scoreText("Score: 0", font, 35);
         scoreText.setFillColor(Color::Yellow);
          scoreText.setPosition(screen_x - 250, 10);

         Text comboText("", font, 30);
            comboText.setFillColor(Color::Cyan);
            comboText.setPosition(screen_x - 250, 50);   

        Text title("Game Menu", font, 100);
        title.setFillColor(Color::Magenta);
        title.setPosition(400, 200); // 250

        Text subtitle("Press 1 for Yellow (Fast) ", font, 50);
        subtitle.setFillColor(Color::Yellow);
        subtitle.setPosition(120, 400); // 120

        Text subtitle2(" Press 2 for Green (Strong Vacuum)", font, 50);
        subtitle2.setFillColor(Color::Green);
        subtitle2.setPosition(100, 500); // 120

        Text subtitle3(" Press Esc to EXIT", font, 50);
        subtitle3.setFillColor(Color::Red);
        subtitle3.setPosition(100, 600); // 120

        bool characterSelected = false;
        while (window.isOpen() && !characterSelected)
        {
            Event e;
            while (window.pollEvent(e))
            {
                if (e.type == Event::Closed)
                    window.close();
                if (e.type == Event::KeyPressed)
                {
                    if (e.key.code == Keyboard::Num1)
                    {

                        PlayerTexture.loadFromFile("Data/player.png");
                        PlayerSprite.setTexture(PlayerTexture);
                        PlayerSprite.setScale(scale, scale);

                        idleTex.loadFromFile("Data/yellowPlayer/idle1.png");
                        jumpTex.loadFromFile("Data/yellowPlayer/jump1.png");
                        walkTex[0].loadFromFile("Data/yellowPlayer/walk1.png");
                        walkTex[1].loadFromFile("Data/yellowPlayer/walk2.png");
                        walkTex[2].loadFromFile("Data/yellowPlayer/walk3.png");
                        walkTex[3].loadFromFile("Data/yellowPlayer/walk4.png");
                        walkTex[4].loadFromFile("Data/yellowPlayer/walk5.png");
                        walkTex[5].loadFromFile("Data/yellowPlayer/walk6.png");

                        deadTex[0].loadFromFile("Data/yellowPlayerDying/dying1.png");
                        deadTex[1].loadFromFile("Data/yellowPlayerDying/dying2.png");
                        deadTex[2].loadFromFile("Data/yellowPlayerDying/dying3.png");
                        deadTex[3].loadFromFile("Data/yellowPlayerDying/dying4.png");
                        deadTex[4].loadFromFile("Data/yellowPlayerDying/dying5.png");
                        deadTex[5].loadFromFile("Data/yellowPlayerDying/dying6.png");
                        deadTex[6].loadFromFile("Data/yellowPlayerDying/dying7.png");
                        deadTex[7].loadFromFile("Data/yellowPlayerDying/dying8.png");

                        PlayerWidth = 31 * scale;
                        PlayerHeight = 42 * scale;

                        speedMultiplier = 1.0f;
                        vacuumPower = 1.2f;
                        characterSelected = true;
                    }
                    if (e.key.code == Keyboard::Num2)
                    {

                        PlayerTexture.loadFromFile("Data/greenPlayer/idle1.png");
                        PlayerSprite.setTexture(PlayerTexture);

                        PlayerSprite.setScale(scale, scale);

                        idleTex.loadFromFile("Data/greenPlayer/idle1.png");
                        jumpTex.loadFromFile("Data/greenPlayer/jump1.png");
                        walkTex[0].loadFromFile("Data/greenPlayer/walk1.png");
                        walkTex[1].loadFromFile("Data/greenPlayer/walk2.png");
                        walkTex[2].loadFromFile("Data/greenPlayer/walk3.png");
                        walkTex[3].loadFromFile("Data/greenPlayer/walk4.png");
                        walkTex[4].loadFromFile("Data/greenPlayer/walk5.png");
                        walkTex[5].loadFromFile("Data/greenPlayer/walk6.png");

                        deadTex[0].loadFromFile("Data/greenPlayerDying/dying1.png");
                        deadTex[1].loadFromFile("Data/greenPlayerDying/dying2.png");
                        deadTex[2].loadFromFile("Data/greenPlayerDying/dying3.png");
                        deadTex[3].loadFromFile("Data/greenPlayerDying/dying4.png");
                        deadTex[4].loadFromFile("Data/greenPlayerDying/dying5.png");
                        deadTex[5].loadFromFile("Data/greenPlayerDying/dying6.png");
                        deadTex[6].loadFromFile("Data/greenPlayerDying/dying7.png");
                        deadTex[7].loadFromFile("Data/greenPlayerDying/dying8.png");

                        PlayerWidth = 31 * scale;
                        PlayerHeight = 42 * scale;

                        speedMultiplier = 1.5f;
                        vacuumPower = 1.0f;
                        characterSelected = true;
                    }
                    if (e.key.code == Keyboard::Escape)
                        window.close();
                }
            }
            window.clear(Color::Black);
            window.draw(menuBGSprite);
            window.draw(title);

            window.draw(subtitle);
            window.draw(subtitle2);
            window.draw(subtitle3);
            window.display();
        }

        if (!characterSelected)
            return 0;

        Texture bgTex;
        Sprite bgSprite;
        Texture blockTexture;
        Sprite blockSprite;

        bgTex.loadFromFile("Data/bg.png");
        bgSprite.setTexture(bgTex);
        bgSprite.setPosition(0, 0);
        bgSprite.setScale(2.0f, 2.0f);

        blockTexture.loadFromFile("Data/block1.png");
        blockSprite.setTexture(blockTexture);

        Music lvlMusic;
        lvlMusic.openFromFile("Data/mus.ogg");
        lvlMusic.setVolume(20);
        lvlMusic.setLoop(true);
        lvlMusic.play();

        float player_x = 850.0f; // 650.f
        float player_y = 450.f;

        float speed = 140.0f * speedMultiplier;  // Apply character speed bonus

        const float jumpStrength = -150.0f; // 150
        const float gravity = 90.f;         // 90

        bool isJumping = false;

         // ghosts
        int enemyCount = 0;
        const int maxEnemyCount = 8;

        float enemiesX[maxEnemyCount];
        float enemiesY[maxEnemyCount];
        float enemySpeed[maxEnemyCount];
        int enemyDirection[maxEnemyCount];
        float platformLeftEdge[maxEnemyCount];
        float platformRightEdge[maxEnemyCount];
        bool enemyIsCaught[maxEnemyCount]; // SAFETY FLAG

        int EnemyHeight = 60;
        int EnemyWidth = 72;

        Texture EnemyTexture;
        Sprite EnemySprite;

        EnemyTexture.loadFromFile("Data/ghost.png");
        EnemySprite.setTexture(EnemyTexture);
        EnemySprite.setScale(2, 2);

        // Skeleton enemies (can move between platforms)
        int skeletonCount = 0;
        const int maxSkeletonCount = 4;

        float skeletonsX[maxSkeletonCount];
        float skeletonsY[maxSkeletonCount];
        float skeletonSpeed[maxSkeletonCount];
        int skeletonDirection[maxSkeletonCount];
        float skeletonVelocityY[maxSkeletonCount];
        bool skeletonOnGround[maxSkeletonCount];
        float skeletonJumpTimer[maxSkeletonCount];    //  Timer for jump intervals
        float skeletonJumpCooldown[maxSkeletonCount]; //  Random cooldown between jumps
        bool skeletonShouldJump[maxSkeletonCount];    //  Flag to control when to attempt jump
        int skeletonStableFrames[maxSkeletonCount];   //  Count frames on ground before allowing jump
        bool skeletonIsCaught[maxSkeletonCount];      // SAFETY FLAG

        Texture skeletonWalkTex[4]; // Array for 4 walking frames
        int skeletonAnimFrame[maxSkeletonCount];
        int skeletonAnimCounter[maxSkeletonCount];
        int skeletonAnimSpeed = 8; // Adjust this to control animation speed

        int SkeletonHeight = 92; // 60
        int SkeletonWidth = 72;

        Texture SkeletonTexture;
        Sprite SkeletonSprite;

        SkeletonTexture.loadFromFile("Data/skeleton.png");
        SkeletonSprite.setTexture(SkeletonTexture);
        SkeletonSprite.setScale(2, 2);

        skeletonWalkTex[0].loadFromFile("Data/skeletonWalk/walk1.png");
        skeletonWalkTex[1].loadFromFile("Data/skeletonWalk/walk2.png");
        skeletonWalkTex[2].loadFromFile("Data/skeletonWalk/walk3.png");
        skeletonWalkTex[3].loadFromFile("Data/skeletonWalk/walk4.png");

        bool onGround = false;

        int playerLives = 3;
        float respawnX = 850.0f;
        float respawnY = 450.0f;
        bool showGameOver = false;
        float deathDelayCounter = 0.0f;
        float deathDelayTime = 2.0f;
        bool waitingToRespawn = false;

        float offset_y = 0;
        float velocityY = 0;
        float terminal_Velocity = 300.f; // 300
        
        // --- POWERUP SYSTEM ---
const int maxPowerups = 4;
int powerupCount = 0;

float powerupsX[maxPowerups];
float powerupsY[maxPowerups];
int powerupType[maxPowerups]; // 1=Speed, 2=Range, 3=Power, 4=ExtraLife
bool powerupActive[maxPowerups];
float powerupAnimTimer[maxPowerups];

int PowerupWidth = 48;
int PowerupHeight = 48;

// Powerup effect durations and flags
float speedBoostTimer = 0.0f;
float rangeBoostTimer = 0.0f;
float powerBoostTimer = 0.0f;
const float powerupDuration = 10.0f; // 10 seconds for temporary powerups

bool hasSpeedBoost = false;
bool hasRangeBoost = false;
bool hasPowerBoost = false;

float originalSpeed = 140.0f;
float originalVacuumPower = 1.0f;

// Textures for powerups
Texture speedPowerupTex, rangePowerupTex, powerPowerupTex, lifePowerupTex;
Sprite powerupSprite;

            // Load powerup textures
if (!speedPowerupTex.loadFromFile("Data/speed.png"))
    cout << "Speed powerup texture missing!\n";
if (!rangePowerupTex.loadFromFile("Data/range.png"))
    cout << "Range powerup texture missing!\n";
if (!powerPowerupTex.loadFromFile("Data/power.png"))
    cout << "Power powerup texture missing!\n";
if (!lifePowerupTex.loadFromFile("Data/life.png"))
    cout << "Life powerup texture missing!\n";

powerupSprite.setScale(2.0f, 2.0f);

        // int PlayerHeight = 102;
        // int PlayerWidth = 96;

        // --- VACUUM SETUP ---
        Texture vacTexHorz, vacTexVert;
        if (!vacTexHorz.loadFromFile("Data/horizontalVacuum.png")) cout << "Horizontal Vacuum texture missing\n";
        if (!vacTexVert.loadFromFile("Data/verticalVacuum.png")) cout << "Vertical vacuum texture missing\n";

        Sprite vacSprite;
        vacSprite.setScale(2.0f, 2.0f); 

        int vacDirection = 0; 
        bool isVacuuming = false;
        float vacFlickerTimer = 0.0f;
        bool showVacSprite = true; 

        const int MAX_CAPACITY = 3; 
        int capturedEnemies[MAX_CAPACITY]; 
        int capturedCount = 0;

        // --- LEVEL CREATION (YOUR ORIGINAL MAP) ---
        lvl = new char *[height];
        for (int i = 0; i < height; i += 1)
        {
            lvl[i] = new char[width];
            // IMPORTANT: Fill with empty space first to remove garbage data!
            for (int j = 0; j < width; j++)
                lvl[i][j] = ' ';
        }

        // Paste of your original level layout
        lvl[1][3] = '-';
        lvl[1][4] = '-';
        lvl[1][5] = '-';
        lvl[1][6] = '-';
        lvl[1][7] = '-';
        lvl[1][8] = '-';
        lvl[1][9] = '-';
        lvl[1][10] = '-';
        lvl[1][11] = '-';
        lvl[1][12] = '-';
        lvl[1][13] = '-';
        lvl[1][14] = '-';

        lvl[9][3] = '-';
        lvl[9][4] = '-';
        lvl[9][5] = '-';
        lvl[9][6] = '-';
        lvl[9][7] = '-';
        lvl[9][8] = '-';
        lvl[9][9] = '-';
        lvl[9][10] = '-';
        lvl[9][11] = '-';
        lvl[9][12] = '-';
        lvl[9][13] = '-';
        lvl[9][14] = '-';

        lvl[8][8] = '-';
        lvl[8][9] = '-';

        lvl[7][1] = '-';
        lvl[7][2] = '-';
        lvl[7][3] = '-';
        lvl[7][9] = '-';
        lvl[7][8] = '-';
        lvl[7][7] = '-';
        lvl[7][10] = '-';
        lvl[7][14] = '-';
        lvl[7][15] = '-';
        lvl[7][16] = '-';

        lvl[6][7] = '-';
        lvl[6][10] = '-';

        // --- MODIFICATION: I changed these two to '-' so you can test jumping through them ---
        lvl[5][7] = '-';
        lvl[5][10] = '-';

        lvl[5][3] = '-';
        lvl[5][4] = '-';
        lvl[5][5] = '-';
        lvl[5][6] = '-';
        lvl[5][11] = '-';
        lvl[5][12] = '-';
        lvl[5][13] = '-';
        lvl[5][14] = '-';

        lvl[4][7] = '-';
        lvl[4][10] = '-';
        lvl[3][7] = '-';
        lvl[3][10] = '-';
        lvl[3][8] = '-';
        lvl[3][9] = '-';
        lvl[3][1] = '-';
        lvl[3][2] = '-';
        lvl[3][3] = '-';
        lvl[3][16] = '-';
        lvl[3][15] = '-';
        lvl[3][14] = '-';

        lvl[2][8] = '-';
        lvl[2][9] = '-';

        // Floor and Sides (Restored from your code)
        for (int j = 0; j <= 18; j++)
            lvl[11][j] = '#';
        for (int i = 0; i <= 10; i++)
        {
            lvl[i][0] = '#';
            lvl[i][18] = '#';
        }
        lvl[7][17] = '#';
        lvl[3][17] = '#';
        lvl[2][8] = '#';
        lvl[2][9] = '#';
        lvl[8][8] = '#';
        lvl[8][9] = '#';
        lvl[7][7] = '#';
        lvl[7][8] = '#';
        lvl[7][9] = '#';
        lvl[7][10] = '#';
        lvl[6][7] = '#';
        lvl[6][10] = '#';
        lvl[5][7] = '#';
        lvl[5][10] = '#';
        lvl[4][7] = '#';
        lvl[4][10] = '#';
        lvl[3][7] = '#';
        lvl[3][8] = '#';
        lvl[3][9] = '#';
        lvl[3][10] = '#';

        lvl[0][5] = 'e';
        lvl[0][12] = 'e';
        lvl[2][2] = 'e';
        lvl[2][15] = 'e';
        lvl[4][4] = 'e';
        lvl[4][13] = 'e';
        lvl[8][6] = 'e';
        lvl[8][10] = 'e';

        // Skeleton spawn points (marked with 's') - TOP PLATFORMS
        lvl[0][7] = 's';  // Top middle platform
        lvl[0][10] = 's'; // Top middle platform
        lvl[2][4] = 's';  // Upper left platform
        lvl[2][13] = 's'; // Upper right platform

        for (int r = 0; r < height; r++)
        {
            for (int c = 0; c < width; c++)
            {

                if (lvl[r][c] == 'e' && enemyCount < maxEnemyCount)
                {
                    int platformRow = r + 1;
                    char below = get_tile(lvl, platformRow, c, height, width);

                    // Enemy must stand on a platform or wall
                    if (below == '-' || below == '#')
                    {
                        enemiesX[enemyCount] = c * cell_size;
                        enemiesY[enemyCount] = r * cell_size;

                        // Detect edges of THAT platform
                        int leftEdge = c + 1;
                        int rightEdge = c + 1;

                        while (leftEdge > 0 && (lvl[platformRow][leftEdge - 1] == '-' || lvl[platformRow][leftEdge - 1] == '#'))
                            leftEdge--;

                        while (rightEdge < width - 1 && (lvl[platformRow][rightEdge + 1] == '-' || lvl[platformRow][rightEdge + 1] == '#'))
                            rightEdge++;

                        platformLeftEdge[enemyCount] = leftEdge * cell_size + 10;
                        platformRightEdge[enemyCount] = (rightEdge + 1) * cell_size - 48 - 10;

                        enemySpeed[enemyCount] = 15.f; // 15.f
                        enemyDirection[enemyCount] = 1;

                        enemyCount++;
                    }

                    lvl[r][c] = ' '; // Clear the 'e' marker so it doesn't interfere with rendering
                }
            }
        }
        cout << "Total enemies: " << enemyCount << endl;

        // Initialize skeletons
        for (int r = 0; r < height; r++)
        {
            for (int c = 0; c < width; c++)
            {
                if (lvl[r][c] == 's' && skeletonCount < maxSkeletonCount)
                {
                    skeletonsX[skeletonCount] = c * cell_size;
                    skeletonsY[skeletonCount] = r * cell_size;
                    skeletonSpeed[skeletonCount] = 40.f; // 40
                    skeletonDirection[skeletonCount] = 1;
                    skeletonVelocityY[skeletonCount] = 0;
                    skeletonOnGround[skeletonCount] = false;
                    skeletonJumpTimer[skeletonCount] = 0.f;
                    skeletonJumpCooldown[skeletonCount] = 1.5f + (rand() % 20) / 10.0f; // Random 1.5-3.5s
                    skeletonShouldJump[skeletonCount] = false;
                    skeletonStableFrames[skeletonCount] = 0;

                    skeletonAnimFrame[skeletonCount] = 0;
                    skeletonAnimCounter[skeletonCount] = 0;

                    skeletonCount++;
                    lvl[r][c] = ' '; // Clear the marker
                }
            }
        }
        cout << "Total skeletons: " << skeletonCount << endl;

        // End of original map 
        
        // Spawn initial powerups

// Spawn initial powerups
for (int i = 0; i < 3; i++) // Spawn 3 random powerups
{
    spawnPowerup(powerupsX, powerupsY, powerupType, powerupActive, 
                 powerupAnimTimer, powerupCount, maxPowerups, 
                 lvl, width, height, cell_size);  // <-- Added 'lvl' parameter
}

// Store original values for character
originalSpeed = speed;
originalVacuumPower = vacuumPower;

// Store original values for character
originalSpeed = speed;
originalVacuumPower = vacuumPower;  
          
        Event ev;

        while (window.isOpen() && !restartGame)
        {
            while (window.pollEvent(ev))
            {
                if (ev.type == Event::Closed)
                    window.close();
            }
            if (Keyboard::isKeyPressed(Keyboard::Escape))
                window.close();
            
            // Update timers
levelTimer += dt;
comboTimer += dt;
multiKillTimer += dt;

// Reset combo if timeout
if (comboTimer >= comboTimeout)
{
    comboStreak = 0;
}

// Check multi-kill timeout
if (multiKillTimer >= multiKillWindow && multiKillCount > 0)
{
    checkMultiKill(multiKillCount, multiKillTimer, playerScore);
}

            // Update powerup timers
if (hasSpeedBoost)
{
    speedBoostTimer += dt;
    if (speedBoostTimer >= powerupDuration)
    {
        hasSpeedBoost = false;
        speed = originalSpeed * speedMultiplier; // Reset to normal
        cout << "Speed boost expired\n";
    }
}

if (hasRangeBoost)
{
    rangeBoostTimer += dt;
    if (rangeBoostTimer >= powerupDuration)
    {
        hasRangeBoost = false;
        cout << "Range boost expired\n";
    }
}

if (hasPowerBoost)
{
    powerBoostTimer += dt;
    if (powerBoostTimer >= powerupDuration)
    {
        hasPowerBoost = false;
        vacuumPower = originalVacuumPower; // Reset to character's base power
        cout << "Power boost expired\n";
    }
}

// Update powerup animations
for (int i = 0; i < powerupCount; i++)
{
    if (powerupActive[i])
        powerupAnimTimer[i] += dt;
}
            
            // *** RESET SAFETY FLAGS EVERY FRAME ***
            for (int i = 0; i < maxEnemyCount; i++) enemyIsCaught[i] = false;
            for (int i = 0; i < maxSkeletonCount; i++) skeletonIsCaught[i] = false;

            // --- VACUUM INPUT ---
            isVacuuming = false;
            if (Keyboard::isKeyPressed(Keyboard::Space))
            {
                isVacuuming = true;
                if (Keyboard::isKeyPressed(Keyboard::D)) 
                {
                    vacDirection = 0; // Right
                    facing = 1; // Force Player Right
                }
                else if (Keyboard::isKeyPressed(Keyboard::A)) 
                {
                    vacDirection = 1; // Left
                    facing = 0; // Force Player Left
                }
                else if (Keyboard::isKeyPressed(Keyboard::W)) vacDirection = 2; // Up
                else if (Keyboard::isKeyPressed(Keyboard::S)) vacDirection = 3; // Down
                else vacDirection = facing; // Default to player facing
            }
            else showVacSprite = true;

            // --- EXECUTE VACUUM LOGIC BEFORE ENEMY LOGIC (drawOnly = false) ---
            // This ensures enemyIsCaught flags are set correctly
            handleVacuum(window, vacSprite, vacTexHorz, vacTexVert, 
                 player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming, 
                 enemiesX, enemiesY, enemyCount, capturedEnemies, capturedCount, MAX_CAPACITY, 1, 
                 vacFlickerTimer, showVacSprite, dt, 
                 enemyIsCaught, false,vacuumPower,
                   playerScore,  comboStreak, comboTimer,
                 multiKillCount, multiKillTimer,
     hasRangeBoost); 

            handleVacuum(window, vacSprite, vacTexHorz, vacTexVert, 
                 player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming, 
                 skeletonsX, skeletonsY, skeletonCount, capturedEnemies, capturedCount, MAX_CAPACITY, 2, 
                 vacFlickerTimer, showVacSprite, dt, 
                 skeletonIsCaught, false,vacuumPower,
                   playerScore, comboStreak, comboTimer,
                  multiKillCount,  multiKillTimer,
     hasRangeBoost);
                  
             // --- POWERUP COLLECTION ---
for (int i = 0; i < powerupCount; i++)
{
    if (!powerupActive[i]) continue;
    
    // Check collision with player
    if ((player_x < powerupsX[i] + PowerupWidth) && 
        (player_x + PlayerWidth > powerupsX[i]) && 
        (player_y < powerupsY[i] + PowerupHeight) && 
        (player_y + PlayerHeight > powerupsY[i]))
    {
        // Collect powerup
        powerupActive[i] = false;
        
        switch(powerupType[i])
        {
            case 1: // Speed Boost
                hasSpeedBoost = true;
                speedBoostTimer = 0.0f;
                speed = originalSpeed * speedMultiplier * 2.0f; // Double speed
                playerScore += 100;
                cout << "SPEED BOOST!\n";
                break;
                
            case 2: // Range Boost (wider vacuum)
                hasRangeBoost = true;
                rangeBoostTimer = 0.0f;
                playerScore += 100;
                cout << "RANGE BOOST!\n";
                break;
                
            case 3: // Power Boost (stronger vacuum)
                hasPowerBoost = true;
                powerBoostTimer = 0.0f;
                vacuumPower = originalVacuumPower * 1.5f; // 1.5x stronger
                playerScore += 100;
                cout << "POWER BOOST!\n";
                break;
                
            case 4: // Extra Life
                playerLives++;
                playerScore += 200;
                cout << "EXTRA LIFE!\n";
                break;
        }
        
        // Remove collected powerup
        powerupsX[i] = powerupsX[powerupCount - 1];
        powerupsY[i] = powerupsY[powerupCount - 1];
        powerupType[i] = powerupType[powerupCount - 1];
        powerupActive[i] = powerupActive[powerupCount - 1];
        powerupCount--;
        i--;
    }
}     

            isMoving = 0;
            // Movement - 
            playerCollision_x(lvl, player_x, player_y, speed, cell_size, PlayerHeight,
                              PlayerWidth, height, width, dt, isMoving, facing);
            updatePlayerAnimation(PlayerSprite, facing, isMoving, isDead, onGround, idleTex,
                                  walkTex, jumpTex, deadTex, animFrame, deadAnimFrame, animCounter, deadAnimCounter, animSpeed, deadAnimSpeed);
            // Sprite &PlayerSprite, int facing, int isMoving, bool isDead, bool onGround,
            //                Texture &idleTex, Texture walkTex[], Texture &jumpTex, Texture &deadTex[],
            //              int &animFrame, int &deadAnimFrame, int &animCounter, int deadAnimCounter, int animSpeed
            if (Keyboard::isKeyPressed(Keyboard::Up) && onGround)
            {
                velocityY = jumpStrength;
                onGround = false;
                isJumping = true;
            }

            // Apply gravity
            player_gravity(lvl, offset_y, velocityY, onGround, gravity, terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth, height, width, dt);

            // --- 1. GHOST ENEMY LOOP ---
            for (int i = 0; i < enemyCount; i++)
            {
                enemiesX[i] += enemySpeed[i] * enemyDirection[i] * dt;

                // Check boundaries and reverse direction
                if (enemiesX[i] <= platformLeftEdge[i])
                {
                    enemiesX[i] = platformLeftEdge[i];
                    enemyDirection[i] = 1; // Move right
                }
                else if (enemiesX[i] >= platformRightEdge[i])
                {
                    enemiesX[i] = platformRightEdge[i];
                    enemyDirection[i] = -1; // Move left
                }

                // CHECK COLLISION (Only if not caught by vacuum)
                if (!enemyIsCaught[i])
                {
                    if (collisionDetection(window, player_x, player_y, enemiesX[i], enemiesY[i],
                                           PlayerWidth, PlayerHeight, EnemyWidth, EnemyHeight, isDead))
                    {
                        if (!waitingToRespawn)
                        {
                            playerLives--;
                            waitingToRespawn = true;
                            deathDelayCounter = 0.0f;
                             levelNoDamage = false; 
                          playerScore -= 50; //  DAMAGE PENALTY
                        }
                    }
                }
            } // end of ghost loop

            // --- 2. SKELETON ENEMY LOOP ---
            // Update skeletons with gravity, platform movement, and intelligent jumping
            for (int i = 0; i < skeletonCount; i++)
            {
                // *** HORIZONTAL MOVEMENT (WORKS IN AIR AND ON GROUND) ***
                float newX = skeletonsX[i] + skeletonSpeed[i] * skeletonDirection[i] * dt;

                // Check wall collision for horizontal movement
                char right_check = get_tile(lvl, (int)(skeletonsY[i] + SkeletonHeight / 2) / cell_size,
                                            (int)(newX + SkeletonWidth) / cell_size, height, width);
                char left_check = get_tile(lvl, (int)(skeletonsY[i] + SkeletonHeight / 2) / cell_size,
                                           (int)newX / cell_size, height, width);

                // Reverse direction if hitting wall
                if ((skeletonDirection[i] == 1 && right_check == '#') ||
                    (skeletonDirection[i] == -1 && left_check == '#'))
                {
                    skeletonDirection[i] *= -1;
                }
                else
                {
                    skeletonsX[i] = newX; // Move horizontally whether jumping or not
                }

                // Apply gravity to skeletons (AFTER horizontal movement)
                player_gravity(lvl, offset_y, skeletonVelocityY[i], skeletonOnGround[i],
                               gravity, terminal_Velocity, skeletonsX[i], skeletonsY[i],
                               cell_size, SkeletonHeight, SkeletonWidth, height, width, dt);

                // Track how long skeleton has been stable on ground
                if (skeletonOnGround[i])
                {
                    skeletonStableFrames[i]++;
                }
                else
                {
                    skeletonStableFrames[i] = 0;
                }

                // *** INTELLIGENT JUMPING LOGIC ***
                skeletonJumpTimer[i] += dt;

                // Only check for new jump opportunity if skeleton has been stable for a bit
                if (skeletonOnGround[i] && skeletonStableFrames[i] > 350 && !skeletonShouldJump[i])
                {
                    // Random chance to decide to jump (8% chance per frame for more frequent jumps)
                    if (rand() % 100 < 1)
                    {
                        // Check if there's a platform above to jump to
                        int currentRow = (int)(skeletonsY[i] + SkeletonHeight) / cell_size;
                        int skeletonCol = (int)(skeletonsX[i] + SkeletonWidth / 2) / cell_size;

                        bool platformAbove = false;

                        // Look for platforms 2-5 rows above (wider search range)
                        for (int checkRow = currentRow - 5; checkRow < currentRow - 1; checkRow++)
                        {
                            if (checkRow >= 0)
                            {
                                // Check if there's a platform within jumping range
                                for (int checkCol = skeletonCol - 3; checkCol <= skeletonCol + 3; checkCol++)
                                {
                                    char tile = get_tile(lvl, checkRow, checkCol, height, width);
                                    if (tile == '-' || tile == '#')
                                    {
                                        platformAbove = true;
                                        break;
                                    }
                                }
                            }
                            if (platformAbove)
                                break;
                        }

                        // Only set jump flag if there's actually a platform to jump to
                        if (platformAbove)
                        {
                            skeletonShouldJump[i] = true;
                            skeletonJumpTimer[i] = 0.f;
                        }
                    }
                }

                // Execute the jump if flagged and on ground
                if (skeletonShouldJump[i] && skeletonOnGround[i] && skeletonStableFrames[i] > 30)
                {
                    skeletonVelocityY[i] = jumpStrength; // Full player jump strength
                    skeletonOnGround[i] = false;
                    skeletonShouldJump[i] = false; // Reset flag
                    skeletonStableFrames[i] = 0;
                }

                // Collision detection with player (Only if not caught by vacuum)
                if (!skeletonIsCaught[i])
                {
                    if (collisionDetection(window, player_x, player_y, skeletonsX[i], skeletonsY[i],
                                           PlayerWidth, PlayerHeight, SkeletonWidth, SkeletonHeight, isDead))
                    {
                        if (!waitingToRespawn)
                        {
                            playerLives--;
                            waitingToRespawn = true;
                            deathDelayCounter = 0.0f;
                        }
                    }
                }
            } // end of skeleton loop

           // Check if level is complete (all enemies defeated)
if (enemyCount == 0 && skeletonCount == 0 && capturedCount == 0)
{
    // Level 1 Complete Bonus
    playerScore += 1000; // Level Clear
    
    if (levelNoDamage)
        playerScore += 1500; // No Damage Bonus
    
    // Speed bonuses
    if (levelTimer < 30.0f)
        playerScore += 2000;
    else if (levelTimer < 45.0f)
        playerScore += 1000;
    else if (levelTimer < 60.0f)
        playerScore += 500;
    
    // Character bonus
    if (speedMultiplier == 1.5f) // Yellow character
        playerScore += 500; // Speed Demon
    else if (vacuumPower == 1.2f) // Green character
        playerScore += 500; // Max Capacity
    
    cout << "LEVEL COMPLETE! Final Score: " << playerScore << endl;
    // TODO: Move to next level or show victory screen
}

            // game logic
            if (waitingToRespawn)
            {
                deathDelayCounter += dt;

                if (deathDelayCounter >= deathDelayTime)
                {
                    if (playerLives > 0)
                    {
                        // Respawn player
                        isDead = false;
                        player_x = respawnX;
                        player_y = respawnY;
                        velocityY = 0;
                        onGround = false;
                        waitingToRespawn = false;
                        deadAnimFrame = 0;
                        deadAnimCounter = 0;
                        deathDelayCounter = 0.0f;
                    }
                    else
                    {
                        // Game Over
                        showGameOver = true;
                        waitingToRespawn = false;

                    }
                }
            }

            // Display Game Over screen
            if (showGameOver)
            {
                bool waitingForRestart = true;
                while (waitingForRestart && window.isOpen())
                {
                    Event gameOverEvent;
                    while (window.pollEvent(gameOverEvent))
                    {
                        if (gameOverEvent.type == Event::Closed)
                        {
                            window.close();
                            return 0;
                        }

                        if (gameOverEvent.type == Event::KeyPressed &&
                            gameOverEvent.key.code == Keyboard::Enter)
                        {
                            waitingForRestart = false;
                            showGameOver = false;
                           
                            restartGame = true; 
                             playagain = true;
                            // RESET variables when restarting

                           player_x = respawnX;
                           player_y = respawnY;
                           velocityY = 0;
                           onGround = false;
                           isDead = false;
                            deadAnimFrame = 0;
                          deadAnimCounter = 0;
                         playerLives = 3;
                         playerScore = 0; 
                       comboStreak = 0; 
                      levelTimer = 0.0f; 
                        levelNoDamage = true;

                        // Reset powerups
powerupCount = 0;
hasSpeedBoost = false;
hasRangeBoost = false;
hasPowerBoost = false;
speed = originalSpeed * speedMultiplier;
vacuumPower = originalVacuumPower;
                          
                        }

                        if (gameOverEvent.type == Event::KeyPressed &&
                            gameOverEvent.key.code == Keyboard::Escape)
                        {
                            window.close();
                            return 0;
                        }
                    }

                    window.clear();
                    window.draw(gameOverBGSprite); 
                    window.draw(gameOverText);
                    livesRemainingText.setString("Final Score: " + to_string(playerScore));
                    livesRemainingText.setString("You ran out of lives!");
                    window.draw(livesRemainingText);
                    window.draw(restartText);
                    window.draw(escText);
                   
                    window.display();
                }
            }
            
            if (restartGame) break; 

            // render (drawing everything once per frame)
            //window.clear();
            display_level(window, lvl, bgTex, bgSprite, blockTexture, blockSprite, height, width, cell_size);

            // 2.8 x 64 player's png width is 64
            // 2.8 x 64 player's png height is 64
            float Xoffset = (64 * scale - PlayerWidth) / 2.0f; // sprite is drawn slightly above b/c the animation frames
            float Yoffset = (64 * scale - PlayerHeight);       // I used were of size 64, 64 that is differnt from player height

            PlayerSprite.setPosition(player_x - Xoffset, player_y - Yoffset);

            window.draw(PlayerSprite);

            // --- DRAW PASS (drawOnly = true) ---
            handleVacuum(window, vacSprite, vacTexHorz, vacTexVert, 
                 player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming, 
                 enemiesX, enemiesY, enemyCount, capturedEnemies, capturedCount, MAX_CAPACITY, 1, 
                 vacFlickerTimer, showVacSprite, dt, 
                 enemyIsCaught, true,vacuumPower,
                  playerScore,  comboStreak,  comboTimer,
                   multiKillCount,  multiKillTimer,
     hasRangeBoost); 

            handleVacuum(window, vacSprite, vacTexHorz, vacTexVert, 
                 player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming, 
                 skeletonsX, skeletonsY, skeletonCount, capturedEnemies, capturedCount, MAX_CAPACITY, 2, 
                 vacFlickerTimer, showVacSprite, dt, 
                 skeletonIsCaught, true,vacuumPower,
                   playerScore, comboStreak,  comboTimer,
                   multiKillCount,  multiKillTimer,
     hasRangeBoost);

            // collision box start
            RectangleShape collBox;
            collBox.setSize(Vector2f(PlayerWidth, PlayerHeight));
            collBox.setPosition(player_x, player_y); // This is where the physics thinks you are
            collBox.setFillColor(Color::Transparent);
            collBox.setOutlineColor(Color::Red);
            collBox.setOutlineThickness(2);
            window.draw(collBox);
            // collision box end

            // Draw enemies
            for (int i = 0; i < enemyCount; i++)
            {
                EnemySprite.setPosition(enemiesX[i], enemiesY[i]);
                window.draw(EnemySprite);
            }

            // Draw skeletons with animation
            for (int i = 0; i < skeletonCount; i++)
            {
                // Update animation frame
                skeletonAnimCounter[i]++;
                if (skeletonAnimCounter[i] >= skeletonAnimSpeed)
                {
                    skeletonAnimCounter[i] = 0;
                    skeletonAnimFrame[i]++;
                    if (skeletonAnimFrame[i] >= 4) // 4 frames total
                        skeletonAnimFrame[i] = 0;
                }

                // Set the current animation frame texture
                SkeletonSprite.setTexture(skeletonWalkTex[skeletonAnimFrame[i]], true);

                // Flip sprite based on direction
                int texW = skeletonWalkTex[skeletonAnimFrame[i]].getSize().x;
                int texH = skeletonWalkTex[skeletonAnimFrame[i]].getSize().y;

                if (skeletonDirection[i] == 1) // Facing right - FLIP IT
                {
                    SkeletonSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
                }
                else // Facing left - NORMAL
                {
                    SkeletonSprite.setTextureRect(IntRect(0, 0, texW, texH));
                }

                // ADD OFFSETS to align sprite with collision box (same as player)
                float skeletonScale = 2.0f; // Same scale as SkeletonSprite.setScale(2, 2)
                float XoffsetSkeleton = (64 * skeletonScale - SkeletonWidth) / 2.0f;
                float YoffsetSkeleton = (64 * skeletonScale - SkeletonHeight);

                SkeletonSprite.setPosition(skeletonsX[i] - XoffsetSkeleton, skeletonsY[i] - YoffsetSkeleton);
                window.draw(SkeletonSprite);
            }
            
            // Draw powerups with floating animation
for (int i = 0; i < powerupCount; i++)
{
    if (!powerupActive[i]) continue;
    
    // Floating animation
    float floatOffset = sin(powerupAnimTimer[i] * 3.0f) * 5.0f;
    
    // Set texture based on type
    switch(powerupType[i])
    {
        case 1:
          powerupSprite.setTexture(speedPowerupTex);
           break;
        case 2:
         powerupSprite.setTexture(rangePowerupTex);
          break;
        case 3:
         powerupSprite.setTexture(powerPowerupTex); 
          break;
        case 4:
         powerupSprite.setTexture(lifePowerupTex);
         break;
    }
    
    powerupSprite.setPosition(powerupsX[i], powerupsY[i] + floatOffset);
    window.draw(powerupSprite);
}
            
            Text livesDisplay("Lives: " + to_string(playerLives), font, 40);
            livesDisplay.setFillColor(Color::Magenta);
            livesDisplay.setPosition(70, 0);
            window.draw(livesDisplay);
            // Update and draw score
scoreText.setString("Score: " + to_string(playerScore));
window.draw(scoreText);

// Draw combo indicator
if (comboStreak >= 3)
{
    comboText.setString("COMBO x" + to_string(comboStreak) + "!");
    window.draw(comboText);
}
      // Draw active powerup indicators
Text powerupStatus("", font, 25);
powerupStatus.setFillColor(Color::Cyan);
powerupStatus.setPosition(20, 80);

string activeEffects = "";
if (hasSpeedBoost) activeEffects += "SPEED ";
if (hasRangeBoost) activeEffects += "RANGE ";
if (hasPowerBoost) activeEffects += "POWER ";

if (activeEffects != "")
{
    powerupStatus.setString("Active: " + activeEffects);
    window.draw(powerupStatus);
}

            window.display();

        } // <--- End of while(window.isOpen())
     
       
        lvlMusic.stop();
        for (int i = 0; i < height; i++)
        {
            delete[] lvl[i];
        }
        delete[] lvl;

        
    }
}
