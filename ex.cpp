// fahad boss zindabad
// ali boss saari speeds change krdi hain. ab wo pixels/sec k hisab se move krengi chill scene he
#include <iostream>
#include <fstream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

using namespace sf;
using namespace std;

int screen_x = 1200;
int screen_y = 950;

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

        velocityY += gravity * dt; //this is dt. To smoothen out things
        if (velocityY >= terminal_Velocity)
            velocityY = terminal_Velocity;
    }
}

void playerCollision_x(char **lvl, float &player_x, float player_y, const float &speed, const int cell_size, const int Pheight, const int Pwidth, int height, int width, float dt)
{
    float offsetX_right = player_x + speed * dt;
    float offsetX_left = player_x - speed * dt;

    // --- Right Movement ---
    if (Keyboard::isKeyPressed(Keyboard::Right))
    {
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

    const float dt = 0.0167f; //dt to smooth everything
    RenderWindow window(VideoMode(screen_x, screen_y), "Tumble-POP", Style::Close | Style::Resize);

    const int cell_size = 64;
    const int height = 14;
    const int width = 18;
    char **lvl;

    Texture bgTex;
    Sprite bgSprite;
    Texture blockTexture;
    Sprite blockSprite;

    bgTex.loadFromFile("Data/bg.png");
    bgSprite.setTexture(bgTex);
    bgSprite.setPosition(0, 0);

    blockTexture.loadFromFile("Data/block1.png");
    blockSprite.setTexture(blockTexture);

    Music lvlMusic;
    lvlMusic.openFromFile("Data/mus.ogg");
    lvlMusic.setVolume(20);
    lvlMusic.setLoop(true);

    float player_x = 650;
    float player_y = 450;

    float speed = 140.f; //0.5 

    const float jumpStrength = -150.0f;
    const float gravity = 90.f; //0.6

    bool isJumping = false;

    Texture PlayerTexture;
    Sprite PlayerSprite;

    bool onGround = false;

    float offset_y = 0;
    float velocityY = 0;
    float terminal_Velocity = 300.f; // 1

    int PlayerHeight = 102;
    int PlayerWidth = 96;

    PlayerTexture.loadFromFile("Data/player.png");
    PlayerSprite.setTexture(PlayerTexture);
    PlayerSprite.setScale(3, 3);

    // --- LEVEL CREATION (YOUR ORIGINAL MAP) ---
    lvl = new char *[height];
    for (int i = 0; i < height; i += 1)
    {
        lvl[i] = new char[width];
        // IMPORTANT: Fill with empty space first to remove garbage data!
        for(int j=0; j<width; j++) lvl[i][j] = ' ';
    }

    // Paste of your original level layout
    lvl[1][3]= '-'; lvl[1][4]= '-'; lvl[1][5]= '-'; lvl[1][6]= '-'; lvl[1][7]= '-';
    lvl[1][8]= '-'; lvl[1][9]= '-'; lvl[1][10]= '-'; lvl[1][11]= '-'; lvl[1][12]= '-';
    lvl[1][13]= '-'; lvl[1][14]= '-';
   
    lvl[9][3]= '-'; lvl[9][4]= '-'; lvl[9][5]= '-'; lvl[9][6]= '-'; lvl[9][7]= '-';
    lvl[9][8]= '-'; lvl[9][9]= '-'; lvl[9][10]= '-'; lvl[9][11]= '-'; lvl[9][12]= '-';
    lvl[9][13]= '-'; lvl[9][14]= '-';
   
    lvl[8][8]= '-'; lvl[8][9]= '-';
   
    lvl[7][1]= '-'; lvl[7][2]= '-'; lvl[7][3]= '-'; lvl[7][9]= '-'; lvl[7][8]= '-';
    lvl[7][7]= '-'; lvl[7][10]= '-'; lvl[7][14]= '-'; lvl[7][15]= '-'; lvl[7][16]= '-';
   
    lvl[6][7]= '-'; lvl[6][10]= '-';
   
    // --- MODIFICATION: I changed these two to '-' so you can test jumping through them ---
    lvl[5][7]= '-'; lvl[5][10]= '-';
   
    lvl[5][3]= '-'; lvl[5][4]= '-'; lvl[5][5]= '-'; lvl[5][6]= '-'; lvl[5][11]= '-';
    lvl[5][12]= '-'; lvl[5][13]= '-'; lvl[5][14]= '-';
   
    lvl[4][7]= '-'; lvl[4][10]= '-';
    lvl[3][7]= '-'; lvl[3][10]= '-'; lvl[3][8]= '-'; lvl[3][9]= '-';
    lvl[3][1]= '-'; lvl[3][2]= '-'; lvl[3][3]= '-'; lvl[3][16]= '-'; lvl[3][15]= '-'; lvl[3][14]= '-';
   
    lvl[2][8]= '-'; lvl[2][9]= '-';

    // Floor and Sides (Restored from your code)
    for(int j=0; j<=17; j++) lvl[11][j] = '#';
    for(int i=0; i<=10; i++) { lvl[i][0] = '#'; lvl[i][17] = '#'; }
    lvl[2][8]='#';
    lvl[2][9]='#';
    lvl[8][8]='#';
    lvl[8][9]='#';
    lvl[7][7]='#';
    lvl[7][8]='#';
    lvl[7][9]='#';
    lvl[7][10]='#';
    lvl[6][7]='#';
    lvl[6][10]='#';
    lvl[5][7]='#';
    lvl[5][10]='#';
    lvl[4][7]='#';
    lvl[4][10]='#';
    lvl[3][7]='#';
    lvl[3][8]='#';
    lvl[3][9]='#';
    lvl[3][10]='#';
    
    // End of original map paste

    Event ev;
   
    while (window.isOpen())
    {
        while (window.pollEvent(ev))
        {
            if (ev.type == Event::Closed) window.close();
        }
        if (Keyboard::isKeyPressed(Keyboard::Escape)) window.close();

        // Movement (Now allows air control)
        playerCollision_x(lvl, player_x, player_y, speed, cell_size, PlayerHeight, PlayerWidth, height, width, dt);
       
        if (Keyboard::isKeyPressed(Keyboard::Up) && onGround)
        {
            velocityY = jumpStrength;
            onGround = false;
            isJumping = true;
        }

        window.clear();

        display_level(window, lvl, bgTex, bgSprite, blockTexture, blockSprite, height, width, cell_size);
        player_gravity(lvl, offset_y, velocityY, onGround, gravity, terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth, height, width, dt);
        PlayerSprite.setPosition(player_x, player_y);
        window.draw(PlayerSprite);

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
