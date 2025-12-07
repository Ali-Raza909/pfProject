
                                                   //Ali Raza (25i-0656) , Syed Fahad Nasir (25i-0984)
                                                   //CS-G
                                                   


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
        // reset the death animation if we respawn
        deadAnimFrame = 0;

        if (!onGround) // for jumping
        {
            PlayerSprite.setTexture(jumpTex, true);
            texW = jumpTex.getSize().x;
            texH = jumpTex.getSize().y;
        }
        else if (isMoving == 1) // for walking
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
        else // when doing nothing
        {
            animFrame = 0;
            PlayerSprite.setTexture(idleTex, true);
            texW = idleTex.getSize().x;
            texH = idleTex.getSize().y;
        }

        // for flpping 
        if (facing == 1) // when face is towards left it will be noted as flipped
        {
            PlayerSprite.setTextureRect(IntRect(texW, 0, -texW, texH)); //-ve width will flip the sprite horizontally
        }
        else // when face is towards right then no flip , it will be normal
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

// function to update animations of enemy

void updateEnemyAnimation(Sprite &sprite, float dt,
                          Texture *walkTextures, Texture *suckTextures,
                          int &animFrame, int &animCounter, int animSpeed,
                          int direction, bool isCaught,
                          float x, float y, float scale,
                          int logicalWidth, int logicalHeight) 
{
    // updating animation counters
    animCounter++;
    if (animCounter >= animSpeed)
    {
        animCounter = 0;
        animFrame++;
        if (animFrame >= 4)
            animFrame = 0;
    }

    // selecting textures
    Texture *currentTex;
    if (isCaught)
        currentTex = &suckTextures[animFrame];
    else
        currentTex = &walkTextures[animFrame];

    sprite.setTexture(*currentTex, true);

    // to find the exact widht and height
    int texW = currentTex->getSize().x;
    int texH = currentTex->getSize().y;

    

    // to leave equal space on both sides of sprite inside the hitbox
    float offsetX = (logicalWidth - texW * scale) / 2.0f;

    // for height of sprite in hitbox
    float offsetY = logicalHeight - (texH * scale);

   
    sprite.setPosition(x + offsetX, y + offsetY);
    sprite.setScale(scale, scale);

    // flip Logic
    if (direction == 1) // for moving right
        sprite.setTextureRect(IntRect(texW, 0, -texW, texH));
    else // for moving Left
        sprite.setTextureRect(IntRect(0, 0, texW, texH));
}

// player box overlaps enemy box then player will be killed
bool collisionDetection(RenderWindow &window, float playerX, float playerY, float enemyX, float enemyY, float playerW, float playerH, float enemyW, float enemyH, bool &isDead)
{
    if ((playerX < enemyX + enemyW) && (playerX + playerW > enemyX) && (playerY < enemyY + enemyH) && (playerY + playerH > enemyY))
    {
        
        isDead = true;
        return true;
    }
    return false;
}

// check to prevent error when player goes out of the screen
char get_tile(char **lvl, int row, int col, int height, int width)
{
    if (row < 0 || row >= height || col < 0 || col >= width)
    {
        return ' '; // outside the map will be air
    }
    return lvl[row][col];
}

// function to draw level
void display_level(RenderWindow &window, char **lvl, Texture &bgTex, Sprite &bgSprite,
                   Texture &blockTexture, Sprite &blockSprite,
                   Texture &slopeLeft, Sprite &slopeLeftSpr,
                   Texture &slopeRight, Sprite &slopeRightSpr,
                   Texture &slopeLeftMirror, Sprite &slopeLeftMirrorSpr,
                   Texture &slopeRightMirror, Sprite &slopeRightMirrorSpr,
                   const int height, const int width, const int cell_size)
{
    window.draw(bgSprite);

    for (int i = 0; i < height; i += 1)
    {
        for (int j = 0; j < width; j += 1)
        {
            // drawing walls using # and platforms using -
            if (lvl[i][j] == '#' || lvl[i][j] == '-')
            {
                blockSprite.setPosition(j * cell_size, i * cell_size);
                window.draw(blockSprite);
            }
            // drawing left slope for lvl 2 using / or l
            
            else if (lvl[i][j] == '/' || lvl[i][j] == 'l')
            {
                slopeLeftSpr.setPosition(j * cell_size, i * cell_size);
                window.draw(slopeLeftSpr);
            }
           // drawing right slope for lvl 2 using \\ or r
            else if (lvl[i][j] == '\\' || lvl[i][j] == 'r')
            {
                slopeRightSpr.setPosition(j * cell_size, i * cell_size);
                window.draw(slopeRightSpr);
            }
            else if (lvl[i][j] == 'L')
            {
                slopeLeftMirrorSpr.setPosition(j * cell_size, i * cell_size);
                window.draw(slopeLeftMirrorSpr);
            }
            else if (lvl[i][j] == 'R')
            {
                slopeRightMirrorSpr.setPosition(j * cell_size, i * cell_size);
                window.draw(slopeRightMirrorSpr);
            }
        }
    }
}

// function for gravity , when player jumps so it doen't remain in air
void player_gravity(char **lvl, float &offset_y, float &velocityY, bool &onGround, const float &gravity, float &terminal_Velocity, float &player_x, float &player_y, const int cell_size, int &Pheight, int &Pwidth, int height, int width, float dt)
{

    float new_y = player_y + velocityY * dt;

    // collision with ceiling , moving up
    if (velocityY < 0)
    {
        char top_left = get_tile(lvl, (int)new_y / cell_size, (int)player_x / cell_size, height, width);
        char top_right = get_tile(lvl, (int)new_y / cell_size, (int)(player_x + Pwidth) / cell_size, height, width);

        // only stop if hit a solid wall that is represented by #
        if (top_left == '#' || top_right == '#')
        {
            velocityY = 0;
            new_y = ((int)new_y / cell_size + 1) * cell_size;
        }
    }

    //  collision with floor , moving down
    int feet_row = (int)(new_y + Pheight) / cell_size;
    int feet_col_left = (int)(player_x) / cell_size;
    int feet_col_right = (int)(player_x + Pwidth) / cell_size;

    bool landed = false;

    if (velocityY >= 0)
    {
        char block_left = get_tile(lvl, feet_row, feet_col_left, height, width);
        char block_right = get_tile(lvl, feet_row, feet_col_right, height, width);

        // if solid wall it will always land
        if (block_left == '#' || block_right == '#')
        {
            landed = true;
        }
        // if platfrom then only land if crossing the top edge
        else if (block_left == '-' || block_right == '-')
        {
            float block_top_pixel = feet_row * cell_size;
            const float tolerance = 4.0f;

            // to check if we are crossing the line downward or standing on it
            if ((player_y + Pheight <= block_top_pixel + tolerance) && (new_y + Pheight >= block_top_pixel))
            {
                landed = true;
            }
        }
        // if they are tiles of slope like '/', '\', 'l', or 'r' then use them as platform
        else if (block_left == '/' || block_right == '/' || block_left == '\\' || block_right == '\\' ||
                 block_left == 'l' || block_right == 'l' || block_left == 'r' || block_right == 'r' ||
                 block_left == 'L' || block_right == 'L' || block_left == 'R' || block_right == 'R')
        {
            float block_top_pixel = feet_row * cell_size;
            const float tolerance = 4.0f;
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

        velocityY += gravity * dt; // dt to smoothen out everything
        if (velocityY >= terminal_Velocity)
            velocityY = terminal_Velocity;
    }
}

// function to control player movement
void playerMovement(char **lvl, float &player_x, float player_y,
                    const float &speed, const int cell_size, const int Pheight,
                    const int Pwidth, int height, int width, float dt, int &isMoving, int &facing)
{
    float offsetX_right = player_x + speed * dt;
    float offsetX_left = player_x - speed * dt;

    // for right movement
    if (Keyboard::isKeyPressed(Keyboard::Right))
    {
        isMoving = 1;
        facing = 1;

        char top_right = get_tile(lvl, (int)player_y / cell_size, (int)(offsetX_right + Pwidth) / cell_size, height, width);
        char mid_right = get_tile(lvl, (int)(player_y + Pheight / 2) / cell_size, (int)(offsetX_right + Pwidth) / cell_size, height, width);
        char bot_right = get_tile(lvl, (int)(player_y + Pheight - 5) / cell_size, (int)(offsetX_right + Pwidth) / cell_size, height, width);

        // only stop if there is a solid wall 
        if (top_right == '#' || mid_right == '#' || bot_right == '#')
        {
            player_x = ((int)(offsetX_right + Pwidth) / cell_size) * cell_size - Pwidth - 1;
        }
        else
        {
            player_x += speed * dt;
        }
    }

    // for left movement
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

// function for counting score
void addScore(int &playerScore, int &comboStreak, float &comboTimer, int points,
              bool isDefeat, int &multiKillCount, float &multiKillTimer, const float dt)
{
    int finalPoints = points;

    // combo multiplier for deafeats
    if (isDefeat)
    {
        if (comboStreak >= 5)
            finalPoints = (int)(points * 2.0f);
        else if (comboStreak >= 3)
            finalPoints = (int)(points * 1.5f);
    }

    playerScore += finalPoints;

    // reset the timer of combo
    if (isDefeat)
    {
        comboStreak++;
        comboTimer = 0.0f;
    }
}

void checkMultiKill(int &multiKillCount, float &multiKillTimer, int &playerScore)
{
    if (multiKillCount >= 3)
        playerScore += 500; // multikill 3+ enemies
    else if (multiKillCount == 2)
        playerScore += 200; // multikill 2 enemies

    multiKillCount = 0;
    multiKillTimer = 0.0f;
}

// function for vacuum in phase 1
void handleVacuum(RenderWindow &window, Sprite &vacSprite,                      
                  Texture &texHorz, Texture &texVert,                           
                  float player_x, float player_y,                               
                  int PlayerWidth, int PlayerHeight, int vacDir, bool isActive, 
                  float *enemiesX, float *enemiesY, int &enemyCount,            
                  int *inventory, int &capturedCount, int maxCap, int enemyType,
                  float &flickerTimer, bool &showSprite, float dt,
                  bool *enemyIsCaught,
                  bool drawOnly, float vacuumPower,
                  int &playerScore, int &comboStreak, float &comboTimer,
                  int &multiKillCount, float &multiKillTimer,
                  bool hasRangeBoost)
{
    if (!isActive)
        return;
    if (capturedCount >= maxCap)
        return;

    // flicker animation
    if (drawOnly)
    {
        flickerTimer += dt;
        if (flickerTimer >= 0.05f)
        {
            showSprite = !showSprite;
            flickerTimer = 0;
        }
    }

    // calculating centers and dimensions
    int pX = (int)player_x;
    int pY = (int)player_y;
    int pCenterX = pX + PlayerWidth / 2;
    int pCenterY = pY + PlayerHeight / 2;

    float rangeMultiplier = hasRangeBoost ? 1.5f : 1.0f; // 1.5x wider if boosted
    int beamReach = 72 * vacuumPower * rangeMultiplier;
    int beamThick = 30 * vacuumPower * rangeMultiplier;

    // hitbox
    IntRect vacHitbox;

    // direction logic for vacuum , we calculate the scale factors that are based on texture size against the required beam size and multiply it by 2.0f because our global scale is 2.0f
   
   
    float scaleX_Horz = (float)beamReach / texHorz.getSize().x * 2.0f;
    float scaleY_Horz = (float)beamThick / texHorz.getSize().y * 2.0f;
    float scaleX_Vert = (float)beamThick / texVert.getSize().x * 2.0f; // width is thickness for vertical
    float scaleY_Vert = (float)beamReach / texVert.getSize().y * 2.0f; // height is reach for vertical

    if (vacDir == 0) // right
    {
        vacSprite.setTexture(texHorz, true);
        vacSprite.setTextureRect(IntRect(0, 0, texHorz.getSize().x, texHorz.getSize().y)); 
        vacSprite.setScale(scaleX_Horz, scaleY_Horz); 
        
        vacSprite.setPosition(pX + PlayerWidth, pCenterY - (beamThick * 2.0f) / 2.0f + (beamThick/2.0f)); 
        
        vacSprite.setPosition(pX + PlayerWidth, pCenterY - beamThick); // Visual alignment

        vacHitbox = IntRect(pX + PlayerWidth, pCenterY - beamThick, beamReach * 2, beamThick * 2);
    }
    else if (vacDir == 1) // left
    {
        vacSprite.setTexture(texHorz, true);
        // mirror the texture without changing size
        vacSprite.setTextureRect(IntRect(texHorz.getSize().x, 0, -((int)texHorz.getSize().x), texHorz.getSize().y)); 
        vacSprite.setScale(scaleX_Horz, scaleY_Horz); 
        
        vacSprite.setPosition(pX - (beamReach * 2), pCenterY - beamThick);

        vacHitbox = IntRect(pX - (beamReach * 2), pCenterY - beamThick, beamReach * 2, beamThick * 2);
    }
    else if (vacDir == 2) // up
    {
        vacSprite.setTexture(texVert, true);
        vacSprite.setTextureRect(IntRect(0, 0, texVert.getSize().x, texVert.getSize().y));
        vacSprite.setScale(scaleX_Vert, scaleY_Vert);
        
        vacSprite.setPosition(pCenterX - beamThick, pY - (beamReach * 2));

        vacHitbox = IntRect(pCenterX - beamThick, pY - (beamReach * 2), beamThick * 2, beamReach * 2);
    }
    else if (vacDir == 3) // down
    {
        vacSprite.setTexture(texVert, true);
        vacSprite.setTextureRect(IntRect(0, 0, texVert.getSize().x, texVert.getSize().y));
        vacSprite.setScale(scaleX_Vert, scaleY_Vert);
        
        vacSprite.setPosition(pCenterX - beamThick, pY + PlayerHeight);

        vacHitbox = IntRect(pCenterX - beamThick, pY + PlayerHeight, beamThick * 2, beamReach * 2);
    }

    // drawing pass
    if (drawOnly)
    {
        if (showSprite)
        {
            window.draw(vacSprite);
        }
        return; // exit after drawing
    }

    // logic pass , this will only work when drawonly os false
    for (int i = 0; i < enemyCount; i++)
    {
        IntRect enemyRect((int)enemiesX[i], (int)enemiesY[i], 60, 60);

        if (vacHitbox.intersects(enemyRect))
        {
            // enemy will be  marked as caught so the collision doesn't kills the player
            enemyIsCaught[i] = true;

            // pulling
            float pullSpeed = 250.0f * vacuumPower * dt; // pull speed affected by power

            if (enemiesX[i] < pCenterX)
                enemiesX[i] += pullSpeed;
            else
                enemiesX[i] -= pullSpeed;

            if (enemiesY[i] < pCenterY)
                enemiesY[i] += pullSpeed;
            else
                enemiesY[i] -= pullSpeed;

            // capture check ,  and restored to orignal distance
            float dx = (float)(pCenterX - (enemiesX[i] + 30));
            float dy = (float)(pCenterY - (enemiesY[i] + 30));
            float dist = sqrt(dx * dx + dy * dy);

            if (dist < 50.0f * vacuumPower)
            {
                inventory[capturedCount] = enemyType;
                capturedCount++;

                // give capture points only during logic pass
                if (!drawOnly)
                {
                    int capturePoints = 0;
                    if (enemyType == 1)
                        capturePoints = 50; // ghost
                    else if (enemyType == 2)
                        capturePoints = 75; // skeleton
                    else if (enemyType == 3)
                        capturePoints = 150; // invisible Man
                    else if (enemyType == 4)
                        capturePoints = 200; // chelnov

                    addScore(playerScore, comboStreak, comboTimer, capturePoints,
                             false, multiKillCount, multiKillTimer, dt);
                }

                // remove enemy from array
                enemiesX[i] = enemiesX[enemyCount - 1];
                enemiesY[i] = enemiesY[enemyCount - 1];
                enemyCount--;
                i--;
            }
        }
    }
}

// function to handle vacuum logic for phase 2
void handleVacuumPhase2(RenderWindow &window, Sprite &vacSprite,
                        Texture &texHorz, Texture &texVert,
                        float player_x, float player_y,
                        int PlayerWidth, int PlayerHeight, int vacDir, bool isActive,
                        float *enemiesX, float *enemiesY, int &enemyCount,
                        int *&inventory, int &capturedCount, 
                        int enemyType,
                        float &flickerTimer, bool &showSprite, float dt,
                        bool *enemyIsCaught,
                        bool drawOnly, float vacuumPower,
                        int &playerScore, int &comboStreak, float &comboTimer,
                        int &multiKillCount, float &multiKillTimer,
                        bool hasRangeBoost)
{
    if (!isActive)
        return;

    // visuals and hitbox
    if (drawOnly)
    {
        flickerTimer += dt;
        if (flickerTimer >= 0.05f)
        {
            showSprite = !showSprite;
            flickerTimer = 0;
        }
    }

    int pX = (int)player_x;
    int pY = (int)player_y;
    int pCenterX = pX + PlayerWidth / 2;
    int pCenterY = pY + PlayerHeight / 2;
    float rangeMultiplier = hasRangeBoost ? 1.5f : 1.0f;
    int beamReach = 72 * vacuumPower * rangeMultiplier;
    int beamThick = 30 * vacuumPower * rangeMultiplier;

    IntRect vacHitbox;
   // direction logic for vacuum , we calculate the scale factors that are based on texture size against the required beam size and multiply it by 2.0f because our global scale is 2.0f
    float scaleX_Horz = (float)beamReach / texHorz.getSize().x * 2.0f;
    float scaleY_Horz = (float)beamThick / texHorz.getSize().y * 2.0f;
    float scaleX_Vert = (float)beamThick / texVert.getSize().x * 2.0f; // width is thickness for vertical
    float scaleY_Vert = (float)beamReach / texVert.getSize().y * 2.0f; // height is reach for vertical

    if (vacDir == 0) // right
    {
        vacSprite.setTexture(texHorz, true);
        vacSprite.setTextureRect(IntRect(0, 0, texHorz.getSize().x, texHorz.getSize().y)); 
        vacSprite.setScale(scaleX_Horz, scaleY_Horz); 
        
        vacSprite.setPosition(pX + PlayerWidth, pCenterY - (beamThick * 2.0f) / 2.0f + (beamThick/2.0f)); 
        
        vacSprite.setPosition(pX + PlayerWidth, pCenterY - beamThick); 

        vacHitbox = IntRect(pX + PlayerWidth, pCenterY - beamThick, beamReach * 2, beamThick * 2);
    }
    else if (vacDir == 1) // left
    {
        vacSprite.setTexture(texHorz, true);
        // mirror texture without changing size
        vacSprite.setTextureRect(IntRect(texHorz.getSize().x, 0, -((int)texHorz.getSize().x), texHorz.getSize().y)); 
        vacSprite.setScale(scaleX_Horz, scaleY_Horz); 
        
        vacSprite.setPosition(pX - (beamReach * 2), pCenterY - beamThick);

        vacHitbox = IntRect(pX - (beamReach * 2), pCenterY - beamThick, beamReach * 2, beamThick * 2);
    }
    else if (vacDir == 2) // up
    {
        vacSprite.setTexture(texVert, true);
        vacSprite.setTextureRect(IntRect(0, 0, texVert.getSize().x, texVert.getSize().y));
        vacSprite.setScale(scaleX_Vert, scaleY_Vert);
        
        vacSprite.setPosition(pCenterX - beamThick, pY - (beamReach * 2));

        vacHitbox = IntRect(pCenterX - beamThick, pY - (beamReach * 2), beamThick * 2, beamReach * 2);
    }
    else if (vacDir == 3) //down
    {
        vacSprite.setTexture(texVert, true);
        vacSprite.setTextureRect(IntRect(0, 0, texVert.getSize().x, texVert.getSize().y));
        vacSprite.setScale(scaleX_Vert, scaleY_Vert);
        
        vacSprite.setPosition(pCenterX - beamThick, pY + PlayerHeight);

        vacHitbox = IntRect(pCenterX - beamThick, pY + PlayerHeight, beamThick * 2, beamReach * 2);
    }

    if (drawOnly)
    {
        if (showSprite)
            window.draw(vacSprite);
        return;
    }

    // logic Pass
    for (int i = 0; i < enemyCount; i++)
    {
        IntRect enemyRect((int)enemiesX[i], (int)enemiesY[i], 60, 60);

        if (vacHitbox.intersects(enemyRect))
        {
            enemyIsCaught[i] = true;

            // pull Logic
            float pullSpeed = 250.0f * vacuumPower * dt;
            if (enemiesX[i] < pCenterX)
                enemiesX[i] += pullSpeed;
            else
                enemiesX[i] -= pullSpeed;
            if (enemiesY[i] < pCenterY)
                enemiesY[i] += pullSpeed;
            else
                enemiesY[i] -= pullSpeed;

            // capture check
            float dx = (float)(pCenterX - (enemiesX[i] + 30));
            float dy = (float)(pCenterY - (enemiesY[i] + 30));
            float dist = sqrt(dx * dx + dy * dy);

            if (dist < 50.0f * vacuumPower)
            {
                // doing dynamic resizing for phase 2
                
                int *newArr = new int[capturedCount + 1];

                // copy old data
                for (int k = 0; k < capturedCount; k++)
                {
                    newArr[k] = inventory[k];
                }

                // add new item
                newArr[capturedCount] = enemyType;

                // delete old array
                if (inventory != nullptr)
                {
                    delete[] inventory;
                }

                // now point to new array
                inventory = newArr;
                capturedCount++;
                
                if (!drawOnly)
                {
                    
                    addScore(playerScore, comboStreak, comboTimer, 100,
                             false, multiKillCount, multiKillTimer, dt);
                }

                // remove enemy from world
                enemiesX[i] = enemiesX[enemyCount - 1];
                enemiesY[i] = enemiesY[enemyCount - 1];
                enemyCount--;
                i--;
            }
        }
    }
}
// function to display powerups at random locations everytime game is run
void spawnPowerup(float *powerupsX, float *powerupsY, int *powerupType,
                  bool *powerupActive, float *powerupAnimTimer,
                  int &powerupCount, int maxPowerups, char **lvl,
                  int mapWidth, int mapHeight, int cellSize)
{
    if (powerupCount >= maxPowerups)
        return;

    // random powerup type (1-4)
    int type = (rand() % 4) + 1;

    // trying to find a valid platform position
    int attempts = 0;
    int randomCol, randomRow;
    bool foundValidSpot = false;

    while (attempts < 50 && !foundValidSpot)
    {
        // for random positions , while avoiding edges
        randomCol = 2 + (rand() % (mapWidth - 4));
        randomRow = 2 + (rand() % (mapHeight - 3));

        // to check if there is a platform or wall below
        char tileBelow = lvl[randomRow + 1][randomCol];
        // check if current position is empty
        char currentTile = lvl[randomRow][randomCol];

        // spot will be valid if current position is empty and there is a platform or wall below
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


// Function to apply diagonal sliding on slopes 
void applySliding(char **lvl, float &player_x, float &player_y, int PlayerHeight, int PlayerWidth,
                  int cell_size, int height, int width, float dt, bool &onGround, float &velocityY)
{
    if (!onGround)
        return;

    int feet_row = (int)(player_y + PlayerHeight) / cell_size;
    int feet_col_left = (int)(player_x) / cell_size;
    int feet_col_right = (int)(player_x + PlayerWidth) / cell_size;

    char tile_left = get_tile(lvl, feet_row, feet_col_left, height, width);
    char tile_right = get_tile(lvl, feet_row, feet_col_right, height, width);

    float slideSpeedX = 80.0f; // horizontal slide speed
    float slideSpeedY = 80.0f; // vertical slide speed 

    bool onSlope = false;
    int slideDirectionX = 0; // -1 = left, 1 = right

    // down right slope (\) using 'l' and 'r' , slide right and down
    if (tile_left == 'l' || tile_right == 'l' || tile_left == 'r' || tile_right == 'r')
    {
        onSlope = true;
        slideDirectionX = 1; // slide right
    }
    // down left slope (/) using 'L' and 'R' , slide left and down
    else if (tile_left == 'L' || tile_right == 'L' || tile_left == 'R' || tile_right == 'R')
    {
        onSlope = true;
        slideDirectionX = -1; // slide left
    }

    if (onSlope)
    {
        // move horizontally
        player_x += slideSpeedX * slideDirectionX * dt;

        // move down the slope 
        float newY = player_y + slideSpeedY * dt;

        // heck if we can move down means that we are not hitting the floor
        int new_feet_row = (int)(newY + PlayerHeight) / cell_size;
        char below_left = get_tile(lvl, new_feet_row, (int)player_x / cell_size, height, width);
        char below_right = get_tile(lvl, new_feet_row, (int)(player_x + PlayerWidth) / cell_size, height, width);

        // only slide down if not hitting solid floor
        if (below_left != '#' && below_right != '#')
        {
            player_y = newY;

            // keep player attached to slope by giving small downward velocity, this prevents floating
            
            if (velocityY < slideSpeedY)
            {
                velocityY = slideSpeedY * 0.5f;
            }
        }
    }
}

// function to make randomized slope for level 2 using l and r and it will also create horizontal platforms that will not cut the slope

void generateLevel2Design(char **lvl, int platHeight, int platWidth)
{
    // clear the level
    for (int i = 0; i < platHeight; i++)
    {
        for (int j = 0; j < platWidth; j++)
        {
            lvl[i][j] = ' ';
        }
    }

    // creating solid boundaries 
    for (int j = 0; j < platWidth; j++)
    {
        lvl[11][j] = '#';
    }

    // left wall (column 0)
    for (int i = 0; i < platHeight; i++)
    {
        lvl[i][0] = '#';
    }

    // right wall (column 18 for width=20)
    for (int i = 0; i < platHeight; i++)
    {
        lvl[i][platWidth - 2] = '#';
    }

    // generating slanted platforms
    int rowMinBound = 2, rowMaxBound = 4;
    int colMinBound = 3, colMaxBound = 6;

    int randTopRow = rowMinBound + rand() % (rowMaxBound - rowMinBound + 1);
    int randTopCol = colMinBound + rand() % (colMaxBound - colMinBound + 1);

    int slantLength = 5 + rand() % 4;
    int direction = rand() % 2;

    if (direction == 1)
    {
        randTopCol = (platWidth - 4) - rand() % 4;
    }

    if (direction == 0)
    {
        if (randTopCol + slantLength > platWidth - 3)
            slantLength = platWidth - 3 - randTopCol;
    }
    else
    {
        if (randTopCol - slantLength < 2)
            slantLength = randTopCol - 2;
    }

    if (slantLength < 4)
        slantLength = 4;

    for (int step = 0; step < slantLength; step++)
    {
        int row = randTopRow + step;
        int col;

        if (direction == 0)
        {
            col = randTopCol + step;
            if (row < 11 && col > 0 && col < platWidth - 2)
            {
                lvl[row][col] = 'l';
                if (row + 1 < 11)
                    lvl[row + 1][col] = 'r';
            }
        }
        else
        {
            col = randTopCol - step;
            if (row < 11 && col > 0 && col < platWidth - 2)
            {
                lvl[row][col] = 'L';
                if (row + 1 < 11)
                    lvl[row + 1][col] = 'R';
            }
        }
    }

    // for generating horizontal platfroms
    int minPlatformLength = 3;
    int platformRows[] = {2, 4, 6, 8, 9}; 
    int numPlatformRows = 5;

    for (int p = 0; p < numPlatformRows; p++)
    {
        int row = platformRows[p];
        if (row <= 0 || row >= 11)
            continue;

        bool rowHasSlant = false;
        bool aboveRowHasSlant = false;

        for (int j = 0; j < platWidth; j++)
        {
            if (lvl[row][j] == 'l' || lvl[row][j] == 'r')
                rowHasSlant = true;

            if (row > 0 && (lvl[row - 1][j] == 'l' || lvl[row - 1][j] == 'r'))
                aboveRowHasSlant = true;
        }

        int sectionStart = -1;

        for (int j = 1; j < platWidth - 2; j++)
        {
            bool canPlace = (lvl[row][j] == ' ');

            if (row > 0 && (lvl[row - 1][j] == 'l' || lvl[row - 1][j] == 'r'))
                canPlace = false;
            if (row < 11 && (lvl[row + 1][j] == 'l' || lvl[row + 1][j] == 'r'))
                canPlace = false;

            if (canPlace && sectionStart == -1)
            {
                sectionStart = j;
            }
            else if (!canPlace && sectionStart != -1)
            {
                int sectionLength = j - sectionStart;
                if (sectionLength >= minPlatformLength)
                {
                    int platStart = sectionStart + rand() % 2;
                    int platLength = minPlatformLength + rand() % (sectionLength - minPlatformLength + 1);
                    if (platStart + platLength > j)
                        platLength = j - platStart;

                    for (int k = platStart; k < platStart + platLength && k < platWidth - 2; k++)
                    {
                        if (lvl[row - 1][k] == ' ' && row != 10)
                            lvl[row - 1][k] = '-';
                        else if (row == 10 && lvl[row][k] == ' ')
                            lvl[row][k] = '-';
                    }
                }
                sectionStart = -1;
            }
        }

        if (sectionStart != -1)
        {
            int sectionLength = (platWidth - 2) - sectionStart;
            if (sectionLength >= minPlatformLength)
            {
                int platStart = sectionStart;
                int platLength = minPlatformLength + rand() % (sectionLength - minPlatformLength + 1);

                for (int k = platStart; k < platStart + platLength && k < platWidth - 2; k++)
                {
                    if (lvl[row - 1][k] == ' ' && row != 9)
                        lvl[row - 1][k] = '-';
                    else if (row == 9 && lvl[row][k] == ' ')
                        lvl[row][k] = '-';
                }
            }
        }
    }

    // make sure platfroms are with gaps
    bool hasTopLeft = false;
    for (int j = 1; j <= 4; j++)
        if (lvl[2][j] == '-')
            hasTopLeft = true;

    if (!hasTopLeft)
    {
        // create platform with a gap (only columns 1-2, leaving 3-4 empty)
        for (int j = 1; j <= 2; j++)
            if (lvl[1][j] == ' ')
                lvl[1][j] = '-';
    }

    bool hasTopRight = false;
    for (int j = platWidth - 6; j <= platWidth - 3; j++)
        if (lvl[2][j] == '-')
            hasTopRight = true;

    if (!hasTopRight)
    {
        // create platform with a gap (only last 2 columns, leaving others empty)
        for (int j = platWidth - 4; j <= platWidth - 3; j++)
            if (lvl[1][j] == ' ')
                lvl[1][j] = '-';
    }

    // bottom platforms with gaps
    for (int j = 1; j <= 3; j++) 
        if (lvl[9][j] == ' ')
            lvl[9][j] = '-';

    for (int j = platWidth - 5; j <= platWidth - 3; j++) 
        if (lvl[9][j] == ' ')
            lvl[9][j] = '-';

  
    cout << "=== Level 2 Design Generated ===" << endl;
    cout << "Slant direction: " << (direction == 0 ? "Down-Right (\\)" : "Down-Left (/)") << endl;
    cout << "Slant start: row " << randTopRow << ", col " << randTopCol << endl;
    cout << "Slant length: " << slantLength << " tiles" << endl;
}

// function to spawn enemies in level two in 4 waves based on waveNumber
void spawnWave(int waveNumber, int cell_size,
               float *enemiesX, float *enemiesY, float *enemySpeed, int *enemyDirection,
               float *platformLeftEdge, float *platformRightEdge, int &enemyCount,
               float *skeletonsX, float *skeletonsY, float *skeletonSpeed, int *skeletonDirection,
               float *skeletonVelocityY, bool *skeletonOnGround, float *skeletonJumpTimer,
               float *skeletonJumpCooldown, bool *skeletonShouldJump, int *skeletonStableFrames,
               int *skeletonAnimFrame, int *skeletonAnimCounter, int &skeletonCount,
               float *invisiblesX, float *invisiblesY, float *invisibleSpeed, int *invisibleDirection,
               float *invisibleVelocityY, bool *invisibleOnGround, bool *invisibleIsVisible,
               float *invisibleVisibilityTimer, float *invisibleTeleportTimer, int &invisibleCount,
               float *chelnovsX, float *chelnovsY, float *chelnovSpeed, int *chelnovDirection,
               float *chelnovVelocityY, bool *chelnovOnGround, float *chelnovShootTimer,
               bool *chelnovIsShooting, float *chelnovShootPhaseTimer, int &chelnovCount,
               int maxEnemyCount, int maxSkeletonCount, int maxInvisibleCount, int maxChelnovCount)
{
    cout << "Spawning Wave " << (waveNumber + 1) << endl;

    switch (waveNumber)
    {
    case 0: // Wave 1 has 2 Ghosts + 3 Skeletons
    {
       
        float ghostSpawnX[] = {(float)(4 * cell_size), (float)(14 * cell_size)};
        float ghostSpawnY[] = {(float)(0 * cell_size), (float)(0 * cell_size)};
        for (int i = 0; i < 2 && enemyCount < maxEnemyCount; i++)
        {
            enemiesX[enemyCount] = ghostSpawnX[i];
            enemiesY[enemyCount] = ghostSpawnY[i];
            enemySpeed[enemyCount] = 15.f;
            enemyDirection[enemyCount] = 1;
            platformLeftEdge[enemyCount] = (float)(1 * cell_size + 10);
            platformRightEdge[enemyCount] = (float)(17 * cell_size - 10);
            enemyCount++;
        }

       
        float skelSpawnX[] = {(float)(5 * cell_size), (float)(13 * cell_size), (float)(9 * cell_size)};
        float skelSpawnY[] = {(float)(0 * cell_size), (float)(0 * cell_size), (float)(2 * cell_size)};
        for (int i = 0; i < 3 && skeletonCount < maxSkeletonCount; i++)
        {
            skeletonsX[skeletonCount] = skelSpawnX[i];
            skeletonsY[skeletonCount] = skelSpawnY[i];
            skeletonSpeed[skeletonCount] = 40.f;
            skeletonDirection[skeletonCount] = 1;
            skeletonVelocityY[skeletonCount] = 0;
            skeletonOnGround[skeletonCount] = false;
            skeletonJumpTimer[skeletonCount] = 0.f;
            skeletonJumpCooldown[skeletonCount] = 1.5f + (rand() % 20) / 10.0f;
            skeletonShouldJump[skeletonCount] = false;
            skeletonStableFrames[skeletonCount] = 0;
            skeletonAnimFrame[skeletonCount] = 0;
            skeletonAnimCounter[skeletonCount] = 0;
            skeletonCount++;
        }
        break;
    }

    case 1: // Wave 2 has 2 Ghosts + 3 Skeletons
    {
        
        float ghostSpawnX[] = {(float)(3 * cell_size), (float)(15 * cell_size)};
        float ghostSpawnY[] = {(float)(2 * cell_size), (float)(2 * cell_size)};
        for (int i = 0; i < 2 && enemyCount < maxEnemyCount; i++)
        {
            enemiesX[enemyCount] = ghostSpawnX[i];
            enemiesY[enemyCount] = ghostSpawnY[i];
            enemySpeed[enemyCount] = 15.f;
            enemyDirection[enemyCount] = 1;
            platformLeftEdge[enemyCount] = (float)(1 * cell_size + 10);
            platformRightEdge[enemyCount] = (float)(17 * cell_size - 10);
            enemyCount++;
        }

        
        float skelSpawnX[] = {(float)(5 * cell_size), (float)(13 * cell_size), (float)(3 * cell_size)};
        float skelSpawnY[] = {(float)(2 * cell_size), (float)(2 * cell_size), (float)(4 * cell_size)};
        for (int i = 0; i < 3 && skeletonCount < maxSkeletonCount; i++)
        {
            skeletonsX[skeletonCount] = skelSpawnX[i];
            skeletonsY[skeletonCount] = skelSpawnY[i];
            skeletonSpeed[skeletonCount] = 40.f;
            skeletonDirection[skeletonCount] = 1;
            skeletonVelocityY[skeletonCount] = 0;
            skeletonOnGround[skeletonCount] = false;
            skeletonJumpTimer[skeletonCount] = 0.f;
            skeletonJumpCooldown[skeletonCount] = 1.5f + (rand() % 20) / 10.0f;
            skeletonShouldJump[skeletonCount] = false;
            skeletonStableFrames[skeletonCount] = 0;
            skeletonAnimFrame[skeletonCount] = 0;
            skeletonAnimCounter[skeletonCount] = 0;
            skeletonCount++;
        }
        break;
    }

    case 2: // Wave 3 has 3 Skeletons + 2 Chelnovs + 2 Invisible Men
    {
        
        float skelSpawnX[] = {(float)(15 * cell_size), (float)(4 * cell_size), (float)(14 * cell_size)};
        float skelSpawnY[] = {(float)(4 * cell_size), (float)(6 * cell_size), (float)(6 * cell_size)};
        for (int i = 0; i < 3 && skeletonCount < maxSkeletonCount; i++)
        {
            skeletonsX[skeletonCount] = skelSpawnX[i];
            skeletonsY[skeletonCount] = skelSpawnY[i];
            skeletonSpeed[skeletonCount] = 40.f;
            skeletonDirection[skeletonCount] = 1;
            skeletonVelocityY[skeletonCount] = 0;
            skeletonOnGround[skeletonCount] = false;
            skeletonJumpTimer[skeletonCount] = 0.f;
            skeletonJumpCooldown[skeletonCount] = 1.5f + (rand() % 20) / 10.0f;
            skeletonShouldJump[skeletonCount] = false;
            skeletonStableFrames[skeletonCount] = 0;
            skeletonAnimFrame[skeletonCount] = 0;
            skeletonAnimCounter[skeletonCount] = 0;
            skeletonCount++;
        }

        
        float chelSpawnX[] = {(float)(12 * cell_size), (float)(6 * cell_size)};
        float chelSpawnY[] = {(float)(0 * cell_size), (float)(2 * cell_size)};
        for (int i = 0; i < 2 && chelnovCount < maxChelnovCount; i++)
        {
            chelnovsX[chelnovCount] = chelSpawnX[i];
            chelnovsY[chelnovCount] = chelSpawnY[i];
            chelnovSpeed[chelnovCount] = 30.f;
            chelnovDirection[chelnovCount] = 1;
            chelnovVelocityY[chelnovCount] = 0;
            chelnovOnGround[chelnovCount] = false;
            chelnovShootTimer[chelnovCount] = 0.f;
            chelnovIsShooting[chelnovCount] = false;
            chelnovShootPhaseTimer[chelnovCount] = 0.f;
            chelnovCount++;
        }

        
        float invisSpawnX[] = {(float)(6 * cell_size), (float)(13 * cell_size)};
        float invisSpawnY[] = {(float)(0 * cell_size), (float)(4 * cell_size)};
        for (int i = 0; i < 2 && invisibleCount < maxInvisibleCount; i++)
        {
            invisiblesX[invisibleCount] = invisSpawnX[i];
            invisiblesY[invisibleCount] = invisSpawnY[i];
            invisibleSpeed[invisibleCount] = 25.f;
            invisibleDirection[invisibleCount] = 1;
            invisibleVelocityY[invisibleCount] = 0;
            invisibleOnGround[invisibleCount] = false;
            invisibleIsVisible[invisibleCount] = true;
            invisibleVisibilityTimer[invisibleCount] = 0.f;
            invisibleTeleportTimer[invisibleCount] = 0.f;
            invisibleCount++;
        }
        break;
    }

    case 3: // Wave 4  has 2 Chelnovs + 1 Invisible Man
    {
       
        float chelSpawnX[] = {(float)(14 * cell_size), (float)(3 * cell_size)};
        float chelSpawnY[] = {(float)(4 * cell_size), (float)(8 * cell_size)};
        for (int i = 0; i < 2 && chelnovCount < maxChelnovCount; i++)
        {
            chelnovsX[chelnovCount] = chelSpawnX[i];
            chelnovsY[chelnovCount] = chelSpawnY[i];
            chelnovSpeed[chelnovCount] = 30.f;
            chelnovDirection[chelnovCount] = 1;
            chelnovVelocityY[chelnovCount] = 0;
            chelnovOnGround[chelnovCount] = false;
            chelnovShootTimer[chelnovCount] = 0.f;
            chelnovIsShooting[chelnovCount] = false;
            chelnovShootPhaseTimer[chelnovCount] = 0.f;
            chelnovCount++;
        }

        
        if (invisibleCount < maxInvisibleCount)
        {
            invisiblesX[invisibleCount] = (float)(5 * cell_size);
            invisiblesY[invisibleCount] = (float)(4 * cell_size);
            invisibleSpeed[invisibleCount] = 25.f;
            invisibleDirection[invisibleCount] = 1;
            invisibleVelocityY[invisibleCount] = 0;
            invisibleOnGround[invisibleCount] = false;
            invisibleIsVisible[invisibleCount] = true;
            invisibleVisibilityTimer[invisibleCount] = 0.f;
            invisibleTeleportTimer[invisibleCount] = 0.f;
            invisibleCount++;
        }
        break;
    }
    }
}

//  this function will generate Level 2 map with optional enemy spawning
void generateLevel2Map(char **lvl, int height, int width, int cell_size,
                       float *enemiesX, float *enemiesY, float *enemySpeed, int *enemyDirection,
                       float *platformLeftEdge, float *platformRightEdge, int &enemyCount,
                       float *skeletonsX, float *skeletonsY, float *skeletonSpeed, int *skeletonDirection,
                       float *skeletonVelocityY, bool *skeletonOnGround, float *skeletonJumpTimer,
                       float *skeletonJumpCooldown, bool *skeletonShouldJump, int *skeletonStableFrames,
                       int *skeletonAnimFrame, int *skeletonAnimCounter, int &skeletonCount,
                       float *invisiblesX, float *invisiblesY, float *invisibleSpeed, int *invisibleDirection,
                       float *invisibleVelocityY, bool *invisibleOnGround, bool *invisibleIsVisible,
                       float *invisibleVisibilityTimer, float *invisibleTeleportTimer, int &invisibleCount,
                       float *chelnovsX, float *chelnovsY, float *chelnovSpeed, int *chelnovDirection,
                       float *chelnovVelocityY, bool *chelnovOnGround, float *chelnovShootTimer,
                       bool *chelnovIsShooting, float *chelnovShootPhaseTimer, int &chelnovCount,
                       bool spawnAllEnemies = true) // NEW PARAMETER with default value
{
    // reset enemy counts
    enemyCount = 0;
    skeletonCount = 0;
    invisibleCount = 0;
    chelnovCount = 0;

    // using randomized platform generating function
    
    generateLevel2Design(lvl, height, width);

    //  only spawn enemies if requested
    if (spawnAllEnemies)
    {
        // spawn 4 ghosts , using explicit float casts
        float ghostSpawnX[] = {(float)(4 * cell_size), (float)(14 * cell_size), (float)(3 * cell_size), (float)(15 * cell_size)};
        float ghostSpawnY[] = {(float)(0 * cell_size), (float)(0 * cell_size), (float)(2 * cell_size), (float)(2 * cell_size)};
        for (int i = 0; i < 4 && enemyCount < 10; i++)
        {
            enemiesX[enemyCount] = ghostSpawnX[i];
            enemiesY[enemyCount] = ghostSpawnY[i];
            enemySpeed[enemyCount] = 15.f;
            enemyDirection[enemyCount] = 1;
            platformLeftEdge[enemyCount] = (float)(1 * cell_size + 10);
            platformRightEdge[enemyCount] = (float)(17 * cell_size - 10);
            enemyCount++;
        }

        // Spawn 9 skeletons 
        float skelSpawnX[] = {(float)(5 * cell_size), (float)(13 * cell_size), (float)(5 * cell_size), (float)(13 * cell_size), (float)(3 * cell_size),
                              (float)(15 * cell_size), (float)(4 * cell_size), (float)(14 * cell_size), (float)(9 * cell_size)};
        float skelSpawnY[] = {(float)(0 * cell_size), (float)(0 * cell_size), (float)(2 * cell_size), (float)(2 * cell_size), (float)(4 * cell_size),
                              (float)(4 * cell_size), (float)(6 * cell_size), (float)(6 * cell_size), (float)(8 * cell_size)};
        for (int i = 0; i < 9 && skeletonCount < 10; i++)
        {
            skeletonsX[skeletonCount] = skelSpawnX[i];
            skeletonsY[skeletonCount] = skelSpawnY[i];
            skeletonSpeed[skeletonCount] = 40.f;
            skeletonDirection[skeletonCount] = 1;
            skeletonVelocityY[skeletonCount] = 0;
            skeletonOnGround[skeletonCount] = false;
            skeletonJumpTimer[skeletonCount] = 0.f;
            skeletonJumpCooldown[skeletonCount] = 1.5f + (rand() % 20) / 10.0f;
            skeletonShouldJump[skeletonCount] = false;
            skeletonStableFrames[skeletonCount] = 0;
            skeletonAnimFrame[skeletonCount] = 0;
            skeletonAnimCounter[skeletonCount] = 0;
            skeletonCount++;
        }

        // Spawn 3 invisible men 
        float invisSpawnX[] = {(float)(6 * cell_size), (float)(5 * cell_size), (float)(13 * cell_size)};
        float invisSpawnY[] = {(float)(0 * cell_size), (float)(4 * cell_size), (float)(6 * cell_size)};
        for (int i = 0; i < 3 && invisibleCount < 5; i++)
        {
            invisiblesX[invisibleCount] = invisSpawnX[i];
            invisiblesY[invisibleCount] = invisSpawnY[i];
            invisibleSpeed[invisibleCount] = 25.f;
            invisibleDirection[invisibleCount] = 1;
            invisibleVelocityY[invisibleCount] = 0;
            invisibleOnGround[invisibleCount] = false;
            invisibleIsVisible[invisibleCount] = true;
            invisibleVisibilityTimer[invisibleCount] = 0.f;
            invisibleTeleportTimer[invisibleCount] = 0.f;
            invisibleCount++;
        }

        // Spawn 4 chelnovs 
        float chelSpawnX[] = {(float)(12 * cell_size), (float)(6 * cell_size), (float)(14 * cell_size), (float)(3 * cell_size)};
        float chelSpawnY[] = {(float)(0 * cell_size), (float)(2 * cell_size), (float)(4 * cell_size), (float)(8 * cell_size)};
        for (int i = 0; i < 4 && chelnovCount < 5; i++)
        {
            chelnovsX[chelnovCount] = chelSpawnX[i];
            chelnovsY[chelnovCount] = chelSpawnY[i];
            chelnovSpeed[chelnovCount] = 30.f;
            chelnovDirection[chelnovCount] = 1;
            chelnovVelocityY[chelnovCount] = 0;
            chelnovOnGround[chelnovCount] = false;
            chelnovShootTimer[chelnovCount] = 0.f;
            chelnovIsShooting[chelnovCount] = false;
            chelnovShootPhaseTimer[chelnovCount] = 0.f;
            chelnovCount++;
        }
    }

    cout << "Level 2 generated with " << enemyCount << " ghosts, " << skeletonCount << " skeletons, "
         << invisibleCount << " invisible men, " << chelnovCount << " chelnovs" << endl;
}

int main()
{
    bool playagain = true;
    bool firstrun = true;
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
        const float multiKillWindow = 1.0f; // 1 second window for multikills

        // level tracking variables
        int currentLevel = 1;
        int selectedStartLevel = 1; // track which level player selected from menu
        bool showStageClear = false;

        //  character type flag for Phase 2 (360 rotation for yellow)
        bool isYellowCharacter = false;

        // wave spawning system variables
        bool useWaveSpawning = false;
        int currentWave = 0;
        int maxWaves = 4;
        float waveTimer = 0.0f;
        float timeBetweenWaves = 5.0f; // 5 seconds between waves
        bool waveSpawned[4] = {false, false, false, false};

       

        // Boss variables
        int bossHealth = 6; // boss starts with 6 health
        int maxBossHealth = 6;
        float bossX = 0;
        float bossY = 0;
        int bossWidth = 200;
        int bossHeight = 180;
        bool bossIsAngry = false; // becomes true when health <= 2
        bool bossDefeated = false;

        // minion variables that are dynamically spawned by boss
        int maxMinions = 20;
        float *minionsX = NULL;
        float *minionsY = NULL;
        float *minionSpeed = NULL;
        int *minionDirection = NULL;
        float *minionVelocityY = NULL;
        bool *minionOnGround = NULL;
        bool *minionIsCaught = NULL;
        bool *minionFollowingPlayer = NULL; // for angry mode
        int minionCount = 0;
        int minionWidth = 48;
        int minionHeight = 48;
        float minionSpawnTimer = 0.0f;
        float minionSpawnInterval = 3.0f; // spawn minions every 3 seconds

        // tentacle variables using dynamic array that resizes
        int maxTentacles = 10;
        float *tentaclesX = NULL;
        float *tentaclesY = NULL;
        int *tentacleWidth = NULL;
        int *tentacleHeight = NULL;
        float *tentacleTimer = NULL;    // how long tentacle has been active
        float *tentacleDuration = NULL; // how long tentacle will stay
        bool *tentacleActive = NULL;
        int *tentacleTexIndex = NULL; // to store which sprite (0-11) this tentacle uses
        int tentacleCount = 0;
        float tentacleSpawnTimer = 0.0f;
        float tentacleSpawnInterval = 4.0f; // check to spawn tentacle every 4 seconds

        // level 3 specific dimensions (1.5x scaling)
        int level3Height = 21;   // 14 * 1.5 = 21
        int level3Width = 30;    // 20 * 1.5 = 30
        int level3CellSize = 42; // 64 / 1.5 = 42 to fit same screen

        // calculating floor row to be visible on screen
        int floorRow = level3Height - 3; 
        float floorY = floorRow * level3CellSize;

        // Boss level map
        char **bossLvl = NULL;

        
        bool potActive = true; // pot is active until destroyed
        int potHealth = 4;     // pot takes 4 hits to destroy
        int potMaxHealth = 4;
        float potX = 0;
        float potY = 0;
        int potWidth = 80;
        int potHeight = 80;
        float potEnemySpawnTimer = 0.0f;
        float potEnemySpawnInterval = 8.0f; // spawn enemy every 4 seconds
        bool potDestroyed = false;          // visual feedback when destroyed
        float potDestroyTimer = 0.0f;       // timer for destruction animation

        // cloud variables 
        float cloudX = 0;
        float cloudY = 0;
        int cloudWidth = 150;
        int cloudHeight = 50;
        float cloudMinY = 150.0f;     // top bound for cloud movement
        float cloudMaxY = 700.0f;     // bottom bound for cloud movement
        float cloudSpeed = 40.0f;     // cloud movement speed
        int cloudDirection = 1;       // 1 = moving down, -1 = moving up
        bool cloudIsPlatform = false; // it will becomes true after pot is destroyed

        // flag for when boss will appear
        bool bossAppeared = false;

        // dynamic arrays for pot enemies
        float *potEnemiesX = NULL;
        float *potEnemiesY = NULL;
        float *potEnemySpeed = NULL;
        int *potEnemyDirection = NULL;
        float *potEnemyVelocityY = NULL;
        bool *potEnemyOnGround = NULL;
        float *potEnemyVelocityX = NULL; 
        bool *potEnemyIsCaught = NULL;
        int *potEnemyType = NULL; // 1=ghost, 2=skeleton, 3=invisible, 4=chelnov

        int *potEnemyAnimFrame = NULL;   // to track the current frame (0-3)
        int *potEnemyAnimCounter = NULL; // to track speed

        int potEnemyCount = 0;
        int potEnemyCapacity = 0; // current allocated capacity

        // additional behavior arrays for pot enemies such as skeleton jump, invisible visibility, chelnov shooting
        float *potEnemyJumpTimer = NULL;
        bool *potEnemyShouldJump = NULL;
        int *potEnemyStableFrames = NULL;
        bool *potEnemyIsVisible = NULL;        // for invisible man
        float *potEnemyVisibilityTimer = NULL; // for invisible man
        float *potEnemyTeleportTimer = NULL;   // for invisible man
        float *potEnemyShootTimer = NULL;      // for chelnov
        bool *potEnemyIsShooting = NULL;       // for chelnov

        // pot enemy projectile for chelnov
        const int maxPotEnemyProjectiles = 20;
        float potEnemyProjX[20];
        float potEnemyProjY[20];
        int potEnemyProjDirection[20];
        bool potEnemyProjActive[20];
        int potEnemyProjCount = 0;

        int potEnemyProjAnimFrame[20];
        int potEnemyProjAnimCounter[20];

        // dynamic captured enemies array
        int *dynamicCapturedEnemies = NULL;
        int dynamicCapturedCount = 0;
        int dynamicCapturedCapacity = 0;

        const float dt = 0.018f; // dt to smooth everything 0.018
        srand(time(0));          //  initialize random seed for skeleton jump timing
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
         
         
         // intro screen

        if (firstrun) // so the intro screen runs only once
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
            firstrun = false;
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
        gameOverText.setPosition(300, 300);

        Text livesRemainingText("", font, 40);
        livesRemainingText.setFillColor(Color::White);
        livesRemainingText.setPosition(420, 450);

        Text restartText("Press ENTER to continue...", font, 50);
        restartText.setFillColor(Color::Yellow);
        restartText.setPosition(320, 520);

        Text escText("Press ESC to Exit", font, 45);
        escText.setFillColor(Color::Magenta);
        escText.setPosition(430, 600);

        // stage Clear screen text
        Text stageClearText("STAGE CLEAR!", font, 100);
        stageClearText.setFillColor(Color::Green);
        stageClearText.setPosition(250, 200);

        Text stageBonusText("", font, 40);
        stageBonusText.setFillColor(Color::Yellow);
        stageBonusText.setPosition(350, 350);

        Text stageScoreText("", font, 45);
        stageScoreText.setFillColor(Color::White);
        stageScoreText.setPosition(380, 420);

        Text nextLevelText("Press ENTER for Level 2", font, 50);
        nextLevelText.setFillColor(Color::Cyan);
        nextLevelText.setPosition(280, 550);

        // level indicator text
        Text levelText("LEVEL 1", font, 40);
        levelText.setFillColor(Color::White);
        levelText.setPosition(screen_x / 2 - 80, 10);

        // menu screen
        Texture menuBGTexture;
        Sprite menuBGSprite;

        if (!menuBGTexture.loadFromFile("Data/menuBG.png"))
            cout << "Menu background missing!\n";

        menuBGSprite.setTexture(menuBGTexture);

        if (!gameOverBGTexture.loadFromFile("Data/gameover.png"))
            cout << "Game Over background missing!\n";

        gameOverBGSprite.setTexture(gameOverBGTexture);

        gameOverBGSprite.setScale(
            float(screen_x) / gameOverBGTexture.getSize().x,
            float(screen_y) / gameOverBGTexture.getSize().y);

        menuBGSprite.setScale(
            float(screen_x) / menuBGTexture.getSize().x,
            float(screen_y) / menuBGTexture.getSize().y);

        Text scoreText("Score: 0", font, 35);
        scoreText.setFillColor(Color::Yellow);
        scoreText.setPosition(screen_x - 250, 10);

        Text comboText("", font, 30);
        comboText.setFillColor(Color::Cyan);
        comboText.setPosition(screen_x - 250, 50);

        Text title("Game Menu", font, 100);
        title.setFillColor(Color::Magenta);
        title.setPosition(400, 200);

        Text subtitle("Press 1 for Yellow (Strong Vacuum) ", font, 50);
        subtitle.setFillColor(Color::Yellow);
        subtitle.setPosition(120, 400);

        Text subtitle2(" Press 2 for Green (Fast)", font, 50);
        subtitle2.setFillColor(Color::Green);
        subtitle2.setPosition(100, 500);

        Text subtitle3(" Press Esc to EXIT", font, 50);
        subtitle3.setFillColor(Color::Red);
        subtitle3.setPosition(100, 600);

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
                        
                        isYellowCharacter = true;

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
                        
                        isYellowCharacter = false;

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

       // level selection menu
        Text levelSelectTitle("SELECT LEVEL", font, 100);
        levelSelectTitle.setFillColor(Color::Cyan);
        levelSelectTitle.setPosition(400, 200);

        Text level1Option("Press 1 - Level 1 (Classic)", font, 50);
        level1Option.setFillColor(Color::White);
        level1Option.setPosition(100, 400);

        Text level2Option("Press 2 - Level 2 (Slopes & Waves)", font, 50);
        level2Option.setFillColor(Color::White);
        level2Option.setPosition(100, 500);

        Text level3Option("Press 3 - Boss Level (Octopus)", font, 50);
        level3Option.setFillColor(Color(255, 100, 100)); // Reddish for boss
        level3Option.setPosition(100, 600);

        Text backOption("Press ESC to go back", font, 40);
        backOption.setFillColor(Color::Red);
        backOption.setPosition(100, 700);

        // selection indicator for arrow key navigation
        int menuSelectedLevel = 1;
        Text selectionIndicator(">", font, 50);
        selectionIndicator.setFillColor(Color::Yellow);
        selectionIndicator.setPosition(110, 400);

        bool levelSelected = false;

       
        while (window.isOpen() && !levelSelected)
        {
            Event e;
            while (window.pollEvent(e))
            {
                if (e.type == Event::Closed)
                    window.close();

                if (e.type == Event::KeyPressed)
                {
                    // direct number key selection
                    if (e.key.code == Keyboard::Num1)
                    {
                        selectedStartLevel = 1;
                        currentLevel = 1;
                        levelSelected = true;
                        cout << "Level 1 selected!" << endl;
                    }
                    else if (e.key.code == Keyboard::Num2)
                    {
                        selectedStartLevel = 2;
                        currentLevel = 2;
                        useWaveSpawning = true;
                        levelSelected = true;
                        cout << "Level 2 selected!" << endl;
                    }
                    else if (e.key.code == Keyboard::Num3)
                    {
                        // Boss Level
                        selectedStartLevel = 3;
                        currentLevel = 3;
                        levelSelected = true;
                        cout << "Boss Level (Level 3) selected!" << endl;
                    }
                    // arrow key navigation
                    else if (e.key.code == Keyboard::Up)
                    {
                        menuSelectedLevel--;
                        if (menuSelectedLevel < 1)
                            menuSelectedLevel = 3;
                    }
                    else if (e.key.code == Keyboard::Down)
                    {
                        menuSelectedLevel++;
                        if (menuSelectedLevel > 3)
                            menuSelectedLevel = 1;
                    }
                    // pres enter to confirm arrow selection
                    else if (e.key.code == Keyboard::Enter)
                    {
                        if (menuSelectedLevel == 1)
                        {
                            selectedStartLevel = 1;
                            currentLevel = 1;
                            levelSelected = true;
                            cout << "Level 1 selected!" << endl;
                        }
                        else if (menuSelectedLevel == 2)
                        {
                            selectedStartLevel = 2;
                            currentLevel = 2;
                            useWaveSpawning = true;
                            levelSelected = true;
                            cout << "Level 2 selected!" << endl;
                        }
                        else if (menuSelectedLevel == 3)
                        {
                            selectedStartLevel = 3;
                            currentLevel = 3;
                            levelSelected = true;
                            cout << "Boss Level (Level 3) selected!" << endl;
                        }
                    }
                    else if (e.key.code == Keyboard::Escape)
                    {
                        window.close();
                        return 0;
                    }
                }
            }

            // update selection indicator position
            if (menuSelectedLevel == 1)
                selectionIndicator.setPosition(240, 300);
            else if (menuSelectedLevel == 2)
                selectionIndicator.setPosition(160, 400);
            else if (menuSelectedLevel == 3)
                selectionIndicator.setPosition(140, 500);

            // to highlight selected option
            level1Option.setFillColor(menuSelectedLevel == 1 ? Color::Yellow : Color::White);
            level2Option.setFillColor(menuSelectedLevel == 2 ? Color::Yellow : Color::White);
            level3Option.setFillColor(menuSelectedLevel == 3 ? Color::Yellow : Color(255, 100, 100));

           //drwaing menu
            window.clear(Color::Black);
            window.draw(menuBGSprite);
            window.draw(levelSelectTitle);
            window.draw(selectionIndicator);
            window.draw(level1Option);
            window.draw(level2Option);
            window.draw(level3Option);
            window.draw(backOption);

            // show which character was selected on level selection window
            Text charInfo("", font, 30);
            charInfo.setFillColor(Color::Green);
            charInfo.setPosition(20, 20);
            if (isYellowCharacter)
                charInfo.setString("Character: Yellow (Strong Vacuum)");
            else
                charInfo.setString("Character: Green (Fast)");
            window.draw(charInfo);

            window.display();
        }

        if (!levelSelected)
            return 0;
        

        Texture bgTex;
        Sprite bgSprite;
        Texture blockTexture;
        Sprite blockSprite;

        Texture blockTexture2;
        Sprite blockSprite2;
        Texture bgTex2;
        Sprite bgSprite2;

        bgTex.loadFromFile("Data/bg.png");
        bgSprite.setTexture(bgTex);
        bgSprite.setScale(2.0f, 2.0f);

        blockTexture.loadFromFile("Data/block1.png");
        blockSprite.setTexture(blockTexture);

        
        if (!bgTex2.loadFromFile("Data/bg2.png"))
        {
            cout << "ERROR: bg2.png failed to load! Using bg.png as fallback.\n";
            bgTex2.loadFromFile("Data/bg.png"); 
        }

        bgSprite2.setTexture(bgTex2);
        bgSprite2.setScale(
            float(screen_x) / bgTex2.getSize().x,
            float(screen_y) / bgTex2.getSize().y);

        
        blockTexture2.loadFromFile("Data/block2.png"); 
        blockSprite2.setTexture(blockTexture2);

        
        Texture slopeLeftTexture, slopeRightTexture;
        Sprite slopeLeftSprite, slopeRightSprite;

        if (!slopeLeftTexture.loadFromFile("Data/blocks/sloperight.png"))
            cout << "slope_left.png missing!\n";
        if (!slopeRightTexture.loadFromFile("Data/blocks/slopeleft.png"))
            cout << "slope_right.png missing!\n";

        slopeLeftSprite.setTexture(slopeLeftTexture);
        slopeRightSprite.setTexture(slopeRightTexture);

        // for down left slope new sprites are used 

        Texture slopeLeftMirrorTexture, slopeRightMirrorTexture;
        Sprite slopeLeftMirrorSprite, slopeRightMirrorSprite;

        if (!slopeLeftMirrorTexture.loadFromFile("Data/blocks/slopeleft1.png"))
            cout << "slopeleft_mirror.png missing!\n";
        if (!slopeRightMirrorTexture.loadFromFile("Data/blocks/sloperight1.png"))
            cout << "sloperight_mirror.png missing!\n";

        slopeLeftMirrorSprite.setTexture(slopeLeftMirrorTexture);
        slopeRightMirrorSprite.setTexture(slopeRightMirrorTexture);

        Music lvlMusic;
        lvlMusic.openFromFile("Data/mus.ogg");
        lvlMusic.setVolume(20);
        lvlMusic.setLoop(true);
        lvlMusic.play();

        
        Music lvl2Music;
        if (!lvl2Music.openFromFile("Data/mus2.ogg"))
        {
            lvl2Music.openFromFile("Data/mus.ogg");
        }
        lvl2Music.setVolume(20);
        lvl2Music.setLoop(true);

        
        
        Texture bgTex3;
        Sprite bgSprite3;
        if (!bgTex3.loadFromFile("Data/bg3.png"))
        {
            cout << "bg3.png missing! Using bg.png as fallback.\n";
            bgTex3.loadFromFile("Data/bg.png");
        }
        bgSprite3.setTexture(bgTex3);
        bgSprite3.setScale(
            float(screen_x) / bgTex3.getSize().x,
            float(screen_y) / bgTex3.getSize().y);

        
        Texture blockTexture3;
        Sprite blockSprite3;
        if (!blockTexture3.loadFromFile("Data/block3.PNG"))
        {
            cout << "block3.png missing! Using block1.png as fallback.\n";
            blockTexture3.loadFromFile("Data/block1.png");
        }
        blockSprite3.setTexture(blockTexture3);

        
        Music lvl3Music;
        if (!lvl3Music.openFromFile("Data/mus3.ogg"))
        {
            cout << "mus3.ogg missing! Using mus.ogg as fallback.\n";
            lvl3Music.openFromFile("Data/mus.ogg");
        }
        lvl3Music.setVolume(25);
        lvl3Music.setLoop(true);

        
        Texture bossTexture;
        Sprite bossSprite;
        if (!bossTexture.loadFromFile("Data/octopus/Octopus.png"))
        {
            cout << "octopus.png missing!\n";
        }
        bossSprite.setTexture(bossTexture);
        bossSprite.setScale(3.0f, 3.0f);

        
        Texture bossAngryTexture;
        if (!bossAngryTexture.loadFromFile("Data/boss/octopus_angry.png"))
        {
            cout << "octopus_angry.png missing! Will use color tint.\n";
        }

        
        Texture tentacleTexArray[12];
        Sprite tentacleSprite;

        
        tentacleTexArray[0].loadFromFile("Data/octopus/tentacle1.png");
        tentacleTexArray[1].loadFromFile("Data/octopus/tentacle2.png");
        tentacleTexArray[2].loadFromFile("Data/octopus/tentacle3.png");
        tentacleTexArray[3].loadFromFile("Data/octopus/tentacle4.png");
        tentacleTexArray[4].loadFromFile("Data/octopus/tentacle5.png");
        tentacleTexArray[5].loadFromFile("Data/octopus/tentacle6.png");
        tentacleTexArray[6].loadFromFile("Data/octopus/tentacle7.png");
        tentacleTexArray[7].loadFromFile("Data/octopus/tentacle8.png");
        tentacleTexArray[8].loadFromFile("Data/octopus/tentacle9.png");
        tentacleTexArray[9].loadFromFile("Data/octopus/tentacle10.png");
        tentacleTexArray[10].loadFromFile("Data/octopus/tentacle11.png");
        tentacleTexArray[11].loadFromFile("Data/octopus/tentacle12.png");

        tentacleSprite.setScale(2.0f, 2.0f);

        
        Texture minionTexture;
        Sprite minionSprite;
        if (!minionTexture.loadFromFile("Data/octopus/min1.png"))
        {
            cout << "minion.png missing! Using ghost as fallback.\n";
            minionTexture.loadFromFile("Data/ghost.png");
        }
        minionSprite.setTexture(minionTexture);
        minionSprite.setScale(1.5f, 1.5f);

        
        Texture minionRollTex[4];
        if (!minionRollTex[0].loadFromFile("Data/boss/minion_roll1.png"))
        {
            cout << "minion_roll textures missing! Using ghost roll as fallback.\n";
            minionRollTex[0].loadFromFile("Data/ghostRoll/roll1.png");
            minionRollTex[1].loadFromFile("Data/ghostRoll/roll2.png");
            minionRollTex[2].loadFromFile("Data/ghostRoll/roll3.png");
            minionRollTex[3].loadFromFile("Data/ghostRoll/roll4.png");
        }
        else
        {
            minionRollTex[1].loadFromFile("Data/boss/minion_roll2.png");
            minionRollTex[2].loadFromFile("Data/boss/minion_roll3.png");
            minionRollTex[3].loadFromFile("Data/boss/minion_roll4.png");
        }

        
        Texture potTexture;
        Sprite potSprite;
        if (!potTexture.loadFromFile("Data/pot.png"))
        {
            cout << "pot.png missing!\n";
        }
        potSprite.setTexture(potTexture);
        potSprite.setScale(1.7f, 1.7f);

        Texture cloudTexture;
        Sprite cloudSprite;
        if (!cloudTexture.loadFromFile("Data/cloud.png"))
        {
            cout << "cloud.png missing!\n";
        }
        cloudSprite.setTexture(cloudTexture);
        cloudSprite.setScale(2.5f, 2.3f); 

        
        float player_x = 850.0f;
        float player_y = 450.f;

        float speed = 140.0f * speedMultiplier;

        float jumpStrength = -150.0f;
        const float gravity = 90.f;

        bool isJumping = false;

        // ghosts
        int enemyCount = 0;
        const int maxEnemyCount = 10;

        float enemiesX[maxEnemyCount];
        float enemiesY[maxEnemyCount];
        float enemySpeed[maxEnemyCount];
        int enemyDirection[maxEnemyCount];
        float platformLeftEdge[maxEnemyCount];
        float platformRightEdge[maxEnemyCount];
        bool enemyIsCaught[maxEnemyCount];

       
        int EnemyHeight = 60;
        int EnemyWidth = 72;

        
        int SkeletonHeight = 92;
        int SkeletonWidth = 72;

        
        int InvisibleHeight = 80;
        int InvisibleWidth = 60;

        
        int ChelnovHeight = 90;
        int ChelnovWidth = 60;

        Texture EnemyTexture;
        Sprite EnemySprite;

        EnemyTexture.loadFromFile("Data/ghost.png");
        EnemySprite.setTexture(EnemyTexture);
        EnemySprite.setScale(2, 2);

        // Skeleton enemies
        int skeletonCount = 0;
        const int maxSkeletonCount = 10;

        float skeletonsX[maxSkeletonCount];
        float skeletonsY[maxSkeletonCount];
        float skeletonSpeed[maxSkeletonCount];
        int skeletonDirection[maxSkeletonCount];
        float skeletonVelocityY[maxSkeletonCount];
        bool skeletonOnGround[maxSkeletonCount];
        float skeletonJumpTimer[maxSkeletonCount];
        float skeletonJumpCooldown[maxSkeletonCount];
        bool skeletonShouldJump[maxSkeletonCount];
        int skeletonStableFrames[maxSkeletonCount];
        bool skeletonIsCaught[maxSkeletonCount];

        Texture skeletonWalkTex[4];
        int skeletonAnimFrame[maxSkeletonCount];
        int skeletonAnimCounter[maxSkeletonCount];
        int skeletonAnimSpeed = 8;

        Texture SkeletonTexture;
        Sprite SkeletonSprite;

        SkeletonTexture.loadFromFile("Data/skeleton.png");
        SkeletonSprite.setTexture(SkeletonTexture);
        SkeletonSprite.setScale(2, 2);

        skeletonWalkTex[0].loadFromFile("Data/skeletonWalk/walk1.png");
        skeletonWalkTex[1].loadFromFile("Data/skeletonWalk/walk2.png");
        skeletonWalkTex[2].loadFromFile("Data/skeletonWalk/walk3.png");
        skeletonWalkTex[3].loadFromFile("Data/skeletonWalk/walk4.png");

        // Invisible Man  (Level 2 only)
        int invisibleCount = 0;
        const int maxInvisibleCount = 5;

        float invisiblesX[maxInvisibleCount];
        float invisiblesY[maxInvisibleCount];
        float invisibleSpeed[maxInvisibleCount];
        int invisibleDirection[maxInvisibleCount];
        float invisibleVelocityY[maxInvisibleCount];
        bool invisibleOnGround[maxInvisibleCount];
        bool invisibleIsCaught[maxInvisibleCount];
        bool invisibleIsVisible[maxInvisibleCount];
        float invisibleVisibilityTimer[maxInvisibleCount];
        float invisibleTeleportTimer[maxInvisibleCount];

        Texture InvisibleTexture;
        Sprite InvisibleSprite;

        if (!InvisibleTexture.loadFromFile("Data/invisibleMan/walk1.png"))
        {
            InvisibleTexture.loadFromFile("Data/ghost.png");
        }
        InvisibleSprite.setTexture(InvisibleTexture);
        InvisibleSprite.setScale(2, 2);

        // Chelnov (Level 2 only)
        int chelnovCount = 0;
        const int maxChelnovCount = 5;

        float chelnovsX[maxChelnovCount];
        float chelnovsY[maxChelnovCount];
        float chelnovSpeed[maxChelnovCount];
        int chelnovDirection[maxChelnovCount];
        float chelnovVelocityY[maxChelnovCount];
        bool chelnovOnGround[maxChelnovCount];
        bool chelnovIsCaught[maxChelnovCount];
        float chelnovShootTimer[maxChelnovCount];
        bool chelnovIsShooting[maxChelnovCount];
        float chelnovShootPhaseTimer[maxChelnovCount];

        Texture ChelnovTexture;
        Sprite ChelnovSprite;

        if (!ChelnovTexture.loadFromFile("Data/chelnov/walk1.png"))
        {
            ChelnovTexture.loadFromFile("Data/skeleton.png");
        }
        ChelnovSprite.setTexture(ChelnovTexture);
        ChelnovSprite.setScale(2, 2);

        // chelnov projectiles
        const int maxChelnovProjectiles = 10;
        float chelnovProjX[maxChelnovProjectiles];
        float chelnovProjY[maxChelnovProjectiles];
        int chelnovProjDirection[maxChelnovProjectiles];
        bool chelnovProjActive[maxChelnovProjectiles];
        int chelnovProjCount = 0;

        int chelnovProjAnimFrame[maxChelnovProjectiles];
        int chelnovProjAnimCounter[maxChelnovProjectiles];

        for (int i = 0; i < maxChelnovProjectiles; i++)
            chelnovProjActive[i] = false;

        Texture chelnovProjTex[4];
        Sprite chelnovProjSprite;

        // fireball of chelnov
        if (!chelnovProjTex[0].loadFromFile("Data/chelnov/fireball1.png"))
            chelnovProjTex[0].loadFromFile("Data/ghost.png");
        if (!chelnovProjTex[1].loadFromFile("Data/chelnov/fireball2.png"))
            chelnovProjTex[1] = chelnovProjTex[0];
        if (!chelnovProjTex[2].loadFromFile("Data/chelnov/fireball3.png"))
            chelnovProjTex[2] = chelnovProjTex[0];
        if (!chelnovProjTex[3].loadFromFile("Data/chelnov/fireball4.png"))
            chelnovProjTex[3] = chelnovProjTex[0];

        chelnovProjSprite.setScale(2.5f, 2.5f);

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
        float terminal_Velocity = 300.f;

        
        const int maxPowerups = 4;
        int powerupCount = 0;

        float powerupsX[maxPowerups];
        float powerupsY[maxPowerups];
        int powerupType[maxPowerups];
        bool powerupActive[maxPowerups];
        float powerupAnimTimer[maxPowerups];

        int PowerupWidth = 48;
        int PowerupHeight = 48;

        float speedBoostTimer = 0.0f;
        float rangeBoostTimer = 0.0f;
        float powerBoostTimer = 0.0f;
        const float powerupDuration = 10.0f;

        bool hasSpeedBoost = false;
        bool hasRangeBoost = false;
        bool hasPowerBoost = false;

        float originalSpeed = 140.0f;
        float originalVacuumPower = 1.0f;

        Texture speedPowerupTex, rangePowerupTex, powerPowerupTex, lifePowerupTex;
        Sprite powerupSprite;

        if (!speedPowerupTex.loadFromFile("Data/speed.png"))
            cout << "Speed powerup texture missing!\n";
        if (!rangePowerupTex.loadFromFile("Data/range.png"))
            cout << "Range powerup texture missing!\n";
        if (!powerPowerupTex.loadFromFile("Data/power.png"))
            cout << "Power powerup texture missing!\n";
        if (!lifePowerupTex.loadFromFile("Data/life.png"))
            cout << "Life powerup texture missing!\n";

        powerupSprite.setScale(2.0f, 2.0f);

        // for vacuum
        Texture vacTexHorz, vacTexVert;
        if (!vacTexHorz.loadFromFile("Data/horizontalVacuum.png"))
            cout << "Horizontal Vacuum texture missing\n";
        if (!vacTexVert.loadFromFile("Data/verticalVacuum.png"))
            cout << "Vertical vacuum texture missing\n";

        Sprite vacSprite;
        vacSprite.setScale(2.0f, 2.0f);

        int vacDirection = 0;
        bool isVacuuming = false;
        float vacFlickerTimer = 0.0f;
        bool showVacSprite = true;

        int MAX_CAPACITY = 3;
        const int MAX_CAPTURED_ARRAY = 5;
        int capturedEnemies[MAX_CAPTURED_ARRAY];

        // phase 2 Dynamic Variables
        dynamicCapturedEnemies = nullptr;
        dynamicCapturedCount = 0;
        int capturedCount = 0;

        // system for projectiles
        const int MAX_PROJECTILES = 10;
        float projectilesX[MAX_PROJECTILES];
        float projectilesY[MAX_PROJECTILES];
        int projectileType[MAX_PROJECTILES];
        int projectileDirection[MAX_PROJECTILES];
        float projectileVelocityY[MAX_PROJECTILES];
        bool projectileActive[MAX_PROJECTILES];
        bool projectileOnGround[MAX_PROJECTILES];
        int projectileCount = 0;
        float projectileSpeed = 70.0f;
        int releaseDirection = 0;

        int ProjectileWidth = 50;
        int ProjectileHeight = 50;

        Texture ghostRollTex[4];
        ghostRollTex[0].loadFromFile("Data/ghostRoll/roll1.png");
        ghostRollTex[1].loadFromFile("Data/ghostRoll/roll2.png");
        ghostRollTex[2].loadFromFile("Data/ghostRoll/roll3.png");
        ghostRollTex[3].loadFromFile("Data/ghostRoll/roll4.png");

        Texture skeletonRollTex[4];
        skeletonRollTex[0].loadFromFile("Data/skeletonRoll/roll1.png");
        skeletonRollTex[1].loadFromFile("Data/skeletonRoll/roll2.png");
        skeletonRollTex[2].loadFromFile("Data/skeletonRoll/roll3.png");
        skeletonRollTex[3].loadFromFile("Data/skeletonRoll/roll4.png");

        int projectileAnimFrame[MAX_PROJECTILES];
        int projectileAnimCounter[MAX_PROJECTILES];
        int projectileAnimSpeed = 10;
        float projectileLifespan[MAX_PROJECTILES];
        const float MAX_PROJECTILE_LIFE = 8.0f;
        Sprite projectileSprite;
        projectileSprite.setScale(2.0f, 2.0f);

        for (int i = 0; i < MAX_PROJECTILES; i++)
        {
            projectileActive[i] = false;
            projectileAnimFrame[i] = 0;
            projectileAnimCounter[i] = 0;
        }

        // burst mode
        bool burstModeActive = false;
        int burstFrameCounter = 0;
        const int BURST_FRAME_DELAY = 10;
        int burstReleaseDirection = 0;
        int burstPlayerFacing = 0;

        int bottomFloorRow = 11;
        float bottomFloorLeftEdge = 1 * cell_size;
        float bottomFloorRightEdge = 17 * cell_size + cell_size;

        // lvl creation
        lvl = new char *[height];
        for (int i = 0; i < height; i += 1)
        {
            lvl[i] = new char[width];
            for (int j = 0; j < width; j++)
                lvl[i][j] = ' ';
        }

        // ghost animation
        Texture ghostWalkTex[4];
        Texture ghostSuckTex[4]; 

       
        ghostWalkTex[0].loadFromFile("Data/ghostWalk/walk1.png");
        ghostWalkTex[1].loadFromFile("Data/ghostWalk/walk2.png");
        ghostWalkTex[2].loadFromFile("Data/ghostWalk/walk3.png");
        ghostWalkTex[3].loadFromFile("Data/ghostWalk/walk4.png");

       
        ghostSuckTex[0].loadFromFile("Data/ghostSuck/suck1.png");
        ghostSuckTex[1].loadFromFile("Data/ghostSuck/suck2.png");
        ghostSuckTex[2].loadFromFile("Data/ghostSuck/suck3.png");
        ghostSuckTex[3].loadFromFile("Data/ghostSuck/suck4.png");

        int ghostAnimFrame[maxEnemyCount];
        int ghostAnimCounter[maxEnemyCount];
        for (int i = 0; i < maxEnemyCount; i++)
        {
            ghostAnimFrame[i] = 0;
            ghostAnimCounter[i] = 0;
        }

        
        Texture skeletonSuckTex[4];
        skeletonSuckTex[0].loadFromFile("Data/skeletonSuck/suck1.png");
        skeletonSuckTex[1].loadFromFile("Data/skeletonSuck/suck2.png");
        skeletonSuckTex[2].loadFromFile("Data/skeletonSuck/suck3.png");
        skeletonSuckTex[3].loadFromFile("Data/skeletonSuck/suck4.png");

        Texture invisibleWalkTex[4]; 
        Texture invisibleSuckTex[4];

        
        invisibleWalkTex[0].loadFromFile("Data/invisibleMan/walk1.png");
        invisibleWalkTex[1].loadFromFile("Data/invisibleMan/walk2.png"); 
        invisibleWalkTex[2].loadFromFile("Data/invisibleMan/walk1.png");
        invisibleWalkTex[3].loadFromFile("Data/invisibleMan/walk2.png");

       
        invisibleSuckTex[0].loadFromFile("Data/invisibleMan/suck1.png");
        invisibleSuckTex[1].loadFromFile("Data/invisibleMan/suck2.png");
        invisibleSuckTex[2].loadFromFile("Data/invisibleMan/suck3.png");
        invisibleSuckTex[3].loadFromFile("Data/invisibleMan/suck4.png");

        int invisibleAnimFrame[maxInvisibleCount];
        int invisibleAnimCounter[maxInvisibleCount];
        for (int i = 0; i < maxInvisibleCount; i++)
        {
            invisibleAnimFrame[i] = 0;
            invisibleAnimCounter[i] = 0;
        }

        
        Texture chelnovWalkTex[4]; 
        Texture chelnovSuckTex[4];

        
        chelnovWalkTex[0].loadFromFile("Data/chelnov/walk1.png");
        chelnovWalkTex[1].loadFromFile("Data/chelnov/walk2.png");
        chelnovWalkTex[2].loadFromFile("Data/chelnov/walk1.png");
        chelnovWalkTex[3].loadFromFile("Data/chelnov/walk2.png");

        
        chelnovSuckTex[0].loadFromFile("Data/chelnov/suck1.png");
        chelnovSuckTex[1].loadFromFile("Data/chelnov/suck2.png");
        chelnovSuckTex[2].loadFromFile("Data/chelnov/suck3.png");
        chelnovSuckTex[3].loadFromFile("Data/chelnov/suck4.png");

        int chelnovAnimFrame[maxChelnovCount];
        int chelnovAnimCounter[maxChelnovCount];
        for (int i = 0; i < maxChelnovCount; i++)
        {
            chelnovAnimFrame[i] = 0;
            chelnovAnimCounter[i] = 0;
        }

        // intialize level based on what was selected from the  menu
        if (currentLevel == 1)
        {
            
            cout << "Initializing Level 1..." << endl;

            
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

            // floor and slides
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

            // enemy spawn positions
            lvl[0][5] = 'e';
            lvl[0][12] = 'e';
            lvl[2][2] = 'e';
            lvl[2][15] = 'e';
            lvl[4][4] = 'e';
            lvl[4][13] = 'e';
            lvl[8][6] = 'e';
            lvl[8][10] = 'e';
            lvl[0][7] = 's';
            lvl[0][10] = 's';
            lvl[2][4] = 's';
            lvl[2][13] = 's';

            // initialize ghosts from markers
            for (int r = 0; r < height; r++)
            {
                for (int c = 0; c < width; c++)
                {
                    if (lvl[r][c] == 'e' && enemyCount < maxEnemyCount)
                    {
                        int platformRow = r + 1;
                        char below = get_tile(lvl, platformRow, c, height, width);
                        if (below == '-' || below == '#')
                        {
                            enemiesX[enemyCount] = c * cell_size;
                            enemiesY[enemyCount] = r * cell_size;
                            int leftEdge = c + 1;
                            int rightEdge = c + 1;
                            while (leftEdge > 0 && (lvl[platformRow][leftEdge - 1] == '-' || lvl[platformRow][leftEdge - 1] == '#'))
                                leftEdge--;
                            while (rightEdge < width - 1 && (lvl[platformRow][rightEdge + 1] == '-' || lvl[platformRow][rightEdge + 1] == '#'))
                                rightEdge++;
                            platformLeftEdge[enemyCount] = leftEdge * cell_size + 10;
                            platformRightEdge[enemyCount] = (rightEdge + 1) * cell_size - 48 - 10;
                            enemySpeed[enemyCount] = 15.f;
                            enemyDirection[enemyCount] = 1;
                            enemyCount++;
                        }
                        lvl[r][c] = ' ';
                    }
                }
            }
            cout << "Level 1: Total ghosts: " << enemyCount << endl;

            // initialize skeletons from markers
            for (int r = 0; r < height; r++)
            {
                for (int c = 0; c < width; c++)
                {
                    if (lvl[r][c] == 's' && skeletonCount < maxSkeletonCount)
                    {
                        skeletonsX[skeletonCount] = c * cell_size;
                        skeletonsY[skeletonCount] = r * cell_size;
                        skeletonSpeed[skeletonCount] = 40.f;
                        skeletonDirection[skeletonCount] = 1;
                        skeletonVelocityY[skeletonCount] = 0;
                        skeletonOnGround[skeletonCount] = false;
                        skeletonJumpTimer[skeletonCount] = 0.f;
                        skeletonJumpCooldown[skeletonCount] = 1.5f + (rand() % 20) / 10.0f;
                        skeletonShouldJump[skeletonCount] = false;
                        skeletonStableFrames[skeletonCount] = 0;
                        skeletonAnimFrame[skeletonCount] = 0;
                        skeletonAnimCounter[skeletonCount] = 0;
                        skeletonCount++;
                        lvl[r][c] = ' ';
                    }
                }
            }
            cout << "Level 1: Total skeletons: " << skeletonCount << endl;

            
            MAX_CAPACITY = 3;

            
            lvlMusic.play();
        }
        else if (currentLevel == 2)
        {
            
            cout << "Initializing Level 2..." << endl;

            
            generateLevel2Map(lvl, height, width, cell_size,
                              enemiesX, enemiesY, enemySpeed, enemyDirection,
                              platformLeftEdge, platformRightEdge, enemyCount,
                              skeletonsX, skeletonsY, skeletonSpeed, skeletonDirection,
                              skeletonVelocityY, skeletonOnGround, skeletonJumpTimer,
                              skeletonJumpCooldown, skeletonShouldJump, skeletonStableFrames,
                              skeletonAnimFrame, skeletonAnimCounter, skeletonCount,
                              invisiblesX, invisiblesY, invisibleSpeed, invisibleDirection,
                              invisibleVelocityY, invisibleOnGround, invisibleIsVisible,
                              invisibleVisibilityTimer, invisibleTeleportTimer, invisibleCount,
                              chelnovsX, chelnovsY, chelnovSpeed, chelnovDirection,
                              chelnovVelocityY, chelnovOnGround, chelnovShootTimer,
                              chelnovIsShooting, chelnovShootPhaseTimer, chelnovCount,
                              false); // false tells to don't spawn all enemies rather use wave system

            
            MAX_CAPACITY = 5;

            
            useWaveSpawning = true;
            currentWave = 0;
            waveTimer = 0.0f;
            for (int w = 0; w < 4; w++)
                waveSpawned[w] = false;

            
            lvl2Music.play();

            cout << "Level 2 initialized with wave spawning system" << endl;
        }
        else if (currentLevel == 3)
        {
            
            cout << "Initializing Boss Level (Level 3)..." << endl;

            // allocating dynamic arrays for minions 

            minionsX = NULL;
            minionsY = NULL;
            minionSpeed = NULL;
            minionDirection = NULL;
            minionVelocityY = NULL;
            minionOnGround = NULL;
            minionIsCaught = NULL;
            minionFollowingPlayer = NULL;
            minionCount = 0;

            // allocating dynamic arrays for tentacles
            tentaclesX = NULL;
            tentaclesY = NULL;
            tentacleWidth = NULL;
            tentacleHeight = NULL;
            tentacleTimer = NULL;
            tentacleDuration = NULL;
            tentacleActive = NULL;
            tentacleCount = 0;

            
            potActive = true;
            potHealth = potMaxHealth;
            potDestroyed = false;
            potDestroyTimer = 0.0f;
            potEnemySpawnTimer = 0.0f;

            // cloud will be at top center
            cloudX = screen_x / 2 - cloudWidth / 2;
            cloudY = cloudMinY;
            cloudDirection = 1; // start moving down
            cloudIsPlatform = false;

            // place pot on top of cloud
            potX = cloudX + cloudWidth / 2 - potWidth / 2;
            potY = cloudY - potHeight;

            // initialize pot enemy arrays 
            potEnemyCount = 0;
            potEnemyCapacity = 0;
            potEnemiesX = NULL;
            potEnemiesY = NULL;
            potEnemySpeed = NULL;
            potEnemyDirection = NULL;
            potEnemyVelocityY = NULL;
            potEnemyOnGround = NULL;
            potEnemyVelocityX = NULL; 
            potEnemyIsCaught = NULL;
            potEnemyType = NULL;

            // boss will not appear until pot is destroyed
            bossAppeared = false;
            bossHealth = maxBossHealth;
            bossIsAngry = false;
            bossDefeated = false;
            bossX = screen_x / 2 - bossWidth / 2;
            bossY = -bossHeight; // start off-screen and it will move down when appearing

            // reset timers
            minionSpawnTimer = 0.0f;
            tentacleSpawnTimer = 0.0f;

            

            // Boss level map (separate from regular lvl)
            bossLvl = new char *[level3Height];
            for (int i = 0; i < level3Height; i++)
            {
                bossLvl[i] = new char[level3Width];
                for (int j = 0; j < level3Width; j++)
                    bossLvl[i][j] = ' ';
            }

           
            for (int j = 0; j < level3Width; j++)
                bossLvl[level3Height - 3][j] = '#';

            for (int i = 0; i < level3Height - 3; i++)
            {
                bossLvl[i][0] = '#';
                bossLvl[i][level3Width - 2] = '#';
            }

            // platforms
            for (int j = 2; j < 8; j++)
                bossLvl[level3Height - 8][j] = '-';
            for (int j = level3Width - 8; j < level3Width - 2; j++)
                bossLvl[level3Height - 8][j] = '-';
            for (int j = 12; j < 18; j++)
                bossLvl[level3Height - 6][j] = '-';
            for (int j = 3; j < 9; j++)
                bossLvl[level3Height - 12][j] = '-';
            for (int j = level3Width - 9; j < level3Width - 3; j++)
                bossLvl[level3Height - 12][j] = '-';
            for (int j = 11; j < 19; j++)
                bossLvl[level3Height - 15][j] = '-';

            // initializing dynamic captured enemies array for Phase 2
            dynamicCapturedCount = 0;
            dynamicCapturedCapacity = 0;
            dynamicCapturedEnemies = NULL;

            // initializing pot enemy projectile array
            potEnemyProjCount = 0;
            for (int i = 0; i < maxPotEnemyProjectiles; i++)
            {
                potEnemyProjActive[i] = false;
            }

            // no capacity limit in boss level (dynamic capture)
            MAX_CAPACITY = 999;
            MAX_CAPACITY = 999;

            
            lvlMusic.stop();
            lvl2Music.stop();
            lvl3Music.play();

            cout << "Boss Level initialized with Pot/Cloud system!" << endl;
            cout << "Pot Health: " << potHealth << "/" << potMaxHealth << endl;
            cout << "Destroy the pot to make the Boss appear!" << endl;
            cout << "Level dimensions: " << level3Width << "x" << level3Height << " (cell size: " << level3CellSize << ")" << endl;
            cout << "Floor row: " << floorRow << " (y = " << floorRow * level3CellSize << ")" << endl;
            cout << "Player spawn: (" << player_x << ", " << player_y << ")" << endl;
        }
       

        // spawn initial powerups for both levels
        for (int i = 0; i < 3; i++)
        {
            spawnPowerup(powerupsX, powerupsY, powerupType, powerupActive,
                         powerupAnimTimer, powerupCount, maxPowerups,
                         lvl, width, height, cell_size);
        }

        originalSpeed = speed;
        originalVacuumPower = vacuumPower;

        Event ev;

        while (window.isOpen() && !restartGame) // game loop
        {
           
            while (window.pollEvent(ev))
            {
                if (ev.type == Event::Closed)
                    window.close();
            }
            if (Keyboard::isKeyPressed(Keyboard::Escape))
                window.close();

            // update timers
            levelTimer += dt;
            comboTimer += dt;
            multiKillTimer += dt;

            // wave spawing system for level 2
            if (currentLevel == 2 && useWaveSpawning)
            {
                waveTimer += dt;

                // to check if current wave should spawn
                if (currentWave < maxWaves && !waveSpawned[currentWave])
                {
                    // spawn first wave immediately and the others after delay
                    if (currentWave == 0 || waveTimer >= timeBetweenWaves)
                    {
                        spawnWave(currentWave, cell_size,
                                  enemiesX, enemiesY, enemySpeed, enemyDirection,
                                  platformLeftEdge, platformRightEdge, enemyCount,
                                  skeletonsX, skeletonsY, skeletonSpeed, skeletonDirection,
                                  skeletonVelocityY, skeletonOnGround, skeletonJumpTimer,
                                  skeletonJumpCooldown, skeletonShouldJump, skeletonStableFrames,
                                  skeletonAnimFrame, skeletonAnimCounter, skeletonCount,
                                  invisiblesX, invisiblesY, invisibleSpeed, invisibleDirection,
                                  invisibleVelocityY, invisibleOnGround, invisibleIsVisible,
                                  invisibleVisibilityTimer, invisibleTeleportTimer, invisibleCount,
                                  chelnovsX, chelnovsY, chelnovSpeed, chelnovDirection,
                                  chelnovVelocityY, chelnovOnGround, chelnovShootTimer,
                                  chelnovIsShooting, chelnovShootPhaseTimer, chelnovCount,
                                  maxEnemyCount, maxSkeletonCount, maxInvisibleCount, maxChelnovCount);

                        waveSpawned[currentWave] = true;
                        waveTimer = 0.0f;
                    }
                }

                // check if current wave is cleared (all enemies defeated)
                if (waveSpawned[currentWave] &&
                    enemyCount == 0 && skeletonCount == 0 &&
                    invisibleCount == 0 && chelnovCount == 0 &&
                    capturedCount == 0 && projectileCount == 0)
                {
                    currentWave++;
                    waveTimer = 0.0f;

                    if (currentWave < maxWaves)
                    {
                        cout << "Wave " << currentWave << " cleared! Next wave in "
                             << timeBetweenWaves << " seconds..." << endl;
                    }
                }
            }

            
            if (currentLevel == 3)
            {
                
                if (potActive && !potDestroyed)
                {
                    // update movement of cloud up and down. -1 = up and 1 = down
                    cloudY += cloudSpeed * cloudDirection * dt;

                    // reverse direction at top and bottom
                    if (cloudY >= cloudMaxY)
                    {
                        cloudY = cloudMaxY;
                        cloudDirection = -1; 
                    }
                    else if (cloudY <= cloudMinY)
                    {
                        cloudY = cloudMinY;
                        cloudDirection = 1; 
                    }

                    // update pot position to stay on cloud. Pot on center of cloud
                    potX = cloudX + cloudWidth / 2 - potWidth / 2;
                    potY = cloudY - potHeight;

                    // spawn enemies on pot every 4 secs
                    potEnemySpawnTimer += dt;
                    if (potEnemySpawnTimer >= potEnemySpawnInterval)
                    {
                        potEnemySpawnTimer = 0.0f;

                        // define new capacity and type first
                        int newCapacity = potEnemyCapacity + 1;
                        int enemyTypeToSpawn = 1 + (rand() % 4); // 1=ghost, 2=skeleton, 3=invisible, 4=chelnov

                        // allocate all new arrays 
                        float *newX = new float[newCapacity];
                        float *newY = new float[newCapacity];
                        float *newSpeed = new float[newCapacity];
                        int *newDir = new int[newCapacity];
                        float *newVelY = new float[newCapacity];
                        bool *newGround = new bool[newCapacity];
                        float *newVelX = new float[newCapacity];
                        bool *newCaught = new bool[newCapacity];
                        int *newType = new int[newCapacity];

                        // behavior arrays
                        float *newJumpTimer = new float[newCapacity];
                        bool *newShouldJump = new bool[newCapacity];
                        int *newStableFrames = new int[newCapacity];
                        bool *newIsVisible = new bool[newCapacity];
                        float *newVisTimer = new float[newCapacity];
                        float *newTeleTimer = new float[newCapacity];
                        float *newShootTimer = new float[newCapacity];
                        bool *newIsShooting = new bool[newCapacity];

                        // new animation arrays
                        int *newAnimFrame = new int[newCapacity];
                        int *newAnimCounter = new int[newCapacity];

                        // copy existing data
                        for (int i = 0; i < potEnemyCount; i++)
                        {
                            newX[i] = potEnemiesX[i];
                            newY[i] = potEnemiesY[i];
                            newSpeed[i] = potEnemySpeed[i];
                            newDir[i] = potEnemyDirection[i];
                            newVelY[i] = potEnemyVelocityY[i];
                            newGround[i] = potEnemyOnGround[i];
                            newVelX[i] = potEnemyVelocityX[i];
                            newCaught[i] = potEnemyIsCaught[i];
                            newType[i] = potEnemyType[i];

                            // Copy behaviors
                            if (potEnemyJumpTimer != NULL)
                                newJumpTimer[i] = potEnemyJumpTimer[i];
                            if (potEnemyShouldJump != NULL)
                                newShouldJump[i] = potEnemyShouldJump[i];
                            if (potEnemyStableFrames != NULL)
                                newStableFrames[i] = potEnemyStableFrames[i];
                            if (potEnemyIsVisible != NULL)
                                newIsVisible[i] = potEnemyIsVisible[i];
                            if (potEnemyVisibilityTimer != NULL)
                                newVisTimer[i] = potEnemyVisibilityTimer[i];
                            if (potEnemyTeleportTimer != NULL)
                                newTeleTimer[i] = potEnemyTeleportTimer[i];
                            if (potEnemyShootTimer != NULL)
                                newShootTimer[i] = potEnemyShootTimer[i];
                            if (potEnemyIsShooting != NULL)
                                newIsShooting[i] = potEnemyIsShooting[i];

                            // copy animation data
                            if (potEnemyAnimFrame != NULL)
                                newAnimFrame[i] = potEnemyAnimFrame[i];
                            if (potEnemyAnimCounter != NULL)
                                newAnimCounter[i] = potEnemyAnimCounter[i];
                        }

                        // deleting old arrays
                        if (potEnemiesX != NULL)
                            delete[] potEnemiesX;
                        if (potEnemiesY != NULL)
                            delete[] potEnemiesY;
                        if (potEnemySpeed != NULL)
                            delete[] potEnemySpeed;
                        if (potEnemyDirection != NULL)
                            delete[] potEnemyDirection;
                        if (potEnemyVelocityY != NULL)
                            delete[] potEnemyVelocityY;
                        if (potEnemyOnGround != NULL)
                            delete[] potEnemyOnGround;
                        if (potEnemyVelocityX != NULL)
                            delete[] potEnemyVelocityX;
                        if (potEnemyIsCaught != NULL)
                            delete[] potEnemyIsCaught;
                        if (potEnemyType != NULL)
                            delete[] potEnemyType;

                        if (potEnemyJumpTimer != NULL)
                            delete[] potEnemyJumpTimer;
                        if (potEnemyShouldJump != NULL)
                            delete[] potEnemyShouldJump;
                        if (potEnemyStableFrames != NULL)
                            delete[] potEnemyStableFrames;
                        if (potEnemyIsVisible != NULL)
                            delete[] potEnemyIsVisible;
                        if (potEnemyVisibilityTimer != NULL)
                            delete[] potEnemyVisibilityTimer;
                        if (potEnemyTeleportTimer != NULL)
                            delete[] potEnemyTeleportTimer;
                        if (potEnemyShootTimer != NULL)
                            delete[] potEnemyShootTimer;
                        if (potEnemyIsShooting != NULL)
                            delete[] potEnemyIsShooting;

                        
                        if (potEnemyAnimFrame != NULL)
                            delete[] potEnemyAnimFrame;
                        if (potEnemyAnimCounter != NULL)
                            delete[] potEnemyAnimCounter;

                        // assigning new arrays
                        potEnemiesX = newX;
                        potEnemiesY = newY;
                        potEnemySpeed = newSpeed;
                        potEnemyDirection = newDir;
                        potEnemyVelocityY = newVelY;
                        potEnemyOnGround = newGround;
                        potEnemyVelocityX = newVelX;
                        potEnemyIsCaught = newCaught;
                        potEnemyType = newType;

                        potEnemyJumpTimer = newJumpTimer;
                        potEnemyShouldJump = newShouldJump;
                        potEnemyStableFrames = newStableFrames;
                        potEnemyIsVisible = newIsVisible;
                        potEnemyVisibilityTimer = newVisTimer;
                        potEnemyTeleportTimer = newTeleTimer;
                        potEnemyShootTimer = newShootTimer;
                        potEnemyIsShooting = newIsShooting;

                        // assign animation arrays
                        potEnemyAnimFrame = newAnimFrame;
                        potEnemyAnimCounter = newAnimCounter;

                        potEnemyCapacity = newCapacity;

                        // new enemy logic
                        int spawnDirection = rand() % 5;

                        if (spawnDirection == 0) // down
                        {
                            potEnemiesX[potEnemyCount] = potX + potWidth / 2 - 30;
                            potEnemiesY[potEnemyCount] = potY + potHeight;
                            potEnemyVelocityY[potEnemyCount] = 100.0f;
                            potEnemyVelocityX[potEnemyCount] = 0.0f;
                        }
                        else if (spawnDirection == 1) // left
                        {
                            potEnemiesX[potEnemyCount] = potX - 50;
                            potEnemiesY[potEnemyCount] = potY + potHeight / 2;
                            potEnemyVelocityY[potEnemyCount] = -150.0f;
                            potEnemyVelocityX[potEnemyCount] = -200.0f;
                        }
                        else if (spawnDirection == 2) // right
                        {
                            potEnemiesX[potEnemyCount] = potX + potWidth + 50;
                            potEnemiesY[potEnemyCount] = potY + potHeight / 2;
                            potEnemyVelocityY[potEnemyCount] = -150.0f;
                            potEnemyVelocityX[potEnemyCount] = 200.0f;
                        }
                        else if (spawnDirection == 3) // upleft
                        {
                            potEnemiesX[potEnemyCount] = potX + potWidth / 4;
                            potEnemiesY[potEnemyCount] = potY;
                            potEnemyVelocityY[potEnemyCount] = -200.0f;
                            potEnemyVelocityX[potEnemyCount] = -150.0f;
                        }
                        else // upright
                        {
                            potEnemiesX[potEnemyCount] = potX + (potWidth * 3 / 4);
                            potEnemiesY[potEnemyCount] = potY;
                            potEnemyVelocityY[potEnemyCount] = -200.0f;
                            potEnemyVelocityX[potEnemyCount] = 150.0f;
                        }

                        potEnemySpeed[potEnemyCount] = 40.0f + (rand() % 30);
                        potEnemyDirection[potEnemyCount] = (spawnDirection == 1 || spawnDirection == 3) ? -1 : 1;
                        potEnemyOnGround[potEnemyCount] = false;
                        potEnemyIsCaught[potEnemyCount] = false;
                        potEnemyType[potEnemyCount] = enemyTypeToSpawn;

                        // initialize behaviors
                        potEnemyJumpTimer[potEnemyCount] = 0.0f;
                        potEnemyShouldJump[potEnemyCount] = false;
                        potEnemyStableFrames[potEnemyCount] = 0;
                        potEnemyIsVisible[potEnemyCount] = true;
                        potEnemyVisibilityTimer[potEnemyCount] = 0.0f;
                        potEnemyTeleportTimer[potEnemyCount] = 0.0f;
                        potEnemyShootTimer[potEnemyCount] = 0.0f;
                        potEnemyIsShooting[potEnemyCount] = false;

                        // initialize animations
                        potEnemyAnimFrame[potEnemyCount] = 0;
                        potEnemyAnimCounter[potEnemyCount] = 0;

                        potEnemyCount++;
                        cout << "Pot spawned enemy type " << enemyTypeToSpawn << "! Total: " << potEnemyCount << endl;
                    }

                    // pot collision with players
                    if (!isDead && !waitingToRespawn)
                    {
                        if ((player_x < potX + potWidth) &&
                            (player_x + PlayerWidth > potX) &&
                            (player_y < potY + potHeight) &&
                            (player_y + PlayerHeight > potY))
                        {
                            isDead = true;
                            playerLives--;
                            levelNoDamage = false;
                            waitingToRespawn = true;
                            deathDelayCounter = 0.0f;
                            cout << "Player hit by pot! Lives: " << playerLives << endl;
                        }
                    }
                }

                // pot destruction animation
                if (potDestroyed && !bossAppeared) // after pot destroyed the octopus will be spawned 
                {
                    potDestroyTimer += dt;

                    // after 1.5 seconds, boss will appear
                    if (potDestroyTimer >= 1.5f)
                    {
                        bossAppeared = true;
                        cloudIsPlatform = true; // cloud becomes a platform
                        bossX = screen_x / 2 - bossWidth / 2;
                        bossY = 50; // boss appears at top
                        cout << "Boss (octopus) has appeared" << endl;
                    }
                }

                // update pot enemies
                int floorRowForEnemies = level3Height - 2;
                float floorYForEnemies = floorRowForEnemies * level3CellSize;

                for (int pe = 0; pe < potEnemyCount; pe++)
                {
                    if (potEnemyIsCaught[pe])
                    {
                        capturedCount++; 
                        continue;
                    }

                   
                    int enemyH = 60, enemyW = 72; //ghost
                    if (potEnemyType[pe] == 2)
                    {
                        enemyH = 92;
                        enemyW = 72;
                    } // Skeleton
                    else if (potEnemyType[pe] == 3)
                    {
                        enemyH = 80;
                        enemyW = 60;
                    } // Invisible
                    else if (potEnemyType[pe] == 4)
                    {
                        enemyH = 90;
                        enemyW = 60;
                    } // Chelnov

                    

                    // skeleton jumping
                    if (potEnemyType[pe] == 2)
                    {
                        if (potEnemyOnGround[pe])
                            potEnemyStableFrames[pe]++;
                        else
                            potEnemyStableFrames[pe] = 0;

                        potEnemyJumpTimer[pe] += dt;

                        // random chance to jump when stable on ground
                        if (potEnemyOnGround[pe] && potEnemyStableFrames[pe] > 60 && !potEnemyShouldJump[pe])
                        {
                            if (rand() % 100 < 2) // 2% chance each frame
                            {
                                potEnemyShouldJump[pe] = true;
                                potEnemyJumpTimer[pe] = 0.0f;
                            }
                        }

                        // execute jump
                        if (potEnemyShouldJump[pe] && potEnemyOnGround[pe] && potEnemyStableFrames[pe] > 10)
                        {
                            potEnemyVelocityY[pe] = jumpStrength;
                            potEnemyOnGround[pe] = false;
                            potEnemyShouldJump[pe] = false;
                            potEnemyStableFrames[pe] = 0;
                        }
                    }

                    // invisible man behaviour
                    if (potEnemyType[pe] == 3)
                    {
                        // toggle visibility every 3 seconds
                        potEnemyVisibilityTimer[pe] += dt;
                        if (potEnemyVisibilityTimer[pe] >= 3.0f)
                        {
                            potEnemyVisibilityTimer[pe] = 0.0f;
                            potEnemyIsVisible[pe] = !potEnemyIsVisible[pe];
                        }

                        // teleport every 5 seconds
                        potEnemyTeleportTimer[pe] += dt;
                        if (potEnemyTeleportTimer[pe] >= 5.0f)
                        {
                            potEnemyTeleportTimer[pe] = 0.0f;
                            // teleport to random position on floor
                            potEnemiesX[pe] = level3CellSize + (rand() % (screen_x - 2 * level3CellSize - enemyW));
                            potEnemiesY[pe] = floorYForEnemies - enemyH - 50;
                            potEnemyVelocityY[pe] = 0;
                        }
                    }

                    // chelnov shooting
                    if (potEnemyType[pe] == 4)
                    {
                        potEnemyShootTimer[pe] += dt;

                        // shoot every 4 seconds
                        if (potEnemyShootTimer[pe] >= 4.0f)
                        {
                            potEnemyShootTimer[pe] = 0.0f;
                            potEnemyIsShooting[pe] = true;

                            // spawn projectile
                            if (potEnemyProjCount < maxPotEnemyProjectiles)
                            {
                                potEnemyProjX[potEnemyProjCount] = potEnemiesX[pe];
                                potEnemyProjY[potEnemyProjCount] = potEnemiesY[pe] + enemyH / 2;
                                potEnemyProjDirection[potEnemyProjCount] = potEnemyDirection[pe];
                                potEnemyProjActive[potEnemyProjCount] = true;

                                potEnemyProjAnimFrame[potEnemyProjCount] = 0;
                                potEnemyProjAnimCounter[potEnemyProjCount] = 0;

                                potEnemyProjCount++;
                            }
                        }

                        // reset shooting state after 0.5 seconds
                        if (potEnemyIsShooting[pe])
                        {
                            if (potEnemyShootTimer[pe] >= 0.5f)
                                potEnemyIsShooting[pe] = false;
                        }
                    }

                   

                    // apply horizontal velocity (initial launch momentum only)
                    float absVelX = (potEnemyVelocityX[pe] < 0) ? -potEnemyVelocityX[pe] : potEnemyVelocityX[pe];
                    if (absVelX > 1.0f) // only if significant
                    {
                        potEnemiesX[pe] += potEnemyVelocityX[pe] * dt;
                        // friction to slow down horizontal movement over time
                        potEnemyVelocityX[pe] *= 0.95f;
                    }
                    else
                    {
                        potEnemyVelocityX[pe] = 0; // stop when too slow
                    }

                    // apply gravity
                    potEnemyVelocityY[pe] += gravity * dt;
                    if (potEnemyVelocityY[pe] > terminal_Velocity)
                        potEnemyVelocityY[pe] = terminal_Velocity;

                    float newY = potEnemiesY[pe] + potEnemyVelocityY[pe] * dt;

                    
                    // platform collision detection for pot enemies
                    bool potEnemyLanded = false;

                    // check floor collision first
                    if (newY + enemyH >= floorYForEnemies)
                    {
                        newY = floorYForEnemies - enemyH;
                        potEnemyVelocityY[pe] = 0;
                        potEnemyOnGround[pe] = true;
                        potEnemyLanded = true;
                    }

                    // check all platforms in boss level if not on floor
                    if (!potEnemyLanded && potEnemyVelocityY[pe] >= 0)
                    {
                        int feetRow = (int)(newY + enemyH) / level3CellSize;
                        int leftCol = (int)(potEnemiesX[pe]) / level3CellSize;
                        int rightCol = (int)(potEnemiesX[pe] + enemyW) / level3CellSize;

                        // check tiles under the enemy
                        for (int col = leftCol; col <= rightCol && col < level3Width; col++)
                        {
                            if (feetRow >= 0 && feetRow < level3Height)
                            {
                                char tileBelow = bossLvl[feetRow][col];

                                // check for platform '-' or solid block '#'
                                if (tileBelow == '-' || tileBelow == '#')
                                {
                                    float platformTop = feetRow * level3CellSize;

                                    // only land if crossing the platform from above
                                    if (potEnemiesY[pe] + enemyH <= platformTop + 5 && newY + enemyH >= platformTop)
                                    {
                                        newY = platformTop - enemyH;
                                        potEnemyVelocityY[pe] = 0;
                                        potEnemyOnGround[pe] = true;
                                        potEnemyLanded = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // cloud platform collision (when cloud is a platform)
                    if (cloudIsPlatform && !potEnemyLanded && potEnemyVelocityY[pe] >= 0)
                    {
                        if (potEnemiesX[pe] + enemyW > cloudX && potEnemiesX[pe] < cloudX + cloudWidth)
                        {
                            if (potEnemiesY[pe] + enemyH <= cloudY + 10 && newY + enemyH >= cloudY)
                            {
                                newY = cloudY - enemyH;
                                potEnemyVelocityY[pe] = 0;
                                potEnemyOnGround[pe] = true;
                                potEnemyLanded = true;
                            }
                        }
                    }

                    // if no platform found, enemy is in air
                    if (!potEnemyLanded)
                    {
                        potEnemyOnGround[pe] = false;
                    }

                    potEnemiesY[pe] = newY;

                   
                    // Horizontal movement when on ground
                    if (potEnemyOnGround[pe])
                    {
                        
                        // 'currentRow' is the tile BENEATH the feet (The Floor)
                        int feetRow = (int)(potEnemiesY[pe] + enemyH + 1) / level3CellSize;
                        // 'bodyRow' is the tile containing the enemy center (The Wall)
                        int bodyRow = (int)(potEnemiesY[pe] + enemyH / 2) / level3CellSize;

                        // "Bottom floor" check
                        bool onBottomFloor = (feetRow >= level3Height - 3);

                        float newPotX = potEnemiesX[pe] + potEnemySpeed[pe] * potEnemyDirection[pe] * dt;
                        bool hitWall = false;
                       
                        // wall check by using bodyRow
                        
                        int checkCol = (potEnemyDirection[pe] == 1)
                                           ? (int)(newPotX + enemyW) / level3CellSize
                                           : (int)newPotX / level3CellSize;

                        if (bodyRow >= 0 && bodyRow < level3Height &&
                            checkCol >= 0 && checkCol < level3Width)
                        {
                            // if the body hits a wall '#'
                            if (bossLvl[bodyRow][checkCol] == '#')
                                hitWall = true;
                        }

                       // ledge check by using feetRow
                        if (!onBottomFloor && potEnemyType[pe] != 1)
                        {
                            // look ahead for the floor
                            int edgeCheckCol = (potEnemyDirection[pe] == 1)
                                                   ? (int)(newPotX + enemyW + 10) / level3CellSize
                                                   : (int)(newPotX - 10) / level3CellSize;

                            if (feetRow < level3Height &&
                                edgeCheckCol >= 0 && edgeCheckCol < level3Width)
                            {
                                char tileBelowNext = bossLvl[feetRow][edgeCheckCol];
                                // current tile is center of enemy at feet level
                                char tileBelowCurr = bossLvl[feetRow][(int)(potEnemiesX[pe] + enemyW * 0.5f) / level3CellSize];

                                bool currentHasFloor = (tileBelowCurr == '-' || tileBelowCurr == '#');
                                bool nextHasNoFloor = !(tileBelowNext == '-' || tileBelowNext == '#');

                                // only turn if we are currently safe but about to walk off
                                if (currentHasFloor && nextHasNoFloor)
                                    hitWall = true;
                            }
                        }

                        // screen edge limits
                        if (hitWall ||
                            newPotX <= level3CellSize ||
                            newPotX + enemyW >= screen_x - level3CellSize)
                        {
                            potEnemyDirection[pe] *= -1;
                        }
                        else
                        {
                            potEnemiesX[pe] = newPotX;
                        }
                    }

                    // pot enemies collision with player
                    // invisible man only damages when visible
                    bool canDamagePlayer = true;
                    if (potEnemyType[pe] == 3 && !potEnemyIsVisible[pe])
                        canDamagePlayer = false;

                    if (!isDead && !waitingToRespawn && !potEnemyIsCaught[pe] && canDamagePlayer)
                    {
                        if ((player_x < potEnemiesX[pe] + enemyW) &&
                            (player_x + PlayerWidth > potEnemiesX[pe]) &&
                            (player_y < potEnemiesY[pe] + enemyH) &&
                            (player_y + PlayerHeight > potEnemiesY[pe]))
                        {
                            isDead = true;
                            playerLives--;
                            levelNoDamage = false;
                            waitingToRespawn = true;
                            deathDelayCounter = 0.0f;
                            cout << "Player hit by pot enemy! Lives: " << playerLives << endl;
                        }
                    }
                }

                // update chelnov fireballs
                for (int pp = 0; pp < potEnemyProjCount; pp++)
                {
                    if (!potEnemyProjActive[pp])
                        continue;

                    // move projectile
                    potEnemyProjX[pp] += 150.0f * potEnemyProjDirection[pp] * dt;

                    // remove if off screen
                    if (potEnemyProjX[pp] < 0 || potEnemyProjX[pp] > screen_x)
                    {
                        potEnemyProjActive[pp] = potEnemyProjActive[potEnemyProjCount - 1];
                        potEnemyProjX[pp] = potEnemyProjX[potEnemyProjCount - 1];
                        potEnemyProjY[pp] = potEnemyProjY[potEnemyProjCount - 1];
                        potEnemyProjDirection[pp] = potEnemyProjDirection[potEnemyProjCount - 1];
                        potEnemyProjCount--;
                        pp--;
                        continue;
                    }

                    // collision with player
                    if (!isDead && !waitingToRespawn)
                    {
                        if ((player_x < potEnemyProjX[pp] + 30) &&
                            (player_x + PlayerWidth > potEnemyProjX[pp]) &&
                            (player_y < potEnemyProjY[pp] + 30) &&
                            (player_y + PlayerHeight > potEnemyProjY[pp]))
                        {
                            isDead = true;
                            playerLives--;
                            levelNoDamage = false;
                            waitingToRespawn = true;
                            deathDelayCounter = 0.0f;
                            playerScore -= 50;

                            // remove projectile
                            potEnemyProjActive[pp] = potEnemyProjActive[potEnemyProjCount - 1];
                            potEnemyProjX[pp] = potEnemyProjX[potEnemyProjCount - 1];
                            potEnemyProjY[pp] = potEnemyProjY[potEnemyProjCount - 1];
                            potEnemyProjDirection[pp] = potEnemyProjDirection[potEnemyProjCount - 1];
                            potEnemyProjCount--;
                            pp--;

                            cout << "Player hit by pot enemy projectile! Lives: " << playerLives << endl;
                        }
                    }
                }

                // cloud platform collision with player after the pot is destroyed
                if (cloudIsPlatform && velocityY >= 0)
                {
                    if (player_x + PlayerWidth > cloudX && player_x < cloudX + cloudWidth)
                    {
                        if (player_y + PlayerHeight >= cloudY && player_y + PlayerHeight <= cloudY + 15)
                        {
                            player_y = cloudY - PlayerHeight;
                            velocityY = 0;
                            onGround = true;
                        }
                    }
                }

                // boss logic
                if (bossAppeared && !bossDefeated)
                {
                    // update spawn timers
                    minionSpawnTimer += dt;
                    tentacleSpawnTimer += dt;

                    // check if boss is angry (health <= 2)
                    if (bossHealth <= 2 && !bossIsAngry)
                    {
                        bossIsAngry = true;
                        minionSpawnInterval = 2.0f; // spawn faster when angry

                        // make all existing minions follow player
                        for (int m = 0; m < minionCount; m++)
                        {
                            minionFollowingPlayer[m] = true;
                        }

                        cout << "BOSS IS ANGRY! Minions will now follow the player!" << endl;
                    }

                    // spawn minions
                    if (minionSpawnTimer >= minionSpawnInterval && minionCount < maxMinions)
                    
                    {
                        minionSpawnTimer = 0.0f;

                        int minionsToSpawn = 1 + rand() % 3;
                        if (bossIsAngry)
                            minionsToSpawn += 1;

                        for (int m = 0; m < minionsToSpawn && minionCount < maxMinions; m++)
                        {
                            
                            // allocate new arrays of size + 1
                            int newSize = minionCount + 1;
                            float *newX = new float[newSize];
                            float *newY = new float[newSize];
                            float *newSpeed = new float[newSize];
                            int *newDir = new int[newSize];
                            float *newVelY = new float[newSize];
                            bool *newGround = new bool[newSize];
                            bool *newCaught = new bool[newSize];
                            bool *newFollow = new bool[newSize];

                            // copy existing data
                            for (int i = 0; i < minionCount; i++)
                            {
                                newX[i] = minionsX[i];
                                newY[i] = minionsY[i];
                                newSpeed[i] = minionSpeed[i];
                                newDir[i] = minionDirection[i];
                                newVelY[i] = minionVelocityY[i];
                                newGround[i] = minionOnGround[i];
                                newCaught[i] = minionIsCaught[i];
                                newFollow[i] = minionFollowingPlayer[i];
                            }

                            // delete old arrays to free memory
                            if (minionsX != NULL)
                                delete[] minionsX;
                            if (minionsY != NULL)
                                delete[] minionsY;
                            if (minionSpeed != NULL)
                                delete[] minionSpeed;
                            if (minionDirection != NULL)
                                delete[] minionDirection;
                            if (minionVelocityY != NULL)
                                delete[] minionVelocityY;
                            if (minionOnGround != NULL)
                                delete[] minionOnGround;
                            if (minionIsCaught != NULL)
                                delete[] minionIsCaught;
                            if (minionFollowingPlayer != NULL)
                                delete[] minionFollowingPlayer;

                            // point to new arrays
                            minionsX = newX;
                            minionsY = newY;
                            minionSpeed = newSpeed;
                            minionDirection = newDir;
                            minionVelocityY = newVelY;
                            minionOnGround = newGround;
                            minionIsCaught = newCaught;
                            minionFollowingPlayer = newFollow;

                            // add new minion at the end
                            minionsX[minionCount] = bossX + (rand() % bossWidth);
                            minionsY[minionCount] = bossY + bossHeight;
                            minionSpeed[minionCount] = 30.0f + (rand() % 20);
                            minionDirection[minionCount] = (rand() % 2 == 0) ? -1 : 1;
                            minionVelocityY[minionCount] = 0;
                            minionOnGround[minionCount] = false;
                            minionIsCaught[minionCount] = false;
                            minionFollowingPlayer[minionCount] = bossIsAngry;

                            minionCount++;
                            cout << "Minion spawned (Dynamic)! Total: " << minionCount << endl;
                        }
                    }

                    // spawn tentacles
                    if (tentacleSpawnTimer >= tentacleSpawnInterval)
                    {
                        tentacleSpawnTimer = 0.0f;

                        // 60% chance to spawn a tentacle
                        if (rand() % 100 < 60 && tentacleCount < maxTentacles)
                        {
                            // dynamic resizing
                            int newSize = tentacleCount + 1;
                            float *newX = new float[newSize];
                            float *newY = new float[newSize];
                            int *newW = new int[newSize];
                            int *newH = new int[newSize];
                            float *newTimer = new float[newSize];
                            float *newDur = new float[newSize];
                            bool *newActive = new bool[newSize];
                            int *newTexIndex = new int[newSize]; 

                            // copy existing data
                            for (int i = 0; i < tentacleCount; i++)
                            {
                                newX[i] = tentaclesX[i];
                                newY[i] = tentaclesY[i];
                                newW[i] = tentacleWidth[i];
                                newH[i] = tentacleHeight[i];
                                newTimer[i] = tentacleTimer[i];
                                newDur[i] = tentacleDuration[i];
                                newActive[i] = tentacleActive[i];
                                if (tentacleTexIndex != NULL)
                                    newTexIndex[i] = tentacleTexIndex[i];
                            }

                            // delete old arrays
                            if (tentaclesX != NULL)
                                delete[] tentaclesX;
                            if (tentaclesY != NULL)
                                delete[] tentaclesY;
                            if (tentacleWidth != NULL)
                                delete[] tentacleWidth;
                            if (tentacleHeight != NULL)
                                delete[] tentacleHeight;
                            if (tentacleTimer != NULL)
                                delete[] tentacleTimer;
                            if (tentacleDuration != NULL)
                                delete[] tentacleDuration;
                            if (tentacleActive != NULL)
                                delete[] tentacleActive;
                            if (tentacleTexIndex != NULL)
                                delete[] tentacleTexIndex;

                            // sssign new arrays
                            tentaclesX = newX;
                            tentaclesY = newY;
                            tentacleWidth = newW;
                            tentacleHeight = newH;
                            tentacleTimer = newTimer;
                            tentacleDuration = newDur;
                            tentacleActive = newActive;
                            tentacleTexIndex = newTexIndex;

                            // calculating edge spawn and loading textures
                            int side = rand() % 4; // 0=Top, 1=Bottom, 2=Left, 3=Right
                            int fixedThickness = 1; 
                            int fixedLength = 180;

                            int tW, tH;
                            int texID = 0;
                            float spawnX, spawnY;

                            if (side == 0) // top edge , vertical
                            {
                                tW = fixedThickness;
                                tH = fixedLength;
                                texID = 0 + (rand() % 3); 
                                
                                spawnX = bossX + (rand() % (bossWidth - tW));
                                spawnY = bossY - tH + 20; 
                            }
                            else if (side == 1) // bottom edge vertical
                            {
                                tW = fixedThickness;
                                tH = fixedLength;
                                texID = 3 + (rand() % 3); 
                                
                                spawnX = bossX + (rand() % (bossWidth - tW));
                                spawnY = bossY + bossHeight - 20; 
                            }
                            else if (side == 2) // left edge horozontal
                            {
                                // swap dimensions for side tentacles
                                tW = fixedLength;
                                tH = fixedThickness;
                                texID = 6 + (rand() % 3); 
                                
                                spawnX = bossX - tW + 20;
                                spawnY = bossY + (rand() % (bossHeight - tH));
                            }
                            else // right edge horizontal
                            {
                                // swap dimensions for side tentacles
                                tW = fixedLength;
                                tH = fixedThickness;
                                texID = 9 + (rand() % 3); 
                                
                                spawnX = bossX + bossWidth - 20;
                                spawnY = bossY + (rand() % (bossHeight - tH));
                            }

                            // assign values
                            tentaclesX[tentacleCount] = spawnX;
                            tentaclesY[tentacleCount] = spawnY;
                            tentacleWidth[tentacleCount] = tW;
                            tentacleHeight[tentacleCount] = tH;
                            tentacleTimer[tentacleCount] = 0.0f;
                            tentacleDuration[tentacleCount] = 2.0f + (rand() % 40) / 10.0f;
                            tentacleActive[tentacleCount] = true;
                            tentacleTexIndex[tentacleCount] = texID; // Store ID

                            tentacleCount++;
                            cout << "Tentacle spawned on side " << side << " (Tex " << texID << ")" << endl;
                        }
                    }

                    // update tentacles
                    for (int t = 0; t < tentacleCount; t++)
                    {
                        if (tentacleActive[t])
                        {
                            tentacleTimer[t] += dt;

                            // remove tentacle when duration expires
                            if (tentacleTimer[t] >= tentacleDuration[t])
                            {
                                // shift remaining tentacles
                                for (int j = t; j < tentacleCount - 1; j++)
                                {
                                    tentaclesX[j] = tentaclesX[j + 1];
                                    tentaclesY[j] = tentaclesY[j + 1];
                                    tentacleWidth[j] = tentacleWidth[j + 1];
                                    tentacleHeight[j] = tentacleHeight[j + 1];
                                    tentacleTimer[j] = tentacleTimer[j + 1];
                                    tentacleDuration[j] = tentacleDuration[j + 1];
                                    tentacleActive[j] = tentacleActive[j + 1];

                                    
                                    tentacleTexIndex[j] = tentacleTexIndex[j + 1];
                                }
                                tentacleCount--;
                                t--;
                            }
                        }
                    }

                    // update minions
                    int minionFloorRow = level3Height - 2;
                    float minionFloorY = minionFloorRow * level3CellSize;

                    for (int m = 0; m < minionCount; m++)
                    {
                        if (minionIsCaught[m])
                            continue;

                        // apply gravity
                        minionVelocityY[m] += gravity * dt;
                        if (minionVelocityY[m] > terminal_Velocity)
                            minionVelocityY[m] = terminal_Velocity;

                        float newMinionY = minionsY[m] + minionVelocityY[m] * dt;

                        // floor collision
                        if (newMinionY + minionHeight >= minionFloorY)
                        {
                            newMinionY = minionFloorY - minionHeight;
                            minionVelocityY[m] = 0;
                            minionOnGround[m] = true;
                        }
                        else
                        {
                            minionOnGround[m] = false;
                        }

                        minionsY[m] = newMinionY;

                        // horizontal movement
                        if (minionFollowingPlayer[m] && minionOnGround[m])
                        {
                            // follow player when angry
                            if (player_x > minionsX[m] + minionWidth / 2)
                            {
                                minionsX[m] += minionSpeed[m] * dt;
                                minionDirection[m] = 1;
                            }
                            else if (player_x < minionsX[m] - minionWidth / 2)
                            {
                                minionsX[m] -= minionSpeed[m] * dt;
                                minionDirection[m] = -1;
                            }
                        }
                        else if (minionOnGround[m])
                        {
                            // normal patrol movement
                            minionsX[m] += minionSpeed[m] * minionDirection[m] * dt;

                            // bounce off walls
                            if (minionsX[m] <= level3CellSize)
                            {
                                minionsX[m] = level3CellSize;
                                minionDirection[m] = 1;
                            }
                            else if (minionsX[m] + minionWidth >= screen_x - level3CellSize)
                            {
                                minionsX[m] = screen_x - level3CellSize - minionWidth;
                                minionDirection[m] = -1;
                            }
                        }

                        // minnion collision with player
                        if (!isDead && !waitingToRespawn)
                        {
                            if ((player_x < minionsX[m] + minionWidth) &&
                                (player_x + PlayerWidth > minionsX[m]) &&
                                (player_y < minionsY[m] + minionHeight) &&
                                (player_y + PlayerHeight > minionsY[m]))
                            {
                                isDead = true;
                                playerLives--;
                                levelNoDamage = false;
                                waitingToRespawn = true;
                                deathDelayCounter = 0.0f;
                                cout << "Player hit by minion! Lives: " << playerLives << endl;
                            }
                        }
                    }

                    // tentacle collison with player
                    if (!isDead && !waitingToRespawn)
                    {
                        for (int t = 0; t < tentacleCount; t++)
                        {
                            if (!tentacleActive[t])
                                continue;

                            if ((player_x < tentaclesX[t] + tentacleWidth[t]) &&
                                (player_x + PlayerWidth > tentaclesX[t]) &&
                                (player_y < tentaclesY[t] + tentacleHeight[t]) &&
                                (player_y + PlayerHeight > tentaclesY[t]))
                            {
                                isDead = true;
                                playerLives--;
                                levelNoDamage = false;
                                waitingToRespawn = true;
                                deathDelayCounter = 0.0f;
                                cout << "Player hit by tentacle! Lives: " << playerLives << endl;
                            }
                        }
                    }

                    // boss collision with player
                    if (!isDead && !waitingToRespawn)
                    {
                        if ((player_x < bossX + bossWidth) &&
                            (player_x + PlayerWidth > bossX) &&
                            (player_y < bossY + bossHeight) &&
                            (player_y + PlayerHeight > bossY))
                        {
                            isDead = true;
                            playerLives--;
                            levelNoDamage = false;
                            waitingToRespawn = true;
                            deathDelayCounter = 0.0f;
                            cout << "Player hit by boss! Lives: " << playerLives << endl;
                        }
                    }
                }
            }
            

            if (comboTimer >= comboTimeout)
                comboStreak = 0;

            if (multiKillTimer >= multiKillWindow && multiKillCount > 0)
                checkMultiKill(multiKillCount, multiKillTimer, playerScore);

            // update powerup timers
            if (hasSpeedBoost)
            {
                speedBoostTimer += dt;
                if (speedBoostTimer >= powerupDuration)
                {
                    hasSpeedBoost = false;
                    speed = originalSpeed * speedMultiplier;
                }
            }
            if (hasRangeBoost)
            {
                rangeBoostTimer += dt;
                if (rangeBoostTimer >= powerupDuration)
                    hasRangeBoost = false;
            }
            if (hasPowerBoost)
            {
                powerBoostTimer += dt;
                if (powerBoostTimer >= powerupDuration)
                {
                    hasPowerBoost = false;
                    vacuumPower = originalVacuumPower;
                }
            }

            for (int i = 0; i < powerupCount; i++)
            {
                if (powerupActive[i])
                    powerupAnimTimer[i] += dt;
            }

            // reset safety flags
            for (int i = 0; i < maxEnemyCount; i++)
                enemyIsCaught[i] = false;
            for (int i = 0; i < maxSkeletonCount; i++)
                skeletonIsCaught[i] = false;
            for (int i = 0; i < maxInvisibleCount; i++)
                invisibleIsCaught[i] = false;
            for (int i = 0; i < maxChelnovCount; i++)
                chelnovIsCaught[i] = false;

            // vacuum input
            isVacuuming = false;
            if (Keyboard::isKeyPressed(Keyboard::Space))
            {
                isVacuuming = true;
                if (Keyboard::isKeyPressed(Keyboard::D))
                {
                    vacDirection = 0;
                    facing = 1;
                }
                else if (Keyboard::isKeyPressed(Keyboard::A))
                {
                    vacDirection = 1;
                    facing = 0;
                }
                else if (Keyboard::isKeyPressed(Keyboard::W))
                    vacDirection = 2;
                else if (Keyboard::isKeyPressed(Keyboard::S))
                    vacDirection = 3;
                else
                    vacDirection = facing;
            }
            else
                showVacSprite = true;

            // release direction
            if (Keyboard::isKeyPressed(Keyboard::D))
                releaseDirection = 0;
            else if (Keyboard::isKeyPressed(Keyboard::A))
                releaseDirection = 1;
            else if (Keyboard::isKeyPressed(Keyboard::W))
                releaseDirection = 2;
            else if (Keyboard::isKeyPressed(Keyboard::S))
                releaseDirection = 3;
            else
                releaseDirection = facing;

            // Single Shot - E key
            static bool eKeyPressed = false;
           

            if (Keyboard::isKeyPressed(Keyboard::E) && !eKeyPressed)
            {
                eKeyPressed = true;

                // phase 2 shooting
                if (currentLevel >= 3)
                {
                    if (dynamicCapturedCount > 0)
                    {
                        // get the last enemy captured (LIFO)
                        int enemyTypeToRelease = dynamicCapturedEnemies[dynamicCapturedCount - 1];

                        // standard Projectile Spawn Logic
                        if (projectileCount < MAX_PROJECTILES)
                        {
                            projectilesX[projectileCount] = player_x + PlayerWidth / 2 - ProjectileWidth / 2;
                            projectilesY[projectileCount] = player_y + PlayerHeight / 2 - ProjectileHeight / 2;
                            projectileType[projectileCount] = enemyTypeToRelease;
                            projectileActive[projectileCount] = true;
                            projectileAnimFrame[projectileCount] = 0;
                            projectileAnimCounter[projectileCount] = 0;
                            projectileVelocityY[projectileCount] = 0;
                            projectileOnGround[projectileCount] = false;
                            projectileLifespan[projectileCount] = 0.0f;

                            if (releaseDirection == 0)
                            {
                                projectileDirection[projectileCount] = 1;
                            }
                            else if (releaseDirection == 1)
                            {
                                projectileDirection[projectileCount] = -1;
                            }
                            else if (releaseDirection == 2)
                            {
                                projectileDirection[projectileCount] = (facing == 1) ? 1 : -1;
                                projectileVelocityY[projectileCount] = -200.0f;
                            }
                            else if (releaseDirection == 3)
                            {
                                projectileDirection[projectileCount] = (facing == 1) ? 1 : -1;
                                projectileVelocityY[projectileCount] = 200.0f;
                            }

                            projectileCount++;
                        }

                        // dynamic shrinking 
                        dynamicCapturedCount--;

                        if (dynamicCapturedCount == 0)
                        {
                            delete[] dynamicCapturedEnemies;
                            dynamicCapturedEnemies = nullptr; // completely free memory
                        }
                        else
                        {
                            int *newArr = new int[dynamicCapturedCount];
                            for (int k = 0; k < dynamicCapturedCount; k++)
                            {
                                newArr[k] = dynamicCapturedEnemies[k];
                            }
                            delete[] dynamicCapturedEnemies;
                            dynamicCapturedEnemies = newArr;
                        }
                    }
                }
                // phase 1 shooting
                else
                {
                    if (capturedCount > 0 && projectileCount < MAX_PROJECTILES)
                    {
                        capturedCount--;
                        int enemyTypeToRelease = capturedEnemies[capturedCount];
                       
                        projectilesX[projectileCount] = player_x + PlayerWidth / 2 - ProjectileWidth / 2;
                        projectilesY[projectileCount] = player_y + PlayerHeight / 2 - ProjectileHeight / 2;
                        projectileType[projectileCount] = enemyTypeToRelease;
                        projectileActive[projectileCount] = true;
                        projectileAnimFrame[projectileCount] = 0;
                        projectileAnimCounter[projectileCount] = 0;
                        projectileVelocityY[projectileCount] = 0;
                        projectileOnGround[projectileCount] = false;
                        projectileLifespan[projectileCount] = 0.0f;

                        if (releaseDirection == 0)
                        {
                            projectileDirection[projectileCount] = 1;
                        }
                        else if (releaseDirection == 1)
                        {
                            projectileDirection[projectileCount] = -1;
                        }
                        else if (releaseDirection == 2)
                        {
                            projectileDirection[projectileCount] = (facing == 1) ? 1 : -1;
                            projectileVelocityY[projectileCount] = -200.0f;
                        }
                        else if (releaseDirection == 3)
                        {
                            projectileDirection[projectileCount] = (facing == 1) ? 1 : -1;
                            projectileVelocityY[projectileCount] = 200.0f;
                        }
                        projectileCount++;
                    }
                }
            }
            if (!Keyboard::isKeyPressed(Keyboard::E))
                eKeyPressed = false;

            // vacuum Burst - R key
            static bool rKeyPressed = false;
            if (Keyboard::isKeyPressed(Keyboard::R) && !rKeyPressed)
            {
                rKeyPressed = true;
                
                
                int currentStored = (currentLevel == 3) ? dynamicCapturedCount : capturedCount;

                if (currentStored > 0 && !burstModeActive)
                {
                    if (currentStored >= 3)
                        playerScore += 300;
                    burstModeActive = true;
                    burstFrameCounter = 0;
                    burstReleaseDirection = releaseDirection;
                    burstPlayerFacing = facing;
                }
            }
            if (!Keyboard::isKeyPressed(Keyboard::R))
                rKeyPressed = false;

            // burst mode release
            if (burstModeActive)
            {
                burstFrameCounter++;
                if (burstFrameCounter >= BURST_FRAME_DELAY)
                {
                    burstFrameCounter = 0;

                    // lvl 3 burst logic
                    if (currentLevel == 3)
                    {
                        if (dynamicCapturedCount > 0 && projectileCount < MAX_PROJECTILES)
                        {
                            // get last captured enemy from dynamic array
                            int enemyTypeToRelease = dynamicCapturedEnemies[dynamicCapturedCount - 1];

                            // spawn Projectile
                            projectilesX[projectileCount] = player_x + PlayerWidth / 2 - ProjectileWidth / 2;
                            projectilesY[projectileCount] = player_y + PlayerHeight / 2 - ProjectileHeight / 2;
                            projectileType[projectileCount] = enemyTypeToRelease;
                            projectileActive[projectileCount] = true;
                            projectileAnimFrame[projectileCount] = 0;
                            projectileAnimCounter[projectileCount] = 0;
                            projectileVelocityY[projectileCount] = 0;
                            projectileOnGround[projectileCount] = false;
                            projectileLifespan[projectileCount] = 0.0f;

                            // burst direction logic
                            if (burstReleaseDirection == 0)
                            {
                                projectileDirection[projectileCount] = 1;
                                projectileVelocityY[projectileCount] = 0;
                            }
                            else if (burstReleaseDirection == 1)
                            {
                                projectileDirection[projectileCount] = -1;
                                projectileVelocityY[projectileCount] = 0;
                            }
                            else if (burstReleaseDirection == 2)
                            {
                                projectileDirection[projectileCount] = (burstPlayerFacing == 1) ? 1 : -1;
                                projectileVelocityY[projectileCount] = -200.0f;
                            }
                            else if (burstReleaseDirection == 3)
                            {
                                projectileDirection[projectileCount] = (burstPlayerFacing == 1) ? 1 : -1;
                                projectileVelocityY[projectileCount] = 200.0f;
                            }

                            projectileCount++;

                            // remove from dynamic array (Shrink Memory)
                            dynamicCapturedCount--;

                            if (dynamicCapturedCount == 0)
                            {
                                delete[] dynamicCapturedEnemies;
                                dynamicCapturedEnemies = nullptr;
                            }
                            else
                            {
                                int *newArr = new int[dynamicCapturedCount];
                                for (int k = 0; k < dynamicCapturedCount; k++)
                                {
                                    newArr[k] = dynamicCapturedEnemies[k];
                                }
                                delete[] dynamicCapturedEnemies;
                                dynamicCapturedEnemies = newArr;
                            }
                        }

                        // stop burst if empty
                        if (dynamicCapturedCount <= 0)
                            burstModeActive = false;
                    }
                    // lvl 1 and 2 burtst logic
                    else
                    {
                        if (capturedCount > 0 && projectileCount < MAX_PROJECTILES)
                        {
                            capturedCount--;
                            int enemyTypeToRelease = capturedEnemies[capturedCount];

                            projectilesX[projectileCount] = player_x + PlayerWidth / 2 - ProjectileWidth / 2;
                            projectilesY[projectileCount] = player_y + PlayerHeight / 2 - ProjectileHeight / 2;
                            projectileType[projectileCount] = enemyTypeToRelease;
                            projectileActive[projectileCount] = true;
                            projectileAnimFrame[projectileCount] = 0;
                            projectileAnimCounter[projectileCount] = 0;
                            projectileVelocityY[projectileCount] = 0;
                            projectileOnGround[projectileCount] = false;
                            projectileLifespan[projectileCount] = 0.0f;

                            if (burstReleaseDirection == 0)
                            {
                                projectileDirection[projectileCount] = 1;
                                projectileVelocityY[projectileCount] = 0;
                            }
                            else if (burstReleaseDirection == 1)
                            {
                                projectileDirection[projectileCount] = -1;
                                projectileVelocityY[projectileCount] = 0;
                            }
                            else if (burstReleaseDirection == 2)
                            {
                                projectileDirection[projectileCount] = (burstPlayerFacing == 1) ? 1 : -1;
                                projectileVelocityY[projectileCount] = -200.0f;
                            }
                            else if (burstReleaseDirection == 3)
                            {
                                projectileDirection[projectileCount] = (burstPlayerFacing == 1) ? 1 : -1;
                                projectileVelocityY[projectileCount] = 200.0f;
                            }

                            projectileCount++;
                        }
                        if (capturedCount <= 0)
                            burstModeActive = false;
                    }
                }
            }

            // vacuum logic
            handleVacuum(window, vacSprite, vacTexHorz, vacTexVert,
                         player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming,
                         enemiesX, enemiesY, enemyCount, capturedEnemies, capturedCount, MAX_CAPACITY, 1,
                         vacFlickerTimer, showVacSprite, dt, enemyIsCaught, false, vacuumPower,
                         playerScore, comboStreak, comboTimer, multiKillCount, multiKillTimer, hasRangeBoost);

            handleVacuum(window, vacSprite, vacTexHorz, vacTexVert,
                         player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming,
                         skeletonsX, skeletonsY, skeletonCount, capturedEnemies, capturedCount, MAX_CAPACITY, 2,
                         vacFlickerTimer, showVacSprite, dt, skeletonIsCaught, false, vacuumPower,
                         playerScore, comboStreak, comboTimer, multiKillCount, multiKillTimer, hasRangeBoost);

            // level 2 vacuum handling
            if (currentLevel == 2)
            {
                handleVacuum(window, vacSprite, vacTexHorz, vacTexVert,
                             player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming,
                             invisiblesX, invisiblesY, invisibleCount, capturedEnemies, capturedCount, MAX_CAPACITY, 3,
                             vacFlickerTimer, showVacSprite, dt, invisibleIsCaught, false, vacuumPower,
                             playerScore, comboStreak, comboTimer, multiKillCount, multiKillTimer, hasRangeBoost);

                handleVacuum(window, vacSprite, vacTexHorz, vacTexVert,
                             player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming,
                             chelnovsX, chelnovsY, chelnovCount, capturedEnemies, capturedCount, MAX_CAPACITY, 4,
                             vacFlickerTimer, showVacSprite, dt, chelnovIsCaught, false, vacuumPower,
                             playerScore, comboStreak, comboTimer, multiKillCount, multiKillTimer, hasRangeBoost);
            }

            // boss Level (Level 3) vacuum handling for minions
            if (currentLevel == 3 && minionCount > 0)
            {
                // use Phase 2 function (Dynamic Pointer)
                handleVacuumPhase2(window, vacSprite, vacTexHorz, vacTexVert,
                                   player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming,
                                   minionsX, minionsY, minionCount,
                                   dynamicCapturedEnemies, dynamicCapturedCount, 
                                   5,                                           
                                   vacFlickerTimer, showVacSprite, dt, minionIsCaught, false, vacuumPower,
                                   playerScore, comboStreak, comboTimer, multiKillCount, multiKillTimer, hasRangeBoost);
            }

           
            if (currentLevel == 3 && potEnemyCount > 0 && isVacuuming)
            {

                float pCenterX = player_x + PlayerWidth / 2;
                float pCenterY = player_y + PlayerHeight / 2;
                float beamReach = hasRangeBoost ? 180.0f : 120.0f;
                float beamThick = 60.0f;

                // calculate vacuum hitbox based on direction
                float vacLeft, vacTop, vacRight, vacBottom;
                if (vacDirection == 0) // right
                {
                    vacLeft = player_x + PlayerWidth;
                    vacTop = pCenterY - beamThick;
                    vacRight = vacLeft + beamReach * 2;
                    vacBottom = pCenterY + beamThick;
                }
                else if (vacDirection == 1) // left
                {
                    vacRight = player_x;
                    vacTop = pCenterY - beamThick;
                    vacLeft = vacRight - beamReach * 2;
                    vacBottom = pCenterY + beamThick;
                }
                else if (vacDirection == 2) // up
                {
                    vacLeft = pCenterX - beamThick;
                    vacBottom = player_y;
                    vacRight = pCenterX + beamThick;
                    vacTop = vacBottom - beamReach * 2;
                }
                else // down
                {
                    vacLeft = pCenterX - beamThick;
                    vacTop = player_y + PlayerHeight;
                    vacRight = pCenterX + beamThick;
                    vacBottom = vacTop + beamReach * 2;
                }

                for (int pe = 0; pe < potEnemyCount; pe++)
                {
                    // to run logic continuously
                    potEnemyIsCaught[pe] = false;

                  
                    int enemyW = 72, enemyH = 60; // ghost
                    if (potEnemyType[pe] == 2)
                    {
                        enemyW = 72;
                        enemyH = 92;
                    } // Skeleton
                    else if (potEnemyType[pe] == 3)
                    {
                        enemyW = 60;
                        enemyH = 80;
                    } // Invisible
                    else if (potEnemyType[pe] == 4)
                    {
                        enemyW = 60;
                        enemyH = 90;
                    } // Chelnov

                    // check if enemy is in vacuum hitbox
                    if (potEnemiesX[pe] + enemyW > vacLeft && potEnemiesX[pe] < vacRight &&
                        potEnemiesY[pe] + enemyH > vacTop && potEnemiesY[pe] < vacBottom)
                    {
                        potEnemyIsCaught[pe] = true;

                        // pull towards player
                        float pullSpeed = 250.0f * vacuumPower * dt;
                        if (potEnemiesX[pe] < pCenterX)
                            potEnemiesX[pe] += pullSpeed;
                        else
                            potEnemiesX[pe] -= pullSpeed;
                        if (potEnemiesY[pe] < pCenterY)
                            potEnemiesY[pe] += pullSpeed;
                        else
                            potEnemiesY[pe] -= pullSpeed;

                        // check if close enough to capture
                        float dx = pCenterX - (potEnemiesX[pe] + enemyW / 2);
                        float dy = pCenterY - (potEnemiesY[pe] + enemyH / 2);
                        float dist = sqrt(dx * dx + dy * dy);


                        if (dist < 50.0f * vacuumPower) 
                        {
                            
                            // resize the dynamic array to fit exactly one more enemy
                            int *newArr = new int[dynamicCapturedCount + 1];

                            // copy existing enemies
                            for (int k = 0; k < dynamicCapturedCount; k++)
                            {
                                newArr[k] = dynamicCapturedEnemies[k];
                            }

                            // add the new enemy , preserving its specific type
                            newArr[dynamicCapturedCount] = potEnemyType[pe];

                            // delete old array and update pointer
                            if (dynamicCapturedEnemies != NULL)
                                delete[] dynamicCapturedEnemies;
                            dynamicCapturedEnemies = newArr;
                            dynamicCapturedCount++; // update the Phase 2 counter

                            // scores
                            int capturePoints = 50;
                            if (potEnemyType[pe] == 2)
                                capturePoints = 75;
                            else if (potEnemyType[pe] == 3)
                                capturePoints = 150;
                            else if (potEnemyType[pe] == 4)
                                capturePoints = 200;

                            addScore(playerScore, comboStreak, comboTimer, capturePoints, false, multiKillCount, multiKillTimer, dt);

                            
                            for (int j = pe; j < potEnemyCount - 1; j++)
                            {
                                potEnemiesX[j] = potEnemiesX[j + 1];
                                potEnemiesY[j] = potEnemiesY[j + 1];
                                potEnemySpeed[j] = potEnemySpeed[j + 1];
                                potEnemyDirection[j] = potEnemyDirection[j + 1];
                                potEnemyVelocityY[j] = potEnemyVelocityY[j + 1];
                                potEnemyOnGround[j] = potEnemyOnGround[j + 1];
                                potEnemyIsCaught[j] = potEnemyIsCaught[j + 1];
                                potEnemyType[j] = potEnemyType[j + 1];

                                // behavior arrays 
                                potEnemyJumpTimer[j] = potEnemyJumpTimer[j + 1];
                                potEnemyShouldJump[j] = potEnemyShouldJump[j + 1];
                                potEnemyStableFrames[j] = potEnemyStableFrames[j + 1];
                                potEnemyIsVisible[j] = potEnemyIsVisible[j + 1];
                                potEnemyVisibilityTimer[j] = potEnemyVisibilityTimer[j + 1];
                                potEnemyTeleportTimer[j] = potEnemyTeleportTimer[j + 1];
                                potEnemyShootTimer[j] = potEnemyShootTimer[j + 1];
                                potEnemyIsShooting[j] = potEnemyIsShooting[j + 1];
                            }
                            potEnemyCount--;
                            pe--; // adjust index since we removed an enemy

                            cout << "Captured pot enemy! Total Dynamic: " << dynamicCapturedCount << endl;
                        }
                    }
                }
            }
            

            // powerup collection
            for (int i = 0; i < powerupCount; i++)
            {
                if (!powerupActive[i])
                    continue;

                if ((player_x < powerupsX[i] + PowerupWidth) &&
                    (player_x + PlayerWidth > powerupsX[i]) &&
                    (player_y < powerupsY[i] + PowerupHeight) &&
                    (player_y + PlayerHeight > powerupsY[i]))
                {
                    powerupActive[i] = false;

                    switch (powerupType[i])
                    {
                    case 1:
                        hasSpeedBoost = true;
                        speedBoostTimer = 0.0f;
                        speed = originalSpeed * speedMultiplier * 2.0f;
                        playerScore += 100;
                        break;
                    case 2:
                        hasRangeBoost = true;
                        rangeBoostTimer = 0.0f;
                        playerScore += 100;
                        break;
                    case 3:
                        hasPowerBoost = true;
                        powerBoostTimer = 0.0f;
                        vacuumPower = originalVacuumPower * 1.5f;
                        playerScore += 100;
                        break;
                    case 4:
                        playerLives++;
                        playerScore += 200;
                        break;
                    }

                    powerupsX[i] = powerupsX[powerupCount - 1];
                    powerupsY[i] = powerupsY[powerupCount - 1];
                    powerupType[i] = powerupType[powerupCount - 1];
                    powerupActive[i] = powerupActive[powerupCount - 1];
                    powerupCount--;
                    i--;
                }
            }
            isMoving = 0;

            // player collision and physics ,using appropriate level map
            if (currentLevel == 3)
            {
                // boss Level uses bossLvl map with different dimensions
                playerMovement(bossLvl, player_x, player_y, speed, level3CellSize, PlayerHeight,
                               PlayerWidth, level3Height, level3Width, dt, isMoving, facing);
            }
            else
            {
                playerMovement(lvl, player_x, player_y, speed, cell_size, PlayerHeight,
                               PlayerWidth, height, width, dt, isMoving, facing);
            }

            updatePlayerAnimation(PlayerSprite, facing, isMoving, isDead, onGround, idleTex,
                                  walkTex, jumpTex, deadTex, animFrame, deadAnimFrame, animCounter, deadAnimCounter, animSpeed, deadAnimSpeed);

            if (Keyboard::isKeyPressed(Keyboard::Up) && onGround)
            {
                velocityY = jumpStrength;
                onGround = false;
                isJumping = true;
            }

            // player gravity , use appropriate level map
            if (currentLevel == 3)
            {
                player_gravity(bossLvl, offset_y, velocityY, onGround, gravity, terminal_Velocity, player_x, player_y, level3CellSize, PlayerHeight, PlayerWidth, level3Height, level3Width, dt);
            }
            else
            {
                player_gravity(lvl, offset_y, velocityY, onGround, gravity, terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth, height, width, dt);
            }

            
            // apply sliding on slopes (Level 2)
            if (currentLevel == 2)
            {
                applySliding(lvl, player_x, player_y, PlayerHeight, PlayerWidth, cell_size, height, width, dt, onGround, velocityY);
            }

            // ghost enemy loop
            for (int i = 0; i < enemyCount; i++)
            {
                enemiesX[i] += enemySpeed[i] * enemyDirection[i] * dt;

                if (enemiesX[i] <= platformLeftEdge[i])
                {
                    enemiesX[i] = platformLeftEdge[i];
                    enemyDirection[i] = 1;
                }
                else if (enemiesX[i] >= platformRightEdge[i])
                {
                    enemiesX[i] = platformRightEdge[i];
                    enemyDirection[i] = -1;
                }

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
                            playerScore -= 50;
                        }
                    }
                }
            }

            // skeleton enemy loop
            for (int i = 0; i < skeletonCount; i++)
            {
                float newX = skeletonsX[i] + skeletonSpeed[i] * skeletonDirection[i] * dt;

                char right_check = get_tile(lvl, (int)(skeletonsY[i] + SkeletonHeight / 2) / cell_size,
                                            (int)(newX + SkeletonWidth) / cell_size, height, width);
                char left_check = get_tile(lvl, (int)(skeletonsY[i] + SkeletonHeight / 2) / cell_size,
                                           (int)newX / cell_size, height, width);

                if ((skeletonDirection[i] == 1 && right_check == '#') ||
                    (skeletonDirection[i] == -1 && left_check == '#'))
                {
                    skeletonDirection[i] *= -1;
                }
                else
                {
                    skeletonsX[i] = newX;
                }

                player_gravity(lvl, offset_y, skeletonVelocityY[i], skeletonOnGround[i],
                               gravity, terminal_Velocity, skeletonsX[i], skeletonsY[i],
                               cell_size, SkeletonHeight, SkeletonWidth, height, width, dt);

                if (skeletonOnGround[i])
                    skeletonStableFrames[i]++;
                else
                    skeletonStableFrames[i] = 0;

                skeletonJumpTimer[i] += dt;

                if (skeletonOnGround[i] && skeletonStableFrames[i] > 350 && !skeletonShouldJump[i])
                {
                    if (rand() % 100 < 1)
                    {
                        int currentRow = (int)(skeletonsY[i] + SkeletonHeight) / cell_size;
                        int skeletonCol = (int)(skeletonsX[i] + SkeletonWidth / 2) / cell_size;
                        bool platformAbove = false;

                        for (int checkRow = currentRow - 5; checkRow < currentRow - 1; checkRow++)
                        {
                            if (checkRow >= 0)
                            {
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

                        if (platformAbove)
                        {
                            skeletonShouldJump[i] = true;
                            skeletonJumpTimer[i] = 0.f;
                        }
                    }
                }

                if (skeletonShouldJump[i] && skeletonOnGround[i] && skeletonStableFrames[i] > 30)
                {
                    skeletonVelocityY[i] = jumpStrength;
                    skeletonOnGround[i] = false;
                    skeletonShouldJump[i] = false;
                    skeletonStableFrames[i] = 0;
                }

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
            }

            // level 2 enemy loops
            if (currentLevel == 2)
            {
                // invisible Man loop
                for (int i = 0; i < invisibleCount; i++)
                {
                    invisibleVisibilityTimer[i] += dt;
                    if (invisibleVisibilityTimer[i] >= 3.0f)
                    {
                        invisibleVisibilityTimer[i] = 0.0f;
                        invisibleIsVisible[i] = !invisibleIsVisible[i];
                    }

                    invisibleTeleportTimer[i] += dt;
                    if (invisibleTeleportTimer[i] >= 5.0f)
                    {
                        invisibleTeleportTimer[i] = 0.0f;
                        int newCol = 2 + (rand() % 15);
                        int newRow = 1 + (rand() % 8);
                        invisiblesX[i] = newCol * cell_size;
                        invisiblesY[i] = newRow * cell_size;
                    }

                    invisiblesX[i] += invisibleSpeed[i] * invisibleDirection[i] * dt;

                    if (invisiblesX[i] <= cell_size)
                    {
                        invisiblesX[i] = cell_size;
                        invisibleDirection[i] = 1;
                    }
                    else if (invisiblesX[i] >= 17 * cell_size)
                    {
                        invisiblesX[i] = 17 * cell_size;
                        invisibleDirection[i] = -1;
                    }

                    player_gravity(lvl, offset_y, invisibleVelocityY[i], invisibleOnGround[i],
                                   gravity, terminal_Velocity, invisiblesX[i], invisiblesY[i],
                                   cell_size, InvisibleHeight, InvisibleWidth, height, width, dt);

                    if (invisibleIsVisible[i] && !invisibleIsCaught[i])
                    {
                        if (collisionDetection(window, player_x, player_y, invisiblesX[i], invisiblesY[i],
                                               PlayerWidth, PlayerHeight, InvisibleWidth, InvisibleHeight, isDead))
                        {
                            if (!waitingToRespawn)
                            {
                                playerLives--;
                                waitingToRespawn = true;
                                deathDelayCounter = 0.0f;
                                levelNoDamage = false;
                                playerScore -= 50;
                            }
                        }
                    }
                }

                // chelnov loop
                for (int i = 0; i < chelnovCount; i++)
                {
                    chelnovShootTimer[i] += dt;

                    if (chelnovIsShooting[i])
                    {
                        chelnovShootPhaseTimer[i] += dt;
                        if (chelnovShootPhaseTimer[i] >= 1.0f)
                        {
                            chelnovIsShooting[i] = false;
                            chelnovShootPhaseTimer[i] = 0.0f;
                        }
                    }

                    if (chelnovShootTimer[i] >= 4.0f)
                    {
                        chelnovShootTimer[i] = 0.0f;
                        chelnovIsShooting[i] = true;
                        chelnovShootPhaseTimer[i] = 0.0f;

                        if (chelnovProjCount < maxChelnovProjectiles)
                        {
                            chelnovProjX[chelnovProjCount] = chelnovsX[i];
                            chelnovProjY[chelnovProjCount] = chelnovsY[i] + ChelnovHeight / 2;
                            chelnovProjDirection[chelnovProjCount] = (player_x > chelnovsX[i]) ? 1 : -1;
                            chelnovProjActive[chelnovProjCount] = true;

                            chelnovProjAnimFrame[chelnovProjCount] = 0;
                            chelnovProjAnimCounter[chelnovProjCount] = 0;

                            chelnovProjCount++;
                        }
                    }

                    chelnovsX[i] += chelnovSpeed[i] * chelnovDirection[i] * dt;

                    if (chelnovsX[i] <= cell_size)
                    {
                        chelnovsX[i] = cell_size;
                        chelnovDirection[i] = 1;
                    }
                    else if (chelnovsX[i] >= 17 * cell_size)
                    {
                        chelnovsX[i] = 17 * cell_size;
                        chelnovDirection[i] = -1;
                    }

                    player_gravity(lvl, offset_y, chelnovVelocityY[i], chelnovOnGround[i],
                                   gravity, terminal_Velocity, chelnovsX[i], chelnovsY[i],
                                   cell_size, ChelnovHeight, ChelnovWidth, height, width, dt);

                    if (!chelnovIsCaught[i])
                    {
                        if (collisionDetection(window, player_x, player_y, chelnovsX[i], chelnovsY[i],
                                               PlayerWidth, PlayerHeight, ChelnovWidth, ChelnovHeight, isDead))
                        {
                            if (!waitingToRespawn)
                            {
                                playerLives--;
                                waitingToRespawn = true;
                                deathDelayCounter = 0.0f;
                                levelNoDamage = false;
                                playerScore -= 50;
                            }
                        }
                    }
                }

                // chelnov projectile loop
                for (int i = 0; i < chelnovProjCount; i++)
                {
                    if (!chelnovProjActive[i])
                        continue;

                    chelnovProjX[i] += 150.0f * chelnovProjDirection[i] * dt;

                    if (chelnovProjX[i] < 0 || chelnovProjX[i] > screen_x)
                    {
                        chelnovProjActive[i] = false;
                        chelnovProjX[i] = chelnovProjX[chelnovProjCount - 1];
                        chelnovProjY[i] = chelnovProjY[chelnovProjCount - 1];
                        chelnovProjDirection[i] = chelnovProjDirection[chelnovProjCount - 1];
                        chelnovProjActive[i] = chelnovProjActive[chelnovProjCount - 1];
                        chelnovProjCount--;
                        i--;
                        continue;
                    }

                    if ((player_x < chelnovProjX[i] + 30) &&
                        (player_x + PlayerWidth > chelnovProjX[i]) &&
                        (player_y < chelnovProjY[i] + 30) &&
                        (player_y + PlayerHeight > chelnovProjY[i]))
                    {
                        if (!waitingToRespawn)
                        {
                            playerLives--;
                            waitingToRespawn = true;
                            deathDelayCounter = 0.0f;
                            levelNoDamage = false;
                            playerScore -= 50;

                            chelnovProjActive[i] = false;
                            chelnovProjX[i] = chelnovProjX[chelnovProjCount - 1];
                            chelnovProjY[i] = chelnovProjY[chelnovProjCount - 1];
                            chelnovProjDirection[i] = chelnovProjDirection[chelnovProjCount - 1];
                            chelnovProjActive[i] = chelnovProjActive[chelnovProjCount - 1];
                            chelnovProjCount--;
                            i--;
                        }
                    }
                }
            }

            // projectile update loop
            for (int p = 0; p < projectileCount; p++)
            {
                if (!projectileActive[p])
                    continue;

                projectileLifespan[p] += dt;
                if (projectileLifespan[p] >= MAX_PROJECTILE_LIFE)
                {
                    // remove old projectile
                    projectilesX[p] = projectilesX[projectileCount - 1];
                    projectilesY[p] = projectilesY[projectileCount - 1];
                    projectileType[p] = projectileType[projectileCount - 1];
                    projectileDirection[p] = projectileDirection[projectileCount - 1];
                    projectileVelocityY[p] = projectileVelocityY[projectileCount - 1];
                    projectileActive[p] = projectileActive[projectileCount - 1];
                    projectileOnGround[p] = projectileOnGround[projectileCount - 1];
                    projectileAnimFrame[p] = projectileAnimFrame[projectileCount - 1];
                    projectileAnimCounter[p] = projectileAnimCounter[projectileCount - 1];
                    projectileLifespan[p] = projectileLifespan[projectileCount - 1];
                    projectileCount--;
                    p--;
                    continue;
                }

                // update animation frame
                projectileAnimCounter[p]++;
                if (projectileAnimCounter[p] >= projectileAnimSpeed)
                {
                    projectileAnimCounter[p] = 0;
                    projectileAnimFrame[p]++;
                    if (projectileAnimFrame[p] >= 4)
                        projectileAnimFrame[p] = 0;
                }

                float newProjX = projectilesX[p] + projectileSpeed * projectileDirection[p] * dt;

                // use appropriate level map and dimensions for projectile physics
                int useCellSize = (currentLevel == 3) ? level3CellSize : cell_size;
                int useHeight = (currentLevel == 3) ? level3Height : height;
                int useWidth = (currentLevel == 3) ? level3Width : width;
                char **useMap = (currentLevel == 3) ? bossLvl : lvl;

                int projRow = (int)(projectilesY[p] + ProjectileHeight / 2) / useCellSize;
                int projColRight = (int)(newProjX + ProjectileWidth) / useCellSize;
                int projColLeft = (int)newProjX / useCellSize;

                char rightWall = get_tile(useMap, projRow, projColRight, useHeight, useWidth);
                char leftWall = get_tile(useMap, projRow, projColLeft, useHeight, useWidth);

                if (projectileDirection[p] == 1 && rightWall == '#')
                    projectileDirection[p] = -1;
                else if (projectileDirection[p] == -1 && leftWall == '#')
                    projectileDirection[p] = 1;
                else
                    projectilesX[p] = newProjX;

                float newProjY = projectilesY[p] + projectileVelocityY[p] * dt;

                int feetRow = (int)(newProjY + ProjectileHeight) / useCellSize;
                int feetColL = (int)projectilesX[p] / useCellSize;
                int feetColR = (int)(projectilesX[p] + ProjectileWidth) / useCellSize;

                char floorL = get_tile(useMap, feetRow, feetColL, useHeight, useWidth);
                char floorR = get_tile(useMap, feetRow, feetColR, useHeight, useWidth);

                int headRow = (int)newProjY / useCellSize;
                char ceilL = get_tile(useMap, headRow, feetColL, useHeight, useWidth);
                char ceilR = get_tile(useMap, headRow, feetColR, useHeight, useWidth);

                bool landed = false;

                if (projectileVelocityY[p] < 0)
                {
                    if (ceilL == '#' || ceilR == '#')
                    {
                        projectileVelocityY[p] = 0;
                        projectilesY[p] = (headRow + 1) * useCellSize;
                    }
                }

                if (projectileVelocityY[p] >= 0)
                {
                    if (floorL == '#' || floorR == '#')
                        landed = true;
                    else if (floorL == '-' || floorR == '-')
                    {
                        float blockTop = feetRow * useCellSize;
                        if ((projectilesY[p] + ProjectileHeight <= blockTop + 4.0f) && (newProjY + ProjectileHeight >= blockTop))
                            landed = true;
                    }
                }

                if (landed)
                {
                    projectileOnGround[p] = true;
                    projectileVelocityY[p] = 0;
                    projectilesY[p] = (feetRow * useCellSize) - ProjectileHeight;
                }
                else
                {
                    projectileOnGround[p] = false;
                    projectilesY[p] = newProjY;
                    projectileVelocityY[p] += gravity * dt;
                    if (projectileVelocityY[p] > terminal_Velocity)
                        projectileVelocityY[p] = terminal_Velocity;
                }

                bool shouldRemoveProjectile = false;

                if (projectilesX[p] < -100 || projectilesX[p] > screen_x + 100 ||
                    projectilesY[p] < -100 || projectilesY[p] > screen_y + 100)
                    shouldRemoveProjectile = true;

                int currentRow = (int)(projectilesY[p] + ProjectileHeight) / cell_size;
                if (currentRow >= bottomFloorRow - 1)
                {
                    if (projectilesX[p] <= bottomFloorLeftEdge || projectilesX[p] + ProjectileWidth >= bottomFloorRightEdge)
                        shouldRemoveProjectile = true;
                }

                if (shouldRemoveProjectile)
                {
                    projectilesX[p] = projectilesX[projectileCount - 1];
                    projectilesY[p] = projectilesY[projectileCount - 1];
                    projectileType[p] = projectileType[projectileCount - 1];
                    projectileDirection[p] = projectileDirection[projectileCount - 1];
                    projectileVelocityY[p] = projectileVelocityY[projectileCount - 1];
                    projectileActive[p] = projectileActive[projectileCount - 1];
                    projectileOnGround[p] = projectileOnGround[projectileCount - 1];
                    projectileAnimFrame[p] = projectileAnimFrame[projectileCount - 1];
                    projectileAnimCounter[p] = projectileAnimCounter[projectileCount - 1];
                    projectileCount--;
                    p--;
                    continue;
                }

                // collision with ghosts
                for (int e = 0; e < enemyCount; e++)
                {
                    if ((projectilesX[p] < enemiesX[e] + EnemyWidth) &&
                        (projectilesX[p] + ProjectileWidth > enemiesX[e]) &&
                        (projectilesY[p] < enemiesY[e] + EnemyHeight) &&
                        (projectilesY[p] + ProjectileHeight > enemiesY[e]))
                    {
                        int defeatPoints = 50 * 2;
                        if (enemiesY[e] < 400)
                            playerScore += 150;
                        addScore(playerScore, comboStreak, comboTimer, defeatPoints, true, multiKillCount, multiKillTimer, dt);
                        multiKillCount++;
                        multiKillTimer = 0.0f;

                        enemiesX[e] = enemiesX[enemyCount - 1];
                        enemiesY[e] = enemiesY[enemyCount - 1];
                        enemySpeed[e] = enemySpeed[enemyCount - 1];
                        enemyDirection[e] = enemyDirection[enemyCount - 1];
                        platformLeftEdge[e] = platformLeftEdge[enemyCount - 1];
                        platformRightEdge[e] = platformRightEdge[enemyCount - 1];
                        enemyIsCaught[e] = enemyIsCaught[enemyCount - 1];
                        enemyCount--;
                        e--;
                    }
                }

                // collision with skeletons
                for (int s = 0; s < skeletonCount; s++)
                {
                    if ((projectilesX[p] < skeletonsX[s] + SkeletonWidth) &&
                        (projectilesX[p] + ProjectileWidth > skeletonsX[s]) &&
                        (projectilesY[p] < skeletonsY[s] + SkeletonHeight) &&
                        (projectilesY[p] + ProjectileHeight > skeletonsY[s]))
                    {
                        int defeatPoints = 75 * 2;
                        if (!skeletonOnGround[s])
                            playerScore += 150;
                        addScore(playerScore, comboStreak, comboTimer, defeatPoints, true, multiKillCount, multiKillTimer, dt);
                        multiKillCount++;
                        multiKillTimer = 0.0f;

                        skeletonsX[s] = skeletonsX[skeletonCount - 1];
                        skeletonsY[s] = skeletonsY[skeletonCount - 1];
                        skeletonSpeed[s] = skeletonSpeed[skeletonCount - 1];
                        skeletonDirection[s] = skeletonDirection[skeletonCount - 1];
                        skeletonVelocityY[s] = skeletonVelocityY[skeletonCount - 1];
                        skeletonOnGround[s] = skeletonOnGround[skeletonCount - 1];
                        skeletonJumpTimer[s] = skeletonJumpTimer[skeletonCount - 1];
                        skeletonJumpCooldown[s] = skeletonJumpCooldown[skeletonCount - 1];
                        skeletonShouldJump[s] = skeletonShouldJump[skeletonCount - 1];
                        skeletonStableFrames[s] = skeletonStableFrames[skeletonCount - 1];
                        skeletonIsCaught[s] = skeletonIsCaught[skeletonCount - 1];
                        skeletonAnimFrame[s] = skeletonAnimFrame[skeletonCount - 1];
                        skeletonAnimCounter[s] = skeletonAnimCounter[skeletonCount - 1];
                        skeletonCount--;
                        s--;
                    }
                }

                // collision with Invisible Man and Chelnov
                if (currentLevel == 2)
                {
                    for (int inv = 0; inv < invisibleCount; inv++)
                    {
                        if ((projectilesX[p] < invisiblesX[inv] + InvisibleWidth) &&
                            (projectilesX[p] + ProjectileWidth > invisiblesX[inv]) &&
                            (projectilesY[p] < invisiblesY[inv] + InvisibleHeight) &&
                            (projectilesY[p] + ProjectileHeight > invisiblesY[inv]))
                        {
                            int defeatPoints = 150 * 2;
                            addScore(playerScore, comboStreak, comboTimer, defeatPoints, true, multiKillCount, multiKillTimer, dt);
                            multiKillCount++;
                            multiKillTimer = 0.0f;

                            invisiblesX[inv] = invisiblesX[invisibleCount - 1];
                            invisiblesY[inv] = invisiblesY[invisibleCount - 1];
                            invisibleCount--;
                            inv--;
                        }
                    }

                    for (int ch = 0; ch < chelnovCount; ch++)
                    {
                        if ((projectilesX[p] < chelnovsX[ch] + ChelnovWidth) &&
                            (projectilesX[p] + ProjectileWidth > chelnovsX[ch]) &&
                            (projectilesY[p] < chelnovsY[ch] + ChelnovHeight) &&
                            (projectilesY[p] + ProjectileHeight > chelnovsY[ch]))
                        {
                            int defeatPoints = 200 * 2;
                            addScore(playerScore, comboStreak, comboTimer, defeatPoints, true, multiKillCount, multiKillTimer, dt);
                            multiKillCount++;
                            multiKillTimer = 0.0f;

                            chelnovsX[ch] = chelnovsX[chelnovCount - 1];
                            chelnovsY[ch] = chelnovsY[chelnovCount - 1];
                            chelnovCount--;
                            ch--;
                        }
                    }
                }

                
                if (currentLevel == 3)
                {
                    bool projectileRemoved = false;

                    // projectile collision with pot
                    if (potActive && !potDestroyed && !projectileRemoved)
                    {
                        if ((projectilesX[p] < potX + potWidth) &&
                            (projectilesX[p] + ProjectileWidth > potX) &&
                            (projectilesY[p] < potY + potHeight) &&
                            (projectilesY[p] + ProjectileHeight > potY))
                        {
                           
                            potHealth--;
                            playerScore += 200;
                            cout << "POT HIT! Health: " << potHealth << "/" << potMaxHealth << endl;

                            if (potHealth <= 0)
                            {
                                potActive = false;
                                potDestroyed = true;
                                potDestroyTimer = 0.0f;
                                playerScore += 1000;
                                cout << "POT DESTROYED! Boss will appear soon..." << endl;
                            }

                            // remove the projectile
                            projectilesX[p] = projectilesX[projectileCount - 1];
                            projectilesY[p] = projectilesY[projectileCount - 1];
                            projectileType[p] = projectileType[projectileCount - 1];
                            projectileDirection[p] = projectileDirection[projectileCount - 1];
                            projectileVelocityY[p] = projectileVelocityY[projectileCount - 1];
                            projectileActive[p] = projectileActive[projectileCount - 1];
                            projectileOnGround[p] = projectileOnGround[projectileCount - 1];
                            projectileAnimFrame[p] = projectileAnimFrame[projectileCount - 1];
                            projectileAnimCounter[p] = projectileAnimCounter[projectileCount - 1];
                            projectileCount--;
                            p--;
                            projectileRemoved = true;
                        }
                    }

                    // projectile collision with pot enemies
                    if (!projectileRemoved)
                    {
                        for (int pe = 0; pe < potEnemyCount && !projectileRemoved; pe++)
                        {
                            if (potEnemyIsCaught[pe])
                                continue;

                            
                            int enemyW = 72, enemyH = 60; // ghost
                            if (potEnemyType[pe] == 2)
                            {
                                enemyW = 72;
                                enemyH = 92;
                            } // Skeleton
                            else if (potEnemyType[pe] == 3)
                            {
                                enemyW = 60;
                                enemyH = 80;
                            } // Invisible
                            else if (potEnemyType[pe] == 4)
                            {
                                enemyW = 60;
                                enemyH = 90;
                            } // Chelnov

                            if ((projectilesX[p] < potEnemiesX[pe] + enemyW) &&
                                (projectilesX[p] + ProjectileWidth > potEnemiesX[pe]) &&
                                (projectilesY[p] < potEnemiesY[pe] + enemyH) &&
                                (projectilesY[p] + ProjectileHeight > potEnemiesY[pe]))
                            {
                                // defeat pot enemy , points based on type
                                int defeatPoints = 50; // Ghost
                                if (potEnemyType[pe] == 2)
                                    defeatPoints = 75; // Skeleton
                                else if (potEnemyType[pe] == 3)
                                    defeatPoints = 150; // Invisible
                                else if (potEnemyType[pe] == 4)
                                    defeatPoints = 200; // Chelnov

                                addScore(playerScore, comboStreak, comboTimer, defeatPoints * 2, true, multiKillCount, multiKillTimer, dt);
                                multiKillCount++;
                                multiKillTimer = 0.0f;

                                // remove pot enemy by shifting array (dynamic resize down)
                                for (int j = pe; j < potEnemyCount - 1; j++)
                                {
                                    potEnemiesX[j] = potEnemiesX[j + 1];
                                    potEnemiesY[j] = potEnemiesY[j + 1];
                                    potEnemySpeed[j] = potEnemySpeed[j + 1];
                                    potEnemyDirection[j] = potEnemyDirection[j + 1];
                                    potEnemyVelocityY[j] = potEnemyVelocityY[j + 1];
                                    potEnemyOnGround[j] = potEnemyOnGround[j + 1];
                                    potEnemyIsCaught[j] = potEnemyIsCaught[j + 1];
                                    potEnemyType[j] = potEnemyType[j + 1];
                                }
                                potEnemyCount--;
                                pe--;

                                cout << "Pot enemy defeated! Remaining: " << potEnemyCount << endl;
                            }
                        }
                    }

                    // projectile collision with boss
                    if (bossAppeared && !bossDefeated && !projectileRemoved)
                    {
                        // only hits the upper portion of the boss (the head)
                        float bossHeadY = bossY;
                        float bossHeadHeight = bossHeight * 0.5f; // head is top half

                        if ((projectilesX[p] < bossX + bossWidth) &&
                            (projectilesX[p] + ProjectileWidth > bossX) &&
                            (projectilesY[p] < bossHeadY + bossHeadHeight) &&
                            (projectilesY[p] + ProjectileHeight > bossHeadY))
                        {
                            // HIT THE BOSS!
                            bossHealth--;
                            playerScore += 500;
                            cout << "BOSS HIT! Health: " << bossHealth << "/" << maxBossHealth << endl;

                            if (bossHealth <= 0)
                            {
                                bossDefeated = true;
                                playerScore += 5000;
                                cout << "BOSS DEFEATED!" << endl;
                            }

                            // remove the projectile
                            projectilesX[p] = projectilesX[projectileCount - 1];
                            projectilesY[p] = projectilesY[projectileCount - 1];
                            projectileType[p] = projectileType[projectileCount - 1];
                            projectileDirection[p] = projectileDirection[projectileCount - 1];
                            projectileVelocityY[p] = projectileVelocityY[projectileCount - 1];
                            projectileActive[p] = projectileActive[projectileCount - 1];
                            projectileOnGround[p] = projectileOnGround[projectileCount - 1];
                            projectileAnimFrame[p] = projectileAnimFrame[projectileCount - 1];
                            projectileAnimCounter[p] = projectileAnimCounter[projectileCount - 1];
                            projectileCount--;
                            p--;
                            projectileRemoved = true;
                        }
                    }

                    // projectile collision with tentacles
                    
                    if (bossAppeared && !projectileRemoved)
                    {
                        for (int t = 0; t < tentacleCount && !projectileRemoved; t++)
                        {
                            if (!tentacleActive[t])
                                continue;

                            if ((projectilesX[p] < tentaclesX[t] + tentacleWidth[t]) &&
                                (projectilesX[p] + ProjectileWidth > tentaclesX[t]) &&
                                (projectilesY[p] < tentaclesY[t] + tentacleHeight[t]) &&
                                (projectilesY[p] + ProjectileHeight > tentaclesY[t]))
                            {
                                // minion reborns when projectile will hit the tentacle
                                if (minionCount < maxMinions)
                                {
                                    // dynamic resizing for reborn
                                    // allocate new arrays of size + 1
                                    int newSize = minionCount + 1;
                                    float *newX = new float[newSize];
                                    float *newY = new float[newSize];
                                    float *newSpeed = new float[newSize];
                                    int *newDir = new int[newSize];
                                    float *newVelY = new float[newSize];
                                    bool *newGround = new bool[newSize];
                                    bool *newCaught = new bool[newSize];
                                    bool *newFollow = new bool[newSize];

                                    //copy existing data
                                    for (int i = 0; i < minionCount; i++)
                                    {
                                        newX[i] = minionsX[i];
                                        newY[i] = minionsY[i];
                                        newSpeed[i] = minionSpeed[i];
                                        newDir[i] = minionDirection[i];
                                        newVelY[i] = minionVelocityY[i];
                                        newGround[i] = minionOnGround[i];
                                        newCaught[i] = minionIsCaught[i];
                                        newFollow[i] = minionFollowingPlayer[i];
                                    }

                                    // delete old arrays
                                    if (minionsX != NULL)
                                        delete[] minionsX;
                                    if (minionsY != NULL)
                                        delete[] minionsY;
                                    if (minionSpeed != NULL)
                                        delete[] minionSpeed;
                                    if (minionDirection != NULL)
                                        delete[] minionDirection;
                                    if (minionVelocityY != NULL)
                                        delete[] minionVelocityY;
                                    if (minionOnGround != NULL)
                                        delete[] minionOnGround;
                                    if (minionIsCaught != NULL)
                                        delete[] minionIsCaught;
                                    if (minionFollowingPlayer != NULL)
                                        delete[] minionFollowingPlayer;

                                    // point to new arrays
                                    minionsX = newX;
                                    minionsY = newY;
                                    minionSpeed = newSpeed;
                                    minionDirection = newDir;
                                    minionVelocityY = newVelY;
                                    minionOnGround = newGround;
                                    minionIsCaught = newCaught;
                                    minionFollowingPlayer = newFollow;

                                    // add new minion at the projectile's location
                                    minionsX[minionCount] = projectilesX[p];
                                    minionsY[minionCount] = projectilesY[p];
                                    minionSpeed[minionCount] = 30.0f + (rand() % 20);
                                    minionDirection[minionCount] = (rand() % 2 == 0) ? -1 : 1;
                                    minionVelocityY[minionCount] = 0;
                                    minionOnGround[minionCount] = false;
                                    minionIsCaught[minionCount] = false;
                                    minionFollowingPlayer[minionCount] = bossIsAngry;

                                    minionCount++;
                                    cout << "Minion REBORN from tentacle hit! Total: " << minionCount << endl;
                                }

                                // remove the projectile
                                projectilesX[p] = projectilesX[projectileCount - 1];
                                projectilesY[p] = projectilesY[projectileCount - 1];
                                projectileType[p] = projectileType[projectileCount - 1];
                                projectileDirection[p] = projectileDirection[projectileCount - 1];
                                projectileVelocityY[p] = projectileVelocityY[projectileCount - 1];
                                projectileActive[p] = projectileActive[projectileCount - 1];
                                projectileOnGround[p] = projectileOnGround[projectileCount - 1];
                                projectileAnimFrame[p] = projectileAnimFrame[projectileCount - 1];
                                projectileAnimCounter[p] = projectileAnimCounter[projectileCount - 1];
                                projectileCount--;
                                p--;
                                projectileRemoved = true;
                            }
                        }
                    }

                    // projectile collision with minions
                    if (bossAppeared && !projectileRemoved)
                    {
                        for (int m = 0; m < minionCount; m++)
                        {
                            if ((projectilesX[p] < minionsX[m] + minionWidth) &&
                                (projectilesX[p] + ProjectileWidth > minionsX[m]) &&
                                (projectilesY[p] < minionsY[m] + minionHeight) &&
                                (projectilesY[p] + ProjectileHeight > minionsY[m]))
                            {
                                // Defeat minion
                                int defeatPoints = 100;
                                addScore(playerScore, comboStreak, comboTimer, defeatPoints, true, multiKillCount, multiKillTimer, dt);
                                multiKillCount++;
                                multiKillTimer = 0.0f;

                                // Remove minion
                                minionsX[m] = minionsX[minionCount - 1];
                                minionsY[m] = minionsY[minionCount - 1];
                                minionSpeed[m] = minionSpeed[minionCount - 1];
                                minionDirection[m] = minionDirection[minionCount - 1];
                                minionVelocityY[m] = minionVelocityY[minionCount - 1];
                                minionOnGround[m] = minionOnGround[minionCount - 1];
                                minionIsCaught[m] = minionIsCaught[minionCount - 1];
                                minionFollowingPlayer[m] = minionFollowingPlayer[minionCount - 1];
                                minionCount--;
                                m--;
                            }
                        }
                    }
                }
                
            }

           
            // Check if level is complete
            bool allEnemiesDefeated = false;
            if (currentLevel == 1)
            {
                allEnemiesDefeated = (enemyCount == 0 && skeletonCount == 0 && capturedCount == 0 && projectileCount == 0);
            }
            else if (currentLevel == 2)
            {
                // for Level 2 with wave spawning, only complete when all the waves are done
                if (useWaveSpawning)
                {
                    // level complete only if,  all waves spawned and all enemies defeated
                    allEnemiesDefeated = (currentWave >= maxWaves &&
                                          enemyCount == 0 && skeletonCount == 0 &&
                                          invisibleCount == 0 && chelnovCount == 0 &&
                                          capturedCount == 0 && chelnovProjCount == 0 &&
                                          projectileCount == 0);
                }
                else
                {
                    // original behavior if wave spawning is disabled
                    allEnemiesDefeated = (enemyCount == 0 && skeletonCount == 0 &&
                                          invisibleCount == 0 && chelnovCount == 0 &&
                                          capturedCount == 0 && chelnovProjCount == 0 &&
                                          projectileCount == 0);
                }
            }
            else if (currentLevel == 3)
            {
                // boss level complete when boss is defeated
                allEnemiesDefeated = bossDefeated;
            }

            
            static int debugCounter = 0;
            debugCounter++;
            if (debugCounter >= 60)
            {
                debugCounter = 0;
                cout << "Level " << currentLevel << " - Ghosts: " << enemyCount
                     << " Skeletons: " << skeletonCount
                     << " Invisible: " << invisibleCount
                     << " Chelnov: " << chelnovCount
                     << " Captured: " << capturedCount
                     << " ChelProj: " << chelnovProjCount
                     << " YourProj: " << projectileCount
                     << " AllDefeated: " << allEnemiesDefeated << endl;
            }

            if (allEnemiesDefeated && !showStageClear)
            {
                showStageClear = true;

                if (currentLevel == 1)
                {
                    playerScore += 1000;
                    if (levelNoDamage)
                        playerScore += 1500;
                    if (levelTimer < 30.0f)
                        playerScore += 2000;
                    else if (levelTimer < 45.0f)
                        playerScore += 1000;
                    else if (levelTimer < 60.0f)
                        playerScore += 500;
                }
                else if (currentLevel == 2)
                {
                    playerScore += 2000;
                    if (levelNoDamage)
                        playerScore += 2500;
                    if (levelTimer < 60.0f)
                        playerScore += 3000;
                    else if (levelTimer < 90.0f)
                        playerScore += 1500;
                    else if (levelTimer < 120.0f)
                        playerScore += 750;
                }
                else if (currentLevel == 3)
                {
                    playerScore += 10000; // Big bonus for defeating boss
                    if (levelNoDamage)
                        playerScore += 5000;
                    if (levelTimer < 120.0f)
                        playerScore += 5000;
                    else if (levelTimer < 180.0f)
                        playerScore += 2500;
                    else if (levelTimer < 240.0f)
                        playerScore += 1000;
                }

                if (speedMultiplier == 1.5f)
                    playerScore += 500;
                else if (vacuumPower == 1.2f)
                    playerScore += 500;
            }

            // Stage Clear screen
            if (showStageClear)
            {
                bool waitingForNext = true;
                while (waitingForNext && window.isOpen())
                {
                    Event stageClearEvent;
                    while (window.pollEvent(stageClearEvent))
                    {
                        if (stageClearEvent.type == Event::Closed)
                        {
                            window.close();
                            return 0;
                        }

                        if (stageClearEvent.type == Event::KeyPressed &&
                            stageClearEvent.key.code == Keyboard::Enter)
                        {
                            waitingForNext = false;
                            showStageClear = false;

                            if (currentLevel == 1)
                            {
                                currentLevel = 2;

                                // enable wave spawning
                                useWaveSpawning = true;
                                currentWave = 0;
                                waveTimer = 0.0f;
                                for (int i = 0; i < 4; i++)
                                    waveSpawned[i] = false;

                                // generate level without spawning all enemies
                                generateLevel2Map(lvl, height, width, cell_size,
                                                  enemiesX, enemiesY, enemySpeed, enemyDirection,
                                                  platformLeftEdge, platformRightEdge, enemyCount,
                                                  skeletonsX, skeletonsY, skeletonSpeed, skeletonDirection,
                                                  skeletonVelocityY, skeletonOnGround, skeletonJumpTimer,
                                                  skeletonJumpCooldown, skeletonShouldJump, skeletonStableFrames,
                                                  skeletonAnimFrame, skeletonAnimCounter, skeletonCount,
                                                  invisiblesX, invisiblesY, invisibleSpeed, invisibleDirection,
                                                  invisibleVelocityY, invisibleOnGround, invisibleIsVisible,
                                                  invisibleVisibilityTimer, invisibleTeleportTimer, invisibleCount,
                                                  chelnovsX, chelnovsY, chelnovSpeed, chelnovDirection,
                                                  chelnovVelocityY, chelnovOnGround, chelnovShootTimer,
                                                  chelnovIsShooting, chelnovShootPhaseTimer, chelnovCount,
                                                  false); 

                                MAX_CAPACITY = 5;

                                lvlMusic.stop();
                                lvl2Music.play();

                                player_x = 850.0f;
                                player_y = 450.0f;
                                velocityY = 0;
                                onGround = false;

                                levelNoDamage = true;
                                levelTimer = 0.0f;
                                playerLives = 3;

                                capturedCount = 0;
                                projectileCount = 0;
                                for (int i = 0; i < MAX_PROJECTILES; i++)
                                    projectileActive[i] = false;

                                powerupCount = 0;
                                hasSpeedBoost = false;
                                hasRangeBoost = false;
                                hasPowerBoost = false;
                                speed = originalSpeed * speedMultiplier;
                                vacuumPower = originalVacuumPower;

                                for (int i = 0; i < 3; i++)
                                {
                                    spawnPowerup(powerupsX, powerupsY, powerupType, powerupActive,
                                                 powerupAnimTimer, powerupCount, maxPowerups,
                                                 lvl, width, height, cell_size);
                                }
                            }
                            else if (currentLevel == 2)
                            {
                                // transition to boss level
                                currentLevel = 3;

                                // allocate dynamic arrays for minions
                                minionsX = NULL;
                                minionsY = NULL;
                                minionSpeed = NULL;
                                minionDirection = NULL;
                                minionVelocityY = NULL;
                                minionOnGround = NULL;
                                minionIsCaught = NULL;
                                minionFollowingPlayer = NULL;
                                minionCount = 0;

                                for (int i = 0; i < maxMinions; i++)
                                {
                                    minionsX[i] = 0;
                                    minionsY[i] = 0;
                                    minionSpeed[i] = 0;
                                    minionDirection[i] = 0;
                                    minionVelocityY[i] = 0;
                                    minionOnGround[i] = false;
                                    minionIsCaught[i] = false;
                                    minionFollowingPlayer[i] = false;
                                }

                                // allocate dynamic arrays for tentacles
                                tentaclesX = NULL;
                                tentaclesY = NULL;
                                tentacleWidth = NULL;
                                tentacleHeight = NULL;
                                tentacleTimer = NULL;
                                tentacleDuration = NULL;
                                tentacleActive = NULL;
                                tentacleCount = 0;

                                for (int i = 0; i < maxTentacles; i++)
                                {
                                    tentaclesX[i] = 0;
                                    tentaclesY[i] = 0;
                                    tentacleWidth[i] = 40;
                                    tentacleHeight[i] = 120;
                                    tentacleTimer[i] = 0;
                                    tentacleDuration[i] = 0;
                                    tentacleActive[i] = false;
                                }

                               

                                // Create boss level map with proper dimensions
                                bossLvl = new char *[level3Height];
                                for (int i = 0; i < level3Height; i++)
                                {
                                    bossLvl[i] = new char[level3Width];
                                    for (int j = 0; j < level3Width; j++)
                                        bossLvl[i][j] = ' ';
                                }

                                // calculate floor row to ensure visibility
                                int floorRow = level3Height - 3; // floor at third row from bottom
                                float floorY = floorRow * level3CellSize;

                                // Debug information
                                cout << "Creating Level 3 map..." << endl;
                                cout << "Dimensions: " << level3Width << "x" << level3Height << endl;
                                cout << "Cell size: " << level3CellSize << endl;
                                cout << "Floor row: " << floorRow << " (Y position: " << floorY << ")" << endl;
                                cout << "Screen size: " << screen_x << "x" << screen_y << endl;

                                // Create floor within bounds
                                for (int j = 0; j < level3Width; j++)
                                {
                                    if (floorRow * level3CellSize < screen_y) // only create if visible
                                        bossLvl[floorRow][j] = '#';
                                }

                                // create side walls 
                                for (int i = 0; i < level3Height; i++)
                                {
                                    // Left wall
                                    bossLvl[i][0] = '#';

                                    // right wall 
                                    if ((level3Width - 1) * level3CellSize < screen_x)
                                        bossLvl[i][level3Width - 1] = '#';
                                }

                                // creating platforms 
                               
                                int platformRow1 = floorRow - 5;
                                for (int j = 2; j < 8; j++)
                                {
                                    if (platformRow1 * level3CellSize < screen_y)
                                        bossLvl[platformRow1][j] = '-';
                                }

                                // bottom right platform (row 16)
                                for (int j = level3Width - 8; j < level3Width - 2; j++)
                                {
                                    if (platformRow1 * level3CellSize < screen_y)
                                        bossLvl[platformRow1][j] = '-';
                                }

                                // middle platform (row 14)
                                int platformRow2 = floorRow - 7;
                                for (int j = 12; j < 18; j++)
                                {
                                    if (platformRow2 * level3CellSize < screen_y)
                                        bossLvl[platformRow2][j] = '-';
                                }

                                // upper left platform (row 10)
                                int platformRow3 = floorRow - 11;
                                for (int j = 3; j < 9; j++)
                                {
                                    if (platformRow3 * level3CellSize < screen_y)
                                        bossLvl[platformRow3][j] = '-';
                                }

                                // upper right platform (row 10)
                                for (int j = level3Width - 9; j < level3Width - 3; j++)
                                {
                                    if (platformRow3 * level3CellSize < screen_y)
                                        bossLvl[platformRow3][j] = '-';
                                }

                                // top middle platform (row 7)
                                int platformRow4 = floorRow - 14;
                                for (int j = 11; j < 19; j++)
                                {
                                    if (platformRow4 * level3CellSize < screen_y)
                                        bossLvl[platformRow4][j] = '-';
                                }

                                // player starting position
                                player_x = screen_x / 2 - PlayerWidth / 2;
                                player_y = floorY - PlayerHeight - 5; // place player just above floor

                                cout << "Level 3 map created successfully!" << endl;
                                cout << "Player starting position: (" << player_x << ", " << player_y << ")" << endl;

                                // initialize boss
                                bossX = screen_x / 2 - bossWidth / 2;
                                bossY = 50;
                                bossHealth = maxBossHealth;
                                bossIsAngry = false;
                                bossDefeated = false;

                                minionSpawnTimer = 0.0f;
                                tentacleSpawnTimer = 0.0f;

                               
                                // Player starting position for boss level
                                player_x = screen_x / 2 - PlayerWidth / 2;
                                player_y = floorY - PlayerHeight - 5; // place player just above floor

                                velocityY = 0;
                                onGround = false;

                                MAX_CAPACITY = 999;

                                lvl2Music.stop();
                                lvl3Music.play();

                                levelNoDamage = true;
                                levelTimer = 0.0f;

                                capturedCount = 0;
                                projectileCount = 0;
                                for (int i = 0; i < MAX_PROJECTILES; i++)
                                    projectileActive[i] = false;

                                powerupCount = 0;
                                hasSpeedBoost = false;
                                hasRangeBoost = false;
                                hasPowerBoost = false;
                                speed = originalSpeed * speedMultiplier;
                                vacuumPower = originalVacuumPower;

                                cout << "Transitioning to Boss Level!" << endl;
                            }
                            else if (currentLevel == 3)
                            {
                                // game complete when boss is defeated
                                restartGame = true;
                                playagain = true;
                            }
                        }

                        if (stageClearEvent.type == Event::KeyPressed &&
                            stageClearEvent.key.code == Keyboard::Escape)
                        {
                            window.close();
                            return 0;
                        }
                    }

                    window.clear(Color(20, 20, 60));
                    window.draw(stageClearText);

                    if (currentLevel == 1)
                        stageBonusText.setString("Level 1 Complete!");
                    else if (currentLevel == 2)
                        stageBonusText.setString("Level 2 Complete!");
                    else if (currentLevel == 3)
                        stageBonusText.setString("BOSS DEFEATED! YOU WIN!");
                    window.draw(stageBonusText);

                    stageScoreText.setString("Score: " + to_string(playerScore));
                    window.draw(stageScoreText);

                    if (currentLevel == 1)
                        nextLevelText.setString("Press ENTER for Level 2");
                    else if (currentLevel == 2)
                        nextLevelText.setString("Press ENTER for Boss Level");
                    else
                        nextLevelText.setString("Press ENTER to Play Again");
                    window.draw(nextLevelText);

                    window.display();
                }
            }

            if (restartGame)
                break;

            // game over logic
            if (waitingToRespawn)
            {
                deathDelayCounter += dt;

                if (deathDelayCounter >= deathDelayTime)
                {
                    if (playerLives > 0)
                    {
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
                        showGameOver = true;
                        waitingToRespawn = false;
                    }
                }
            }

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

                            powerupCount = 0;
                            hasSpeedBoost = false;
                            hasRangeBoost = false;
                            hasPowerBoost = false;
                            speed = originalSpeed * speedMultiplier;
                            vacuumPower = originalVacuumPower;

                            projectileCount = 0;
                            capturedCount = 0;
                            for (int i = 0; i < MAX_PROJECTILES; i++)
                                projectileActive[i] = false;

                            burstModeActive = false;
                            burstFrameCounter = 0;
                            currentLevel = 1;
                            MAX_CAPACITY = 3;
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
                    livesRemainingText.setString("You ran out of lives!");
                    window.draw(livesRemainingText);
                    window.draw(restartText);
                    window.draw(escText);
                    window.display();
                }
            }

            if (restartGame)
                break;

            // rendering
            if (currentLevel == 1)
            {
                display_level(window, lvl, bgTex, bgSprite, blockTexture, blockSprite,
                              slopeLeftTexture, slopeLeftSprite, slopeRightTexture, slopeRightSprite, slopeLeftMirrorTexture, slopeLeftMirrorSprite, slopeRightMirrorTexture, slopeRightMirrorSprite,
                              height, width, cell_size);
            }
            else if (currentLevel == 2)
            {
                display_level(window, lvl, bgTex2, bgSprite2, blockTexture2, blockSprite2,
                              slopeLeftTexture, slopeLeftSprite, slopeRightTexture, slopeRightSprite, slopeLeftMirrorTexture, slopeLeftMirrorSprite, slopeRightMirrorTexture, slopeRightMirrorSprite,
                              height, width, cell_size);
            }
            else if (currentLevel == 3)
            {
                
                window.draw(bgSprite3);

               
                jumpStrength = -180.f;
               
                // Draw boss level tiles
                for (int i = 0; i < level3Height; i++)
                {
                    for (int j = 0; j < level3Width; j++)
                    {
                        if (bossLvl[i][j] == '#' || bossLvl[i][j] == '-')
                        {
                            blockSprite3.setPosition(j * level3CellSize, i * level3CellSize);
                            
                            float blockScale = (float)level3CellSize / 64.0f;
                            blockSprite3.setScale(blockScale, blockScale);
                            window.draw(blockSprite3);
                        }
                    }
                }

                
                cloudSprite.setPosition(cloudX, cloudY);
                window.draw(cloudSprite);

                // Draw cloud hitbox when it's a platform (debug)
                if (cloudIsPlatform)
                {
                    RectangleShape cloudBox;
                    cloudBox.setSize(Vector2f(cloudWidth, 10));
                    cloudBox.setPosition(cloudX, cloudY);
                    cloudBox.setFillColor(Color::Transparent);
                    cloudBox.setOutlineColor(Color::Cyan);
                    cloudBox.setOutlineThickness(1);
                    window.draw(cloudBox);
                }

               
                if (potActive && !potDestroyed)
                {
                    potSprite.setPosition(potX, potY);
                    window.draw(potSprite);

                    // drawing Pot Health Bar
                    RectangleShape potHealthBarBG;
                    potHealthBarBG.setSize(Vector2f(100, 15));
                    potHealthBarBG.setPosition(potX + potWidth / 2 - 50, potY - 25);
                    potHealthBarBG.setFillColor(Color(50, 50, 50));
                    window.draw(potHealthBarBG);

                    RectangleShape potHealthBar;
                    float potHealthPercent = (float)potHealth / potMaxHealth;
                    potHealthBar.setSize(Vector2f(100 * potHealthPercent, 15));
                    potHealthBar.setPosition(potX + potWidth / 2 - 50, potY - 25);
                    potHealthBar.setFillColor(Color(255, 165, 0)); // for orange
                    window.draw(potHealthBar);

                    // draw pot hitbox 
                    RectangleShape potBox;
                    potBox.setSize(Vector2f(potWidth, potHeight));
                    potBox.setPosition(potX, potY);
                    potBox.setFillColor(Color::Transparent);
                    potBox.setOutlineColor(Color::Yellow);
                    potBox.setOutlineThickness(2);
                    window.draw(potBox);
                }

               
                if (potDestroyed && !bossAppeared)
                {
                    
                    if ((int)(potDestroyTimer * 10) % 2 == 0)
                    {
                        Text destroyText("POT DESTROYED!", font, 50);
                        destroyText.setFillColor(Color::Red);
                        destroyText.setPosition(screen_x / 2 - 200, screen_y / 2 - 100);
                        window.draw(destroyText);
                    }

                    Text bossComingText("BOSS INCOMING...", font, 40);
                    bossComingText.setFillColor(Color::Yellow);
                    bossComingText.setPosition(screen_x / 2 - 180, screen_y / 2);
                    window.draw(bossComingText);
                }

                
                for (int pe = 0; pe < potEnemyCount; pe++)
                {
                    //  Ghost
                    Texture *currentWalk = ghostWalkTex;
                    Texture *currentSuck = ghostSuckTex;
                    Sprite *s = &EnemySprite;
                    int hW = EnemyWidth;
                    int hH = EnemyHeight;

                    
                    if (potEnemyType[pe] == 2) // Skeleton
                    {
                        currentWalk = skeletonWalkTex;
                        currentSuck = skeletonSuckTex;
                        s = &SkeletonSprite;
                        hW = SkeletonWidth;
                        hH = SkeletonHeight;
                    }
                    else if (potEnemyType[pe] == 3) // Invisible Man
                    {
                        currentWalk = invisibleWalkTex;
                        currentSuck = invisibleSuckTex;
                        s = &InvisibleSprite;
                        hW = InvisibleWidth;
                        hH = InvisibleHeight;
                    }
                    else if (potEnemyType[pe] == 4) // Chelnov
                    {
                        currentWalk = chelnovWalkTex;
                        currentSuck = chelnovSuckTex;
                        s = &ChelnovSprite;
                        hW = ChelnovWidth;
                        hH = ChelnovHeight;
                    }
                    

                    if (potEnemyType[pe] != 3 || potEnemyIsVisible[pe])
                    {
                        updateEnemyAnimation(*s, dt, currentWalk, currentSuck,
                                             potEnemyAnimFrame[pe], potEnemyAnimCounter[pe], 15,
                                             potEnemyDirection[pe], potEnemyIsCaught[pe],
                                             potEnemiesX[pe], potEnemiesY[pe], 2.0f,
                                             hW, hH);

                        window.draw(*s);
                    }
                }

                
                for (int pp = 0; pp < potEnemyProjCount; pp++)
                {
                    if (!potEnemyProjActive[pp])
                        continue;

                    // update Animation
                    potEnemyProjAnimCounter[pp]++;
                    if (potEnemyProjAnimCounter[pp] >= 10)
                    {
                        potEnemyProjAnimCounter[pp] = 0;
                        potEnemyProjAnimFrame[pp]++;
                        if (potEnemyProjAnimFrame[pp] >= 4)
                            potEnemyProjAnimFrame[pp] = 0;
                    }

                    // set Texture (Reuse the same texture array)
                    chelnovProjSprite.setTexture(chelnovProjTex[potEnemyProjAnimFrame[pp]]);

                    // draw
                    chelnovProjSprite.setPosition(potEnemyProjX[pp], potEnemyProjY[pp]);
                    window.draw(chelnovProjSprite);
                }

                
                if (bossAppeared && !bossDefeated)
                {
                    if (bossIsAngry)
                    {
                        
                        if (bossAngryTexture.getSize().x > 0)
                            bossSprite.setTexture(bossAngryTexture);
                    }
                    else
                    {
                        bossSprite.setTexture(bossTexture);
                    }
                    bossSprite.setPosition(bossX, bossY);
                    window.draw(bossSprite);

                    //  Boss Health Bar
                    RectangleShape healthBarBG;
                    healthBarBG.setSize(Vector2f(200, 20));
                    healthBarBG.setPosition(screen_x / 2 - 100, 20);
                    healthBarBG.setFillColor(Color(50, 50, 50));
                    window.draw(healthBarBG);

                    RectangleShape healthBar;
                    float healthPercent = (float)bossHealth / maxBossHealth;
                    healthBar.setSize(Vector2f(200 * healthPercent, 20));
                    healthBar.setPosition(screen_x / 2 - 100, 20);
                    if (bossIsAngry)
                        healthBar.setFillColor(Color::Red);
                    else
                        healthBar.setFillColor(Color::Green);
                    window.draw(healthBar);

                    //  Boss Health Text
                    Text bossHealthText("BOSS: " + to_string(bossHealth) + "/" + to_string(maxBossHealth), font, 20);
                    bossHealthText.setFillColor(Color::White);
                    bossHealthText.setPosition(screen_x / 2 - 50, 45);
                    window.draw(bossHealthText);
                }

                //  Tentacles (only when boss has appeared)
                if (bossAppeared)
                {
                    for (int t = 0; t < tentacleCount; t++)
                    {
                        if (!tentacleActive[t])
                            continue;

                        // Scale tentacle sprite
                        int texID = tentacleTexIndex[t];
                        tentacleSprite.setTexture(tentacleTexArray[texID]);

                        // scale based on this specific texture's size
                        float tentScaleX = (float)tentacleWidth[t] / tentacleTexArray[texID].getSize().x;
                        float tentScaleY = (float)tentacleHeight[t] / tentacleTexArray[texID].getSize().y;
                        
                        tentacleSprite.setScale(tentScaleX, tentScaleY);
                        tentacleSprite.setPosition(tentaclesX[t], tentaclesY[t]);
                        window.draw(tentacleSprite);

                        // tentacle hitbox
                        RectangleShape tentBox;
                        tentBox.setSize(Vector2f(tentacleWidth[t], tentacleHeight[t]));
                        tentBox.setPosition(tentaclesX[t], tentaclesY[t]);
                        tentBox.setFillColor(Color::Transparent);
                        tentBox.setOutlineColor(Color::Magenta);
                        tentBox.setOutlineThickness(1);
                        window.draw(tentBox);
                    }

                    // Minions (spawned by boss)
                    for (int m = 0; m < minionCount; m++)
                    {
                        if (minionIsCaught[m])
                            continue;

                        minionSprite.setPosition(minionsX[m], minionsY[m]);

                        // flip sprite based on direction
                        int texW = minionTexture.getSize().x;
                        int texH = minionTexture.getSize().y;
                        if (minionDirection[m] == -1)
                            minionSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
                        else
                            minionSprite.setTextureRect(IntRect(0, 0, texW, texH));

                        window.draw(minionSprite);
                    }
                }

                // angry warning when boss is angry
                if (bossAppeared && bossIsAngry && !bossDefeated)
                {
                    Text angryText("!! BOSS ANGRY - MINIONS FOLLOWING !!", font, 30);
                    angryText.setFillColor(Color::Red);
                    angryText.setPosition(screen_x / 2 - 250, 80);
                    window.draw(angryText);
                }

                // phase instruction text
                if (!bossAppeared)
                {
                    Text phaseText("Destroy the POT to summon the BOSS!", font, 35);
                    phaseText.setFillColor(Color::Yellow);
                    phaseText.setPosition(screen_x / 2 - 280, screen_y - 80);
                    window.draw(phaseText);
                }
            }

            float Xoffset = (64 * scale - PlayerWidth) / 2.0f;
            float Yoffset = (64 * scale - PlayerHeight);

            PlayerSprite.setPosition(player_x - Xoffset, player_y - Yoffset);
            window.draw(PlayerSprite);

            //  vacuum
            handleVacuum(window, vacSprite, vacTexHorz, vacTexVert,
                         player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming,
                         enemiesX, enemiesY, enemyCount, capturedEnemies, capturedCount, MAX_CAPACITY, 1,
                         vacFlickerTimer, showVacSprite, dt, enemyIsCaught, true, vacuumPower,
                         playerScore, comboStreak, comboTimer, multiKillCount, multiKillTimer, hasRangeBoost);

            handleVacuum(window, vacSprite, vacTexHorz, vacTexVert,
                         player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming,
                         skeletonsX, skeletonsY, skeletonCount, capturedEnemies, capturedCount, MAX_CAPACITY, 2,
                         vacFlickerTimer, showVacSprite, dt, skeletonIsCaught, true, vacuumPower,
                         playerScore, comboStreak, comboTimer, multiKillCount, multiKillTimer, hasRangeBoost);

            //  box
            RectangleShape collBox;
            collBox.setSize(Vector2f(PlayerWidth, PlayerHeight));
            collBox.setPosition(player_x, player_y);
            collBox.setFillColor(Color::Transparent);
            collBox.setOutlineColor(Color::Red);
            collBox.setOutlineThickness(2);
            window.draw(collBox);

            
            // Draw ghosts (Animated)
            for (int i = 0; i < enemyCount; i++)
            {
                updateEnemyAnimation(EnemySprite, dt,
                                     ghostWalkTex, ghostSuckTex,
                                     ghostAnimFrame[i], ghostAnimCounter[i], 15,
                                     enemyDirection[i], enemyIsCaught[i],
                                     enemiesX[i], enemiesY[i], 2.0f,
                                     EnemyWidth, EnemyHeight); 

                window.draw(EnemySprite);
            }

            
            // Draw skeletons (Animated)
            for (int i = 0; i < skeletonCount; i++)
            {
                updateEnemyAnimation(SkeletonSprite, dt,
                                     skeletonWalkTex, skeletonSuckTex,
                                     skeletonAnimFrame[i], skeletonAnimCounter[i], 15,
                                     skeletonDirection[i], skeletonIsCaught[i],
                                     skeletonsX[i], skeletonsY[i], 2.0f,
                                     SkeletonWidth, SkeletonHeight); 

                window.draw(SkeletonSprite);
            }

           
            // Draw Level 2 enemies
            if (currentLevel == 2)
            {
                // Invisible Man
                for (int i = 0; i < invisibleCount; i++)
                {
                    if (invisibleIsVisible[i])
                    {
                        updateEnemyAnimation(InvisibleSprite, dt,
                                             invisibleWalkTex, invisibleSuckTex,
                                             invisibleAnimFrame[i], invisibleAnimCounter[i], 15,
                                             invisibleDirection[i], invisibleIsCaught[i],
                                             invisiblesX[i], invisiblesY[i], 2.0f,
                                             InvisibleWidth, InvisibleHeight);

                        window.draw(InvisibleSprite);
                    }
                }

                // Chelnov
                for (int i = 0; i < chelnovCount; i++)
                {
                    updateEnemyAnimation(ChelnovSprite, dt,
                                         chelnovWalkTex, chelnovSuckTex,
                                         chelnovAnimFrame[i], chelnovAnimCounter[i], 15,
                                         chelnovDirection[i], chelnovIsCaught[i],
                                         chelnovsX[i], chelnovsY[i], 2.0f,
                                         ChelnovWidth, ChelnovHeight);

                    window.draw(ChelnovSprite);
                }

                for (int i = 0; i < chelnovProjCount; i++)
                {
                    if (chelnovProjActive[i])
                    {
                        //update Animation
                        chelnovProjAnimCounter[i]++;
                        if (chelnovProjAnimCounter[i] >= 10) 
                        {
                            chelnovProjAnimCounter[i] = 0;
                            chelnovProjAnimFrame[i]++;
                            if (chelnovProjAnimFrame[i] >= 4)
                                chelnovProjAnimFrame[i] = 0;
                        }

                        // set Texture
                        chelnovProjSprite.setTexture(chelnovProjTex[chelnovProjAnimFrame[i]]);

                        // draw
                        chelnovProjSprite.setPosition(chelnovProjX[i], chelnovProjY[i]);
                        window.draw(chelnovProjSprite);
                    }
                }
            }

            // draw projectiles
            for (int p = 0; p < projectileCount; p++)
            {
                if (!projectileActive[p])
                    continue;

                projectileAnimCounter[p]++;
                if (projectileAnimCounter[p] >= projectileAnimSpeed)
                {
                    projectileAnimCounter[p] = 0;
                    projectileAnimFrame[p]++;
                    if (projectileAnimFrame[p] >= 4)
                        projectileAnimFrame[p] = 0;
                }

                int texW, texH;
                if (projectileType[p] == 1)
                {
                    projectileSprite.setTexture(ghostRollTex[projectileAnimFrame[p]], true);
                    texW = ghostRollTex[projectileAnimFrame[p]].getSize().x;
                    texH = ghostRollTex[projectileAnimFrame[p]].getSize().y;
                }
                else
                {
                    projectileSprite.setTexture(skeletonRollTex[projectileAnimFrame[p]], true);
                    texW = skeletonRollTex[projectileAnimFrame[p]].getSize().x;
                    texH = skeletonRollTex[projectileAnimFrame[p]].getSize().y;
                }

                if (projectileDirection[p] == 1)
                    projectileSprite.setTextureRect(IntRect(0, 0, texW, texH));
                else
                    projectileSprite.setTextureRect(IntRect(texW, 0, -texW, texH));

                projectileSprite.setPosition(projectilesX[p], projectilesY[p]);
                window.draw(projectileSprite);
            }

            // Draw powerups
            for (int i = 0; i < powerupCount; i++)
            {
                if (!powerupActive[i])
                    continue;

                float floatOffset = sin(powerupAnimTimer[i] * 3.0f) * 5.0f;

                switch (powerupType[i])
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

            // UI
            if (currentLevel == 3)
                capturedCount = dynamicCapturedCount;
            Text capturedDisplay("Captured: " + to_string(capturedCount) + "/" + to_string(MAX_CAPACITY), font, 30);
            capturedDisplay.setFillColor(Color::Green);
            capturedDisplay.setPosition(20, 45);
            window.draw(capturedDisplay);

            Text livesDisplay("Lives: " + to_string(playerLives), font, 40);
            livesDisplay.setFillColor(Color::Magenta);
            livesDisplay.setPosition(70, 0);
            window.draw(livesDisplay);

            scoreText.setString("Score: " + to_string(playerScore));
            window.draw(scoreText);

            if (comboStreak >= 3)
            {
                comboText.setString("COMBO x" + to_string(comboStreak) + "!");
                window.draw(comboText);
            }

            levelText.setString("LEVEL " + to_string(currentLevel));
            window.draw(levelText);

            //  Wave display for Level 2
            if (currentLevel == 2 && useWaveSpawning)
            {
                Text waveText("Wave: " + to_string(currentWave + 1) + "/" + to_string(maxWaves), font, 35);
                waveText.setFillColor(Color::Red);
                waveText.setPosition(screen_x / 2 - 100, 60);
                window.draw(waveText);

                // show countdown between waves
                if (currentWave < maxWaves && waveSpawned[currentWave] &&
                    enemyCount == 0 && skeletonCount == 0 && invisibleCount == 0 && chelnovCount == 0)
                {
                    int timeUntilNext = (int)(timeBetweenWaves - waveTimer);
                    if (timeUntilNext > 0)
                    {
                        Text nextWaveText("Next wave in: " + to_string(timeUntilNext), font, 30);
                        nextWaveText.setFillColor(Color::Yellow);
                        nextWaveText.setPosition(screen_x / 2 - 120, 100);
                        window.draw(nextWaveText);
                    }
                }
            }

            Text powerupStatus("", font, 25);
            powerupStatus.setFillColor(Color::Cyan);
            powerupStatus.setPosition(20, 80);

            string activeEffects = "";
            if (hasSpeedBoost)
                activeEffects += "SPEED ";
            if (hasRangeBoost)
                activeEffects += "RANGE ";
            if (hasPowerBoost)
                activeEffects += "POWER ";

            if (activeEffects != "")
            {
                powerupStatus.setString("Active: " + activeEffects);
                window.draw(powerupStatus);
            }

            Text controlsHint("E: Single Shot | R: Vacuum Burst | WASD: Aim Direction", font, 20);
            controlsHint.setFillColor(Color(255, 255, 255, 150));
            controlsHint.setPosition(screen_x / 2 - 250, screen_y - 30);
            window.draw(controlsHint);

            window.display();

        } // end of game loop

        lvlMusic.stop();
        lvl2Music.stop();
        lvl3Music.stop();

        for (int i = 0; i < height; i++)
        {
            delete[] lvl[i];
        }
        delete[] lvl;

        // cleanup Boss Level dynamic arrays
        if (bossLvl != NULL)
        {
            for (int i = 0; i < level3Height; i++)
            {
                delete[] bossLvl[i];
            }
            delete[] bossLvl;
            bossLvl = NULL;
        }

        // Cleanup minion arrays
        if (minionsX != NULL)
        {
            delete[] minionsX;
            minionsX = NULL;
        }
        if (minionsY != NULL)
        {
            delete[] minionsY;
            minionsY = NULL;
        }
        if (minionSpeed != NULL)
        {
            delete[] minionSpeed;
            minionSpeed = NULL;
        }
        if (minionDirection != NULL)
        {
            delete[] minionDirection;
            minionDirection = NULL;
        }
        if (minionVelocityY != NULL)
        {
            delete[] minionVelocityY;
            minionVelocityY = NULL;
        }
        if (minionOnGround != NULL)
        {
            delete[] minionOnGround;
            minionOnGround = NULL;
        }
        if (minionIsCaught != NULL)
        {
            delete[] minionIsCaught;
            minionIsCaught = NULL;
        }
        if (minionFollowingPlayer != NULL)
        {
            delete[] minionFollowingPlayer;
            minionFollowingPlayer = NULL;
        }

        // Cleanup tentacle arrays
        if (tentaclesX != NULL)
        {
            delete[] tentaclesX;
            tentaclesX = NULL;
        }
        if (tentaclesY != NULL)
        {
            delete[] tentaclesY;
            tentaclesY = NULL;
        }
        if (tentacleWidth != NULL)
        {
            delete[] tentacleWidth;
            tentacleWidth = NULL;
        }
        if (tentacleHeight != NULL)
        {
            delete[] tentacleHeight;
            tentacleHeight = NULL;
        }
        if (tentacleTimer != NULL)
        {
            delete[] tentacleTimer;
            tentacleTimer = NULL;
        }
        if (tentacleDuration != NULL)
        {
            delete[] tentacleDuration;
            tentacleDuration = NULL;
        }
        if (tentacleActive != NULL)
        {
            delete[] tentacleActive;
            tentacleActive = NULL;
        }
        
        if (tentacleTexIndex != NULL)
        {
            delete[] tentacleTexIndex;
            tentacleTexIndex = NULL;
        }

        // cleanup pot enemy arrays (Phase 2 dynamic arrays)
        if (potEnemiesX != NULL)
        {
            delete[] potEnemiesX;
            potEnemiesX = NULL;
        }
        if (potEnemiesY != NULL)
        {
            delete[] potEnemiesY;
            potEnemiesY = NULL;
        }
        if (potEnemySpeed != NULL)
        {
            delete[] potEnemySpeed;
            potEnemySpeed = NULL;
        }
        if (potEnemyDirection != NULL)
        {
            delete[] potEnemyDirection;
            potEnemyDirection = NULL;
        }
        if (potEnemyVelocityY != NULL)
        {
            delete[] potEnemyVelocityY;
            potEnemyVelocityY = NULL;
        }
        if (potEnemyVelocityX != NULL)
        {
            delete[] potEnemyVelocityX;
            potEnemyVelocityX = NULL;
        }
        if (potEnemyOnGround != NULL)
        {
            delete[] potEnemyOnGround;
            potEnemyOnGround = NULL;
        }
        if (potEnemyIsCaught != NULL)
        {
            delete[] potEnemyIsCaught;
            potEnemyIsCaught = NULL;
        }
        if (potEnemyType != NULL)
        {
            delete[] potEnemyType;
            potEnemyType = NULL;
        }
        // cleanup pot enemy behavior arrays
        if (potEnemyJumpTimer != NULL)
        {
            delete[] potEnemyJumpTimer;
            potEnemyJumpTimer = NULL;
        }
        if (potEnemyShouldJump != NULL)
        {
            delete[] potEnemyShouldJump;
            potEnemyShouldJump = NULL;
        }
        if (potEnemyStableFrames != NULL)
        {
            delete[] potEnemyStableFrames;
            potEnemyStableFrames = NULL;
        }
        if (potEnemyIsVisible != NULL)
        {
            delete[] potEnemyIsVisible;
            potEnemyIsVisible = NULL;
        }
        if (potEnemyVisibilityTimer != NULL)
        {
            delete[] potEnemyVisibilityTimer;
            potEnemyVisibilityTimer = NULL;
        }
        if (potEnemyTeleportTimer != NULL)
        {
            delete[] potEnemyTeleportTimer;
            potEnemyTeleportTimer = NULL;
        }
        if (potEnemyShootTimer != NULL)
        {
            delete[] potEnemyShootTimer;
            potEnemyShootTimer = NULL;
        }
        if (potEnemyIsShooting != NULL)
        {
            delete[] potEnemyIsShooting;
            potEnemyIsShooting = NULL;
        }

        // cleanup dynamic captured enemies array (Phase 2)
        if (dynamicCapturedEnemies != NULL)
        {
            delete[] dynamicCapturedEnemies;
            dynamicCapturedEnemies = NULL;
        }
        if (potEnemyType != NULL)
        {
            delete[] potEnemyType;
            potEnemyType = NULL;
        }

       
        if (potEnemyAnimFrame != NULL)
        {
            delete[] potEnemyAnimFrame;
            potEnemyAnimFrame = NULL;
        }
        if (potEnemyAnimCounter != NULL)
        {
            delete[] potEnemyAnimCounter;
            potEnemyAnimCounter = NULL;
        }

    } // end of playagain loop

    return 0;
}
