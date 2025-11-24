// PF project
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
            PlayerSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
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

// Safety check to prevent crashing if player goes off-screen
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

int main()
{

    const float dt = 0.018f; // dt to smooth everything 0.018
    srand(time(0));          //  Initialize random seed for skeleton jump timing
    RenderWindow window(VideoMode(screen_x, screen_y), "Tumble-POP", Style::Close | Style::Resize);

    const int cell_size = 64;
    const int height = 14;
    const int width = 20; // 18
    char **lvl;

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
    
    // ------------------------------------
    // INTRO SCREEN (BEFORE CHARACTER MENU)
    // ------------------------------------

    Texture introTex;
    Sprite introSprite;

    if (!introTex.loadFromFile("Data/intro.png"))
        cout << "intro.png missing!\n";

    introSprite.setTexture(introTex);
    introSprite.setPosition(0, 0); // adjust if needed
    introSprite.setScale(1.8, 1.8);
    // Press Enter to Start text
    Font fontIntro;
    fontIntro.loadFromFile("Data/font.ttf");

    Text startText("PRESS ENTER TO START", fontIntro, 50);
    startText.setFillColor(Color::White);
    startText.setPosition(300, 800); // adjust to fit your screen

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

    // font
    Font font;
    if (!font.loadFromFile("Data/font.ttf"))
        cout << " font.ttf missing\n";

    Text infoText("", font, 30);

    infoText.setFillColor(Color::White);
    infoText.setPosition(20, 20);

    // menu screen

    Texture menuBGTexture;
    Sprite menuBGSprite;

    if (!menuBGTexture.loadFromFile("Data/menuBG.png")) // <-- your image
        cout << "Menu background missing!\n";

    menuBGSprite.setTexture(menuBGTexture);

    // OPTIONAL — make image fill the whole window
    menuBGSprite.setScale(
        float(screen_x) / menuBGTexture.getSize().x,
        float(screen_y) / menuBGTexture.getSize().y);

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

                    speedMultiplier = 1.5f;
                    vacuumPower = 1.0f;
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

                    speedMultiplier = 1.0f;
                    vacuumPower = 1.2f;
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

    float player_x = 850.0f; // 650.f
    float player_y = 450.f;

    float speed = 140.0f; // 140

    const float jumpStrength = -150.0f; // 150
    const float gravity = 90.f;         // 90

    bool isJumping = false;

    int enemyCount = 0;
    const int maxEnemyCount = 8;

    float enemiesX[maxEnemyCount];
    float enemiesY[maxEnemyCount];
    float enemySpeed[maxEnemyCount];
    int enemyDirection[maxEnemyCount];
    float platformLeftEdge[maxEnemyCount];
    float platformRightEdge[maxEnemyCount];

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

    int SkeletonHeight = 92; // 60
    int SkeletonWidth = 72;

    Texture SkeletonTexture;
    Sprite SkeletonSprite;

    SkeletonTexture.loadFromFile("Data/skeleton.png");
    SkeletonSprite.setTexture(SkeletonTexture);
    SkeletonSprite.setScale(2, 2);

    bool onGround = false;

    float offset_y = 0;
    float velocityY = 0;
    float terminal_Velocity = 300.f; // 300

    // int PlayerHeight = 102;
    // int PlayerWidth = 96;

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

                skeletonCount++;
                lvl[r][c] = ' '; // Clear the marker
            }
        }
    }
    cout << "Total skeletons: " << skeletonCount << endl;

    // End of original map paste

    Event ev;

    while (window.isOpen())
    {
        while (window.pollEvent(ev))
        {
            if (ev.type == Event::Closed)
                window.close();
        }
        if (Keyboard::isKeyPressed(Keyboard::Escape))
            window.close();

        isMoving = 0;
        // Movement - Make sure this is OUTSIDE the event loop
        playerCollision_x(lvl, player_x, player_y, speed, cell_size, PlayerHeight,
                          PlayerWidth, height, width, dt, isMoving, facing);
        updatePlayerAnimation(PlayerSprite, facing, isMoving, isDead, onGround, idleTex,
                              walkTex, jumpTex, deadTex, animFrame, deadAnimFrame, animCounter, deadAnimCounter, animSpeed, deadAnimSpeed);
        //Sprite &PlayerSprite, int facing, int isMoving, bool isDead, bool onGround,
            //               Texture &idleTex, Texture walkTex[], Texture &jumpTex, Texture &deadTex[],
              //             int &animFrame, int &deadAnimFrame, int &animCounter, int deadAnimCounter, int animSpeed
        if (Keyboard::isKeyPressed(Keyboard::Up) && onGround)
        {
            velocityY = jumpStrength;
            onGround = false;
            isJumping = true;
        }

        // Apply gravity
        player_gravity(lvl, offset_y, velocityY, onGround, gravity, terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth, height, width, dt);

        // NOW update enemies AFTER gravity
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

            collisionDetection(window, player_x, player_y, enemiesX[i], enemiesY[i], PlayerWidth, PlayerHeight, EnemyWidth, EnemyHeight, isDead);
        }

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

            // Collision detection with player
            collisionDetection(window, player_x, player_y, skeletonsX[i], skeletonsY[i],
                               PlayerWidth, PlayerHeight, SkeletonWidth, SkeletonHeight, isDead);
        }

        window.clear();
        display_level(window, lvl, bgTex, bgSprite, blockTexture, blockSprite, height, width, cell_size);

        // 2.8 x 64 player's png width is 64
        // 2.8 x 64 player's png height is 64
        float Xoffset = (64 * scale - PlayerWidth) / 2.0f; // sprite is drawn slightly above b/c the animation frames
        float Yoffset = (64 * scale - PlayerHeight);       // I used were of size 64, 64 that is differnt from player height

        PlayerSprite.setPosition(player_x - Xoffset, player_y - Yoffset);

        window.draw(PlayerSprite);

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

        // Draw skeletons
        for (int i = 0; i < skeletonCount; i++)
        {
            SkeletonSprite.setPosition(skeletonsX[i], skeletonsY[i]);
            window.draw(SkeletonSprite);
        }

        window.display();
    }

    lvlMusic.stop();
    for (int i = 0; i < height; i++)
    {
        delete[] lvl[i];
    }
    delete[] lvl;

    return 0;
}
