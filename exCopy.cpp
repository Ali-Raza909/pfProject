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
            // Draw walls '#' AND platforms '-'
            if (lvl[i][j] == '#' || lvl[i][j] == '-')
            {
                blockSprite.setPosition(j * cell_size, i * cell_size);
                window.draw(blockSprite);
            }
            // Draw LEFT slope '/' or 'l' (left part of diagonal)
            // 'l' = left/top part of diagonal tile
            // '/' = alternative notation for down-left slope
            else if (lvl[i][j] == '/' || lvl[i][j] == 'l')
            {
                slopeLeftSpr.setPosition(j * cell_size, i * cell_size);
                window.draw(slopeLeftSpr);
            }
            // Draw RIGHT slope '\' or 'r' (right part of diagonal)
            // 'r' = right/bottom part of diagonal tile
            // '\' = alternative notation for down-right slope
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
        // TYPE C: Slope tiles '/', '\', 'l', or 'r' - treat as platforms
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
        else if (enemyType == 3) capturePoints = 150; // Invisible Man
        else if (enemyType == 4) capturePoints = 200; // Chelnov
        
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

// Function to apply sliding on slopes (Level 2 feature)
// Function to apply diagonal sliding on slopes (Level 2 feature)
void applySliding(char **lvl, float &player_x, float &player_y, int PlayerHeight, int PlayerWidth, 
                  int cell_size, int height, int width, float dt, bool &onGround, float &velocityY)
{
    if (!onGround) return;
    
    int feet_row = (int)(player_y + PlayerHeight) / cell_size;
    int feet_col_left = (int)(player_x) / cell_size;
    int feet_col_right = (int)(player_x + PlayerWidth) / cell_size;
    
    char tile_left = get_tile(lvl, feet_row, feet_col_left, height, width);
    char tile_right = get_tile(lvl, feet_row, feet_col_right, height, width);
    
    float slideSpeedX = 80.0f;  // Horizontal slide speed
    float slideSpeedY = 80.0f;  // Vertical slide speed (going down)
    
    bool onSlope = false;
    int slideDirectionX = 0;  // -1 = left, 1 = right
    
    // Down-right slope (\) using 'l' and 'r' - slide right and down
    if (tile_left == 'l' || tile_right == 'l' || tile_left == 'r' || tile_right == 'r')
    {
        onSlope = true;
        slideDirectionX = 1;  // Slide right
    }
    // Down-left slope (/) using 'L' and 'R' - slide left and down
    else if (tile_left == 'L' || tile_right == 'L' || tile_left == 'R' || tile_right == 'R')
    {
        onSlope = true;
        slideDirectionX = -1;  // Slide left
    }
    
    if (onSlope)
    {
        // Move horizontally
        player_x += slideSpeedX * slideDirectionX * dt;
        
        // Move down the slope (add to Y position)
        float newY = player_y + slideSpeedY * dt;
        
        // Check if we can move down (not hitting floor)
        int new_feet_row = (int)(newY + PlayerHeight) / cell_size;
        char below_left = get_tile(lvl, new_feet_row, (int)player_x / cell_size, height, width);
        char below_right = get_tile(lvl, new_feet_row, (int)(player_x + PlayerWidth) / cell_size, height, width);
        
        // Only slide down if not hitting solid floor
        if (below_left != '#' && below_right != '#')
        {
            player_y = newY;
            
            // Keep player "attached" to slope by giving small downward velocity
            // This prevents floating
            if (velocityY < slideSpeedY)
            {
                velocityY = slideSpeedY * 0.5f;
            }
        }
    }
}

// --- LEVEL 2 RANDOMIZED SLANT PLATFORM GENERATION ---
// This function creates:
// 1. A diagonal slant using 'l' (left half) and 'r' (right half) tiles
// 2. Horizontal platforms that DON'T cut through the slant
// 
// Tile types used:
// 'l' = left part of diagonal (top-left corner filled)
// 'r' = right part of diagonal (bottom-right corner filled)  
// '-' = horizontal platform
// '#' = solid wall/floor
// ' ' = empty space

void generateLevel2Design(char **lvl, int platHeight, int platWidth)
{
    // --- 1. CLEAR THE LEVEL ---
    for (int i = 0; i < platHeight; i++)
    {
        for (int j = 0; j < platWidth; j++)
        {
            lvl[i][j] = ' ';
        }
    }

    // --- 2. CREATE SOLID BOUNDARIES ---
    // Floor (row 11)
    for (int j = 0; j < platWidth; j++)
    {
        lvl[11][j] = '#';
    }

    // Left wall (column 0)
    for (int i = 0; i < platHeight; i++)
    {
        lvl[i][0] = '#';
    }

    // Right wall (column 18 for width=20)
    for (int i = 0; i < platHeight; i++)
    {
        lvl[i][platWidth - 2] = '#';
    }

    // --- 3. GENERATE THE SLANTED PLATFORM ---
    // Random direction: 0 = down-right (\), 1 = down-left (/)
    int direction = rand() % 2;
    
    // Slant length between 5 and 10 blocks
    int slantLength = 5 + rand() % 6;
    
    int randTopRow, randTopCol;
    
    if (direction == 0) // Down-right: start from TOP-LEFT 6x6 square
    {
        // Top-left square: rows 1-6, columns 1-6
        randTopRow = 1 + rand() % 4;  // rows 1-4 (leave room for slant to go down)
        randTopCol = 1 + rand() % 4;  // columns 1-4
        
        // Make sure slant doesn't go off screen
        if (randTopRow + slantLength > 10) 
            slantLength = 10 - randTopRow;
        if (randTopCol + slantLength > platWidth - 3)
            slantLength = platWidth - 3 - randTopCol;
    }
    else // Down-left: start from TOP-RIGHT 6x6 square
    {
        // Top-right square: rows 1-6, columns (width-7) to (width-2)
        randTopRow = 1 + rand() % 4;  // rows 1-4
        randTopCol = (platWidth - 6) + rand() % 4;  // columns 12-15 for width=20
        
        // Make sure slant doesn't go off screen
        if (randTopRow + slantLength > 10)
            slantLength = 10 - randTopRow;
        if (randTopCol - slantLength < 2)
            slantLength = randTopCol - 2;
    }

    // Ensure minimum slant length
    if (slantLength < 5) slantLength = 5;
    
    // Track slant positions for platform avoidance
    bool slantOccupied[14][20] = {false};

    // Draw the slant tiles
    for (int step = 0; step < slantLength; step++)
    {
        int row = randTopRow + step;
        int col;

        if (direction == 0) // Down-right
        {
            col = randTopCol + step;
            if (row < 11 && col > 0 && col < platWidth - 2)
            {
                lvl[row][col] = 'l';
                slantOccupied[row][col] = true;
                if (row + 1 < 11)
                {
                    lvl[row + 1][col] = 'r';
                    slantOccupied[row + 1][col] = true;
                }
            }
        }
        else // Down-left
        {
            col = randTopCol - step;
            if (row < 11 && col > 0 && col < platWidth - 2)
            {
                lvl[row][col] = 'L';
                slantOccupied[row][col] = true;
                if (row + 1 < 11)
                {
                    lvl[row + 1][col] = 'R';
                    slantOccupied[row + 1][col] = true;
                }
            }
        }
    }
    
    // --- 4. ADD PLATFORM CONNECTING SLANT TOP TO WALL ---
    // Platform at the same row as slant start, going back to the wall
    if (direction == 0) // Down-right: platform goes LEFT from slant top to wall
    {
        for (int j = 1; j < randTopCol; j++)
        {
            if (lvl[randTopRow][j] == ' ')
                lvl[randTopRow][j] = '-';
        }
    }
    else // Down-left: platform goes RIGHT from slant top to wall
    {
        for (int j = randTopCol + 1; j < platWidth - 2; j++)
        {
            if (lvl[randTopRow][j] == ' ')
                lvl[randTopRow][j] = '-';
        }
    }

    // --- 5. GENERATE HORIZONTAL PLATFORMS ---
    // Platform rows with AT LEAST 3 rows gap between them (player height ~1.5 blocks)
    // Row 11 is floor, so valid platform rows: 3, 6, 9 (3 rows apart = 2 empty rows between)
    int platformRows[] = {3, 6, 9};
    int numPlatformRows = 3;
    
    // Max platform length: 4-6 blocks
    int minPlatLength = 3;
    int maxPlatLength = 5;

    for (int p = 0; p < numPlatformRows; p++)
    {
        int row = platformRows[p];
        
        // Skip if too close to slant start row
        if (abs(row - randTopRow) < 2) continue;
        
        // Check for slant tiles in this row and adjacent rows
        // Need 3 rows clear above the platform for player movement
        
        // LEFT SIDE PLATFORM (columns 1-8)
        bool leftClear = true;
        for (int checkRow = row - 2; checkRow <= row + 1; checkRow++)
        {
            if (checkRow < 0 || checkRow >= 12) continue;
            for (int j = 1; j <= 8; j++)
            {
                if (slantOccupied[checkRow][j]) leftClear = false;
            }
        }
        
        if (leftClear)
        {
            // Find a clear section for platform
            int platStart = 1 + rand() % 2;
            int platLength = minPlatLength + rand() % (maxPlatLength - minPlatLength + 1);
            
            for (int j = platStart; j < platStart + platLength && j <= 6; j++)
            {
                if (lvl[row][j] == ' ' && !slantOccupied[row][j])
                    lvl[row][j] = '-';
            }
        }
        
        // RIGHT SIDE PLATFORM (columns 10-17)
        bool rightClear = true;
        for (int checkRow = row - 2; checkRow <= row + 1; checkRow++)
        {
            if (checkRow < 0 || checkRow >= 12) continue;
            for (int j = 10; j <= platWidth - 3; j++)
            {
                if (slantOccupied[checkRow][j]) rightClear = false;
            }
        }
        
        if (rightClear)
        {
            int platStart = 11 + rand() % 2;
            int platLength = minPlatLength + rand() % (maxPlatLength - minPlatLength + 1);
            
            for (int j = platStart; j < platStart + platLength && j <= platWidth - 3; j++)
            {
                if (lvl[row][j] == ' ' && !slantOccupied[row][j])
                    lvl[row][j] = '-';
            }
        }
    }
    
    // --- 6. ADD CONNECTING/MIDDLE PLATFORMS FOR NAVIGATION ---
    // Add a middle platform to help player traverse between sides
    // Place it opposite to where the slant is
    int middleRow = 5;
    if (direction == 0) // Slant on left side, add platform on right-center
    {
        for (int j = 8; j <= 11; j++)
        {
            if (lvl[middleRow][j] == ' ' && !slantOccupied[middleRow][j])
                lvl[middleRow][j] = '-';
        }
    }
    else // Slant on right side, add platform on left-center
    {
        for (int j = 6; j <= 9; j++)
        {
            if (lvl[middleRow][j] == ' ' && !slantOccupied[middleRow][j])
                lvl[middleRow][j] = '-';
        }
    }
    
    // --- 7. ENSURE NO PLATFORMS ON ROW 10 (too close to ground) ---
    // Row 10 would create only 1 block gap to floor at row 11
    for (int j = 0; j < platWidth; j++)
    {
        if (lvl[10][j] == '-')
            lvl[10][j] = ' ';
    }

    // --- 8. DEBUG OUTPUT ---
    cout << "=== Level 2 Design Generated ===" << endl;
    cout << "Slant direction: " << (direction == 0 ? "Down-Right (\\)" : "Down-Left (/)") << endl;
    cout << "Slant start: row " << randTopRow << ", col " << randTopCol << endl;
    cout << "Slant length: " << slantLength << " tiles" << endl;
    cout << "Platform connecting to wall at row " << randTopRow << endl;
}




// NEW FUNCTION: Spawn enemies in waves for Level 2

void spawnWave(int waveNumber, int cell_size,
               float* enemiesX, float* enemiesY, float* enemySpeed, int* enemyDirection,
               float* platformLeftEdge, float* platformRightEdge, int& enemyCount,
               float* skeletonsX, float* skeletonsY, float* skeletonSpeed, int* skeletonDirection,
               float* skeletonVelocityY, bool* skeletonOnGround, float* skeletonJumpTimer,
               float* skeletonJumpCooldown, bool* skeletonShouldJump, int* skeletonStableFrames,
               int* skeletonAnimFrame, int* skeletonAnimCounter, int& skeletonCount,
               float* invisiblesX, float* invisiblesY, float* invisibleSpeed, int* invisibleDirection,
               float* invisibleVelocityY, bool* invisibleOnGround, bool* invisibleIsVisible,
               float* invisibleVisibilityTimer, float* invisibleTeleportTimer, int& invisibleCount,
               float* chelnovsX, float* chelnovsY, float* chelnovSpeed, int* chelnovDirection,
               float* chelnovVelocityY, bool* chelnovOnGround, float* chelnovShootTimer,
               bool* chelnovIsShooting, float* chelnovShootPhaseTimer, int& chelnovCount,
               int maxEnemyCount, int maxSkeletonCount, int maxInvisibleCount, int maxChelnovCount)
{
    cout << "Spawning Wave " << (waveNumber + 1) << endl;
    
    switch(waveNumber)
    {
        case 0: // Wave 1 - 2 Ghosts + 3 Skeletons
        {
            // Spawn 2 Ghosts
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
            
            // Spawn 3 Skeletons
            float skelSpawnX[] = {(float)(5*cell_size), (float)(13*cell_size), (float)(9*cell_size)};
            float skelSpawnY[] = {(float)(0*cell_size), (float)(0*cell_size), (float)(2*cell_size)};
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
        
        case 1: // Wave 2 - 2 Ghosts + 3 Skeletons
        {
            // Spawn 2 Ghosts
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
            
            // Spawn 3 Skeletons
            float skelSpawnX[] = {(float)(5*cell_size), (float)(13*cell_size), (float)(3*cell_size)};
            float skelSpawnY[] = {(float)(2*cell_size), (float)(2*cell_size), (float)(4*cell_size)};
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
        
        case 2: // Wave 3 - 3 Skeletons + 2 Chelnovs + 2 Invisible Men
        {
            // Spawn 3 Skeletons
            float skelSpawnX[] = {(float)(15*cell_size), (float)(4*cell_size), (float)(14*cell_size)};
            float skelSpawnY[] = {(float)(4*cell_size), (float)(6*cell_size), (float)(6*cell_size)};
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
            
            // Spawn 2 Chelnovs
            float chelSpawnX[] = {(float)(12*cell_size), (float)(6*cell_size)};
            float chelSpawnY[] = {(float)(0*cell_size), (float)(2*cell_size)};
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
            
            // Spawn 2 Invisible Men
            float invisSpawnX[] = {(float)(6*cell_size), (float)(13*cell_size)};
            float invisSpawnY[] = {(float)(0*cell_size), (float)(4*cell_size)};
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
        
        case 3: // Wave 4 - BOSS WAVE: 2 Chelnovs + 1 Invisible Man
        {
            // Spawn 2 Chelnovs
            float chelSpawnX[] = {(float)(14*cell_size), (float)(3*cell_size)};
            float chelSpawnY[] = {(float)(4*cell_size), (float)(8*cell_size)};
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
            
            // Spawn 1 Invisible Man
            if (invisibleCount < maxInvisibleCount)
            {
                invisiblesX[invisibleCount] = (float)(5*cell_size);
                invisiblesY[invisibleCount] = (float)(4*cell_size);
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

// MODIFIED FUNCTION: Generate Level 2 map with optional enemy spawning
void generateLevel2Map(char** lvl, int height, int width, int cell_size,
                       float* enemiesX, float* enemiesY, float* enemySpeed, int* enemyDirection,
                       float* platformLeftEdge, float* platformRightEdge, int& enemyCount,
                       float* skeletonsX, float* skeletonsY, float* skeletonSpeed, int* skeletonDirection,
                       float* skeletonVelocityY, bool* skeletonOnGround, float* skeletonJumpTimer,
                       float* skeletonJumpCooldown, bool* skeletonShouldJump, int* skeletonStableFrames,
                       int* skeletonAnimFrame, int* skeletonAnimCounter, int& skeletonCount,
                       float* invisiblesX, float* invisiblesY, float* invisibleSpeed, int* invisibleDirection,
                       float* invisibleVelocityY, bool* invisibleOnGround, bool* invisibleIsVisible,
                       float* invisibleVisibilityTimer, float* invisibleTeleportTimer, int& invisibleCount,
                       float* chelnovsX, float* chelnovsY, float* chelnovSpeed, int* chelnovDirection,
                       float* chelnovVelocityY, bool* chelnovOnGround, float* chelnovShootTimer,
                       bool* chelnovIsShooting, float* chelnovShootPhaseTimer, int& chelnovCount,
                       bool spawnAllEnemies = true) // NEW PARAMETER with default value
{
    // Reset enemy counts
    enemyCount = 0;
    skeletonCount = 0;
    invisibleCount = 0;
    chelnovCount = 0;
    
    // --- USE THE RANDOMIZED LEVEL DESIGN FUNCTION ---
    // This generates walls, floor, slanted platform, and horizontal platforms
    generateLevel2Design(lvl, height, width);
    
    // MODIFIED: Only spawn enemies if requested
    if (spawnAllEnemies)
    {
        // Spawn 4 ghosts - using explicit float casts
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
        
        // Spawn 9 skeletons - using explicit float casts
        float skelSpawnX[] = {(float)(5*cell_size), (float)(13*cell_size), (float)(5*cell_size), (float)(13*cell_size), (float)(3*cell_size),
                              (float)(15*cell_size), (float)(4*cell_size), (float)(14*cell_size), (float)(9*cell_size)};
        float skelSpawnY[] = {(float)(0*cell_size), (float)(0*cell_size), (float)(2*cell_size), (float)(2*cell_size), (float)(4*cell_size),
                              (float)(4*cell_size), (float)(6*cell_size), (float)(6*cell_size), (float)(8*cell_size)};
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
        
        // Spawn 3 invisible men - using explicit float casts
        float invisSpawnX[] = {(float)(6*cell_size), (float)(5*cell_size), (float)(13*cell_size)};
        float invisSpawnY[] = {(float)(0*cell_size), (float)(4*cell_size), (float)(6*cell_size)};
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
        
        // Spawn 4 chelnovs - using explicit float casts
        float chelSpawnX[] = {(float)(12*cell_size), (float)(6*cell_size), (float)(14*cell_size), (float)(3*cell_size)};
        float chelSpawnY[] = {(float)(0*cell_size), (float)(2*cell_size), (float)(4*cell_size), (float)(8*cell_size)};
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
       
       // Level tracking variables
       int currentLevel = 1;
       bool showStageClear = false;
       
       // NEW: Wave spawning system variables
       bool useWaveSpawning = false;
       int currentWave = 0;
       int maxWaves = 4;
       float waveTimer = 0.0f;
       float timeBetweenWaves = 5.0f; // 5 seconds between waves
       bool waveSpawned[4] = {false, false, false, false};
       
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
        
        // Stage Clear screen text
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
        
        // --- LEVEL CLEAR CELEBRATION IMAGE ---
        // Single player image displayed on stage clear screen
        Texture levelClearTex;
        Sprite levelClearSprite;
        levelClearTex.loadFromFile("Data/levelClear.png");
        levelClearSprite.setTexture(levelClearTex);
        levelClearSprite.setScale(3.0f, 3.0f); // Make it big and visible
        // Position will be set when displayed
        
        // Level indicator text
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

        Text subtitle("Press 1 for Yellow (Fast) ", font, 50);
        subtitle.setFillColor(Color::Yellow);
        subtitle.setPosition(120, 400);

        Text subtitle2(" Press 2 for Green (Strong Vacuum)", font, 50);
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
        
        Texture blockTexture2;
Sprite blockSprite2;
Texture bgTex2;
Sprite bgSprite2;

         bgTex.loadFromFile("Data/bg.png");
bgSprite.setTexture(bgTex);
bgSprite.setScale(2.0f, 2.0f);

blockTexture.loadFromFile("Data/block1.png");
blockSprite.setTexture(blockTexture);

          // LEVEL 2 ASSETS

   if (!bgTex2.loadFromFile("Data/bg2.png"))
{
    cout << "ERROR: bg2.png failed to load! Using bg.png as fallback.\n";
    bgTex2.loadFromFile("Data/bg.png"); // Fallback to level 1 background
}

bgSprite2.setTexture(bgTex2);
bgSprite2.setScale(
    float(screen_x) / bgTex2.getSize().x,
    float(screen_y) / bgTex2.getSize().y
);

// BLOCKS FOR LEVEL 2
blockTexture2.loadFromFile("Data/block2.png");   // new block
blockSprite2.setTexture(blockTexture2);


        // Add after loading block textures (around line 920)
Texture slopeLeftTexture, slopeRightTexture;
Sprite slopeLeftSprite, slopeRightSprite;

if (!slopeLeftTexture.loadFromFile("Data/blocks/sloperight.png"))
    cout << "slope_left.png missing!\n";
if (!slopeRightTexture.loadFromFile("Data/blocks/slopeleft.png"))
    cout << "slope_right.png missing!\n";

slopeLeftSprite.setTexture(slopeLeftTexture);
slopeRightSprite.setTexture(slopeRightTexture);

// for down left slope load new sprites

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
        
        // Level 2 music
        Music lvl2Music;
        if (!lvl2Music.openFromFile("Data/mus2.ogg"))
        {
            lvl2Music.openFromFile("Data/mus.ogg");
        }
        lvl2Music.setVolume(20);
        lvl2Music.setLoop(true);

        float player_x = 850.0f;
        float player_y = 450.f;

        float speed = 140.0f * speedMultiplier;

        const float jumpStrength = -150.0f;
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

        Texture EnemyTexture;
        Sprite EnemySprite;

        EnemyTexture.loadFromFile("Data/ghost.png");
        EnemySprite.setTexture(EnemyTexture);
        EnemySprite.setScale(2, 2);
        
        // --- GHOST WALKING ANIMATION ---
        Texture ghostWalkTex[4];
        ghostWalkTex[0].loadFromFile("Data/ghostWalk/walk1.png");
        ghostWalkTex[1].loadFromFile("Data/ghostWalk/walk2.png");
        ghostWalkTex[2].loadFromFile("Data/ghostWalk/walk3.png");
        ghostWalkTex[3].loadFromFile("Data/ghostWalk/walk4.png");
        
        int ghostAnimFrame[maxEnemyCount];
        int ghostAnimCounter[maxEnemyCount];
        int ghostAnimSpeed = 8;
        
        for (int i = 0; i < maxEnemyCount; i++)
        {
            ghostAnimFrame[i] = 0;
            ghostAnimCounter[i] = 0;
        }
        
        // --- GHOST SUCKING ANIMATION ---
        Texture ghostSuckTex[4];
        ghostSuckTex[0].loadFromFile("Data/ghostSuck/suck1.png");
        ghostSuckTex[1].loadFromFile("Data/ghostSuck/suck2.png");
        ghostSuckTex[2].loadFromFile("Data/ghostSuck/suck3.png");
        ghostSuckTex[3].loadFromFile("Data/ghostSuck/suck4.png");
        
        int ghostSuckFrame[maxEnemyCount];
        int ghostSuckCounter[maxEnemyCount];
        int ghostSuckSpeed = 5;
        
        for (int i = 0; i < maxEnemyCount; i++)
        {
            ghostSuckFrame[i] = 0;
            ghostSuckCounter[i] = 0;
        }

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

        int SkeletonHeight = 92;
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
        
        // --- SKELETON SUCKING ANIMATION ---
        Texture skeletonSuckTex[4];
        skeletonSuckTex[0].loadFromFile("Data/skeletonSuck/suck1.png");
        skeletonSuckTex[1].loadFromFile("Data/skeletonSuck/suck2.png");
        skeletonSuckTex[2].loadFromFile("Data/skeletonSuck/suck3.png");
        skeletonSuckTex[3].loadFromFile("Data/skeletonSuck/suck4.png");
        
        int skeletonSuckFrame[maxSkeletonCount];
        int skeletonSuckCounter[maxSkeletonCount];
        int skeletonSuckSpeed = 5;
        
        for (int i = 0; i < maxSkeletonCount; i++)
        {
            skeletonSuckFrame[i] = 0;
            skeletonSuckCounter[i] = 0;
        }
        
        // Invisible Man enemies (Level 2 only)
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
        
        int InvisibleHeight = 80;
        int InvisibleWidth = 60;
        
        Texture InvisibleTexture;
        Sprite InvisibleSprite;
        
        if (!InvisibleTexture.loadFromFile("Data/invisibleMan/walk1.png"))
        {
            InvisibleTexture.loadFromFile("Data/ghost.png");
        }
        InvisibleSprite.setTexture(InvisibleTexture);
        InvisibleSprite.setScale(2, 2);
        
        // --- INVISIBLE MAN WALKING ANIMATION ---
        Texture invisibleWalkTex[4];
        invisibleWalkTex[0].loadFromFile("Data/invisibleMan/walk1.png");
        invisibleWalkTex[1].loadFromFile("Data/invisibleMan/walk2.png");
        invisibleWalkTex[2].loadFromFile("Data/invisibleMan/walk3.png");
        invisibleWalkTex[3].loadFromFile("Data/invisibleMan/walk4.png");
        
        int invisibleAnimFrame[maxInvisibleCount];
        int invisibleAnimCounter[maxInvisibleCount];
        int invisibleAnimSpeed = 8;
        
        // --- INVISIBLE MAN SUCKING ANIMATION ---
        Texture invisibleSuckTex[4];
        invisibleSuckTex[0].loadFromFile("Data/invisibleMan/suck1.png");
        invisibleSuckTex[1].loadFromFile("Data/invisibleMan/suck2.png");
        invisibleSuckTex[2].loadFromFile("Data/invisibleMan/suck3.png");
        invisibleSuckTex[3].loadFromFile("Data/invisibleMan/suck4.png");
        
        int invisibleSuckFrame[maxInvisibleCount];
        int invisibleSuckCounter[maxInvisibleCount];
        int invisibleSuckSpeed = 5;
        
        for (int i = 0; i < maxInvisibleCount; i++)
        {
            invisibleAnimFrame[i] = 0;
            invisibleAnimCounter[i] = 0;
            invisibleSuckFrame[i] = 0;
            invisibleSuckCounter[i] = 0;
        }
        
        // Chelnov enemies (Level 2 only)
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
        
        int ChelnovHeight = 90;
        int ChelnovWidth = 60;
        
        Texture ChelnovTexture;
        Sprite ChelnovSprite;
        
        if (!ChelnovTexture.loadFromFile("Data/chelnov/walk1.png"))
        {
            ChelnovTexture.loadFromFile("Data/skeleton.png");
        }
        ChelnovSprite.setTexture(ChelnovTexture);
        ChelnovSprite.setScale(2, 2);
        
        // --- CHELNOV WALKING ANIMATION ---
        Texture chelnovWalkTex[4];
        chelnovWalkTex[0].loadFromFile("Data/chelnov/walk1.png");
        chelnovWalkTex[1].loadFromFile("Data/chelnov/walk2.png");
        chelnovWalkTex[2].loadFromFile("Data/chelnov/walk3.png");
        chelnovWalkTex[3].loadFromFile("Data/chelnov/walk4.png");
        
        int chelnovAnimFrame[maxChelnovCount];
        int chelnovAnimCounter[maxChelnovCount];
        int chelnovAnimSpeed = 8;
        
        // --- CHELNOV SUCKING ANIMATION ---
        Texture chelnovSuckTex[4];
        chelnovSuckTex[0].loadFromFile("Data/chelnov/suck1.png");
        chelnovSuckTex[1].loadFromFile("Data/chelnov/suck2.png");
        chelnovSuckTex[2].loadFromFile("Data/chelnov/suck3.png");
        chelnovSuckTex[3].loadFromFile("Data/chelnov/suck4.png");
        
        int chelnovSuckFrame[maxChelnovCount];
        int chelnovSuckCounter[maxChelnovCount];
        int chelnovSuckSpeed = 5;
        
        for (int i = 0; i < maxChelnovCount; i++)
        {
            chelnovAnimFrame[i] = 0;
            chelnovAnimCounter[i] = 0;
            chelnovSuckFrame[i] = 0;
            chelnovSuckCounter[i] = 0;
        }
        
        // Chelnov projectiles
        const int maxChelnovProjectiles = 10;
        float chelnovProjX[maxChelnovProjectiles];
        float chelnovProjY[maxChelnovProjectiles];
        int chelnovProjDirection[maxChelnovProjectiles];
        bool chelnovProjActive[maxChelnovProjectiles];
        int chelnovProjCount = 0;
        
        for (int i = 0; i < maxChelnovProjectiles; i++)
            chelnovProjActive[i] = false;
        
        Texture chelnovProjTexture;
        Sprite chelnovProjSprite;
        
        if (!chelnovProjTexture.loadFromFile("Data/fireball.png"))
        {
            chelnovProjTexture.loadFromFile("Data/ghost.png");
        }
        chelnovProjSprite.setTexture(chelnovProjTexture);
        chelnovProjSprite.setScale(1.5f, 1.5f);

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
        
        // --- POWERUP SYSTEM ---
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

        int MAX_CAPACITY = 3;
        const int MAX_CAPTURED_ARRAY = 5;
        int capturedEnemies[MAX_CAPTURED_ARRAY]; 
        int capturedCount = 0;

        // --- PROJECTILE SYSTEM ---
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
        int projectileAnimSpeed = 5;
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
        
        // --- BURST MODE SYSTEM ---
        bool burstModeActive = false;
        int burstFrameCounter = 0;
        const int BURST_FRAME_DELAY = 10;
        int burstReleaseDirection = 0;
        int burstPlayerFacing = 0;
        
        int bottomFloorRow = 11;
        float bottomFloorLeftEdge = 1 * cell_size;
        float bottomFloorRightEdge = 17 * cell_size + cell_size;

        // --- LEVEL CREATION ---
        lvl = new char *[height];
        for (int i = 0; i < height; i += 1)
        {
            lvl[i] = new char[width];
            for (int j = 0; j < width; j++)
                lvl[i][j] = ' ';
        }

        // Level 1 layout (your original)
        lvl[1][3] = '-'; lvl[1][4] = '-'; lvl[1][5] = '-'; lvl[1][6] = '-';
        lvl[1][7] = '-'; lvl[1][8] = '-'; lvl[1][9] = '-'; lvl[1][10] = '-';
        lvl[1][11] = '-'; lvl[1][12] = '-'; lvl[1][13] = '-'; lvl[1][14] = '-';

        lvl[9][3] = '-'; lvl[9][4] = '-'; lvl[9][5] = '-'; lvl[9][6] = '-';
        lvl[9][7] = '-'; lvl[9][8] = '-'; lvl[9][9] = '-'; lvl[9][10] = '-';
        lvl[9][11] = '-'; lvl[9][12] = '-'; lvl[9][13] = '-'; lvl[9][14] = '-';

        lvl[8][8] = '-'; lvl[8][9] = '-';
        lvl[7][1] = '-'; lvl[7][2] = '-'; lvl[7][3] = '-';
        lvl[7][9] = '-'; lvl[7][8] = '-'; lvl[7][7] = '-'; lvl[7][10] = '-';
        lvl[7][14] = '-'; lvl[7][15] = '-'; lvl[7][16] = '-';
        lvl[6][7] = '-'; lvl[6][10] = '-';
        lvl[5][7] = '-'; lvl[5][10] = '-';
        lvl[5][3] = '-'; lvl[5][4] = '-'; lvl[5][5] = '-'; lvl[5][6] = '-';
        lvl[5][11] = '-'; lvl[5][12] = '-'; lvl[5][13] = '-'; lvl[5][14] = '-';
        lvl[4][7] = '-'; lvl[4][10] = '-';
        lvl[3][7] = '-'; lvl[3][10] = '-'; lvl[3][8] = '-'; lvl[3][9] = '-';
        lvl[3][1] = '-'; lvl[3][2] = '-'; lvl[3][3] = '-';
        lvl[3][16] = '-'; lvl[3][15] = '-'; lvl[3][14] = '-';
        lvl[2][8] = '-'; lvl[2][9] = '-';

        // Floor and Sides
        for (int j = 0; j <= 18; j++) lvl[11][j] = '#';
        for (int i = 0; i <= 10; i++) { lvl[i][0] = '#'; lvl[i][18] = '#'; }
        lvl[7][17] = '#'; lvl[3][17] = '#';
        lvl[2][8] = '#'; lvl[2][9] = '#';
        lvl[8][8] = '#'; lvl[8][9] = '#';
        lvl[7][7] = '#'; lvl[7][8] = '#'; lvl[7][9] = '#'; lvl[7][10] = '#';
        lvl[6][7] = '#'; lvl[6][10] = '#';
        lvl[5][7] = '#'; lvl[5][10] = '#';
        lvl[4][7] = '#'; lvl[4][10] = '#';
        lvl[3][7] = '#'; lvl[3][8] = '#'; lvl[3][9] = '#'; lvl[3][10] = '#';

        // Enemy spawn markers
        lvl[0][5] = 'e'; lvl[0][12] = 'e'; lvl[2][2] = 'e'; lvl[2][15] = 'e';
        lvl[4][4] = 'e'; lvl[4][13] = 'e'; lvl[8][6] = 'e'; lvl[8][10] = 'e';
        lvl[0][7] = 's'; lvl[0][10] = 's'; lvl[2][4] = 's'; lvl[2][13] = 's';

        // Initialize ghosts
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
        cout << "Total skeletons: " << skeletonCount << endl;

        // Spawn initial powerups
        for (int i = 0; i < 3; i++)
        {
            spawnPowerup(powerupsX, powerupsY, powerupType, powerupActive, 
                         powerupAnimTimer, powerupCount, maxPowerups, 
                         lvl, width, height, cell_size);
        }

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

            // NEW: Wave spawning system for Level 2
            if (currentLevel == 2 && useWaveSpawning)
            {
                waveTimer += dt;
                
                // Check if current wave should spawn
                if (currentWave < maxWaves && !waveSpawned[currentWave])
                {
                    // Spawn first wave immediately, others after delay
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
                
                // Check if current wave is cleared (all enemies defeated)
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

            if (comboTimer >= comboTimeout)
                comboStreak = 0;

            if (multiKillTimer >= multiKillWindow && multiKillCount > 0)
                checkMultiKill(multiKillCount, multiKillTimer, playerScore);

            // Update powerup timers
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
            
            // Reset safety flags
            for (int i = 0; i < maxEnemyCount; i++) enemyIsCaught[i] = false;
            for (int i = 0; i < maxSkeletonCount; i++) skeletonIsCaught[i] = false;
            for (int i = 0; i < maxInvisibleCount; i++) invisibleIsCaught[i] = false;
            for (int i = 0; i < maxChelnovCount; i++) chelnovIsCaught[i] = false;

            // Vacuum input
            isVacuuming = false;
            if (Keyboard::isKeyPressed(Keyboard::Space))
            {
                isVacuuming = true;
                if (Keyboard::isKeyPressed(Keyboard::D)) { vacDirection = 0; facing = 1; }
                else if (Keyboard::isKeyPressed(Keyboard::A)) { vacDirection = 1; facing = 0; }
                else if (Keyboard::isKeyPressed(Keyboard::W)) vacDirection = 2;
                else if (Keyboard::isKeyPressed(Keyboard::S)) vacDirection = 3;
                else vacDirection = facing;
            }
            else showVacSprite = true;

            // Release direction
            if (Keyboard::isKeyPressed(Keyboard::D)) releaseDirection = 0;
            else if (Keyboard::isKeyPressed(Keyboard::A)) releaseDirection = 1;
            else if (Keyboard::isKeyPressed(Keyboard::W)) releaseDirection = 2;
            else if (Keyboard::isKeyPressed(Keyboard::S)) releaseDirection = 3;
            else releaseDirection = facing;
            
            // Single Shot - E key
            static bool eKeyPressed = false;
            if (Keyboard::isKeyPressed(Keyboard::E) && !eKeyPressed)
            {
                eKeyPressed = true;
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
                       projectileVelocityY[projectileCount] = 0; 
                       }
                    else if (releaseDirection == 1) { projectileDirection[projectileCount] = -1; projectileVelocityY[projectileCount] = 0; }
                    else if (releaseDirection == 2) { projectileDirection[projectileCount] = (facing == 1) ? 1 : -1; projectileVelocityY[projectileCount] = -200.0f; }
                    else if (releaseDirection == 3) { projectileDirection[projectileCount] = (facing == 1) ? 1 : -1; projectileVelocityY[projectileCount] = 200.0f; }
                    
                    projectileCount++;
                }
            }
            if (!Keyboard::isKeyPressed(Keyboard::E)) eKeyPressed = false;
            
            // Vacuum Burst - R key
            static bool rKeyPressed = false;
            if (Keyboard::isKeyPressed(Keyboard::R) && !rKeyPressed)
            {
                rKeyPressed = true;
                if (capturedCount > 0 && !burstModeActive)
                {
                    if (capturedCount >= 3) playerScore += 300;
                    burstModeActive = true;
                    burstFrameCounter = 0;
                    burstReleaseDirection = releaseDirection;
                    burstPlayerFacing = facing;
                }
            }
            if (!Keyboard::isKeyPressed(Keyboard::R)) rKeyPressed = false;
            
            // Burst mode release
            if (burstModeActive)
            {
                burstFrameCounter++;
                if (burstFrameCounter >= BURST_FRAME_DELAY)
                {
                    burstFrameCounter = 0;
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
                        
                        if (burstReleaseDirection == 0) { projectileDirection[projectileCount] = 1; projectileVelocityY[projectileCount] = 0; }
                        else if (burstReleaseDirection == 1) { projectileDirection[projectileCount] = -1; projectileVelocityY[projectileCount] = 0; }
                        else if (burstReleaseDirection == 2) { projectileDirection[projectileCount] = (burstPlayerFacing == 1) ? 1 : -1; projectileVelocityY[projectileCount] = -200.0f; }
                        else if (burstReleaseDirection == 3) { projectileDirection[projectileCount] = (burstPlayerFacing == 1) ? 1 : -1; projectileVelocityY[projectileCount] = 200.0f; }
                        
                        projectileCount++;
                    }
                    if (capturedCount <= 0) burstModeActive = false;
                }
            }

            // Vacuum logic
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
     
            // Level 2 vacuum handling
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
                  
            // Powerup collection
            for (int i = 0; i < powerupCount; i++)
            {
                if (!powerupActive[i]) continue;
                
                if ((player_x < powerupsX[i] + PowerupWidth) && 
                    (player_x + PlayerWidth > powerupsX[i]) && 
                    (player_y < powerupsY[i] + PowerupHeight) && 
                    (player_y + PlayerHeight > powerupsY[i]))
                {
                    powerupActive[i] = false;
                    
                    switch(powerupType[i])
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
        playerCollision_x(lvl, player_x, player_y, speed, cell_size, PlayerHeight,
                          PlayerWidth, height, width, dt, isMoving, facing);
        updatePlayerAnimation(PlayerSprite, facing, isMoving, isDead, onGround, idleTex,
                              walkTex, jumpTex, deadTex, animFrame, deadAnimFrame, animCounter, deadAnimCounter, animSpeed, deadAnimSpeed);

        if (Keyboard::isKeyPressed(Keyboard::Up) && onGround)
        {
            velocityY = jumpStrength;
            onGround = false;
            isJumping = true;
        }

        player_gravity(lvl, offset_y, velocityY, onGround, gravity, terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth, height, width, dt);
        
        // Apply sliding on slopes (Level 2)
        // Apply sliding on slopes (Level 2)
if (currentLevel == 2)
{
    applySliding(lvl, player_x, player_y, PlayerHeight, PlayerWidth, cell_size, height, width, dt, onGround, velocityY);
}

        // Ghost enemy loop
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

        // Skeleton enemy loop
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
                        if (platformAbove) break;
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
        
        // Level 2 enemy loops
        if (currentLevel == 2)
        {
            // Invisible Man loop
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
            
            // Chelnov loop
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
            
            // Chelnov projectile loop
            for (int i = 0; i < chelnovProjCount; i++)
            {
                if (!chelnovProjActive[i]) continue;
                
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

        // Projectile update loop
        for (int p = 0; p < projectileCount; p++)
        {
            if (!projectileActive[p])
                 continue;
                 
            projectileLifespan[p] += dt;
if (projectileLifespan[p] >= MAX_PROJECTILE_LIFE)
{
    // Remove old projectile
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

            // Update animation frame
            projectileAnimCounter[p]++;
if (projectileAnimCounter[p] >= projectileAnimSpeed)
{
projectileAnimCounter[p] = 0;
projectileAnimFrame[p]++;
if (projectileAnimFrame[p] >= 4)
projectileAnimFrame[p] = 0;
}

float newProjX = projectilesX[p] + projectileSpeed * projectileDirection[p] * dt;
            
            int projRow = (int)(projectilesY[p] + ProjectileHeight / 2) / cell_size;
            int projColRight = (int)(newProjX + ProjectileWidth) / cell_size;
            int projColLeft = (int)newProjX / cell_size;
            
            char rightWall = get_tile(lvl, projRow, projColRight, height, width);
            char leftWall = get_tile(lvl, projRow, projColLeft, height, width);
            
            if (projectileDirection[p] == 1 && rightWall == '#')
                projectileDirection[p] = -1;
            else if (projectileDirection[p] == -1 && leftWall == '#')
                projectileDirection[p] = 1;
            else
                projectilesX[p] = newProjX;
            
            float newProjY = projectilesY[p] + projectileVelocityY[p] * dt;
            
            int feetRow = (int)(newProjY + ProjectileHeight) / cell_size;
            int feetColL = (int)projectilesX[p] / cell_size;
            int feetColR = (int)(projectilesX[p] + ProjectileWidth) / cell_size;
            
            char floorL = get_tile(lvl, feetRow, feetColL, height, width);
            char floorR = get_tile(lvl, feetRow, feetColR, height, width);
            
            int headRow = (int)newProjY / cell_size;
            char ceilL = get_tile(lvl, headRow, feetColL, height, width);
            char ceilR = get_tile(lvl, headRow, feetColR, height, width);
            
            bool landed = false;
            
            if (projectileVelocityY[p] < 0)
            {
                if (ceilL == '#' || ceilR == '#')
                {
                    projectileVelocityY[p] = 0;
                    projectilesY[p] = (headRow + 1) * cell_size;
                }
            }
            
            if (projectileVelocityY[p] >= 0)
            {
                if (floorL == '#' || floorR == '#')
                    landed = true;
                else if (floorL == '-' || floorR == '-')
                {
                    float blockTop = feetRow * cell_size;
                    if ((projectilesY[p] + ProjectileHeight <= blockTop + 4.0f) && (newProjY + ProjectileHeight >= blockTop))
                        landed = true;
                }
            }
            
            if (landed)
            {
                projectileOnGround[p] = true;
                projectileVelocityY[p] = 0;
                projectilesY[p] = (feetRow * cell_size) - ProjectileHeight;
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
            
            // Collision with ghosts
            for (int e = 0; e < enemyCount; e++)
            {
                if ((projectilesX[p] < enemiesX[e] + EnemyWidth) &&
                    (projectilesX[p] + ProjectileWidth > enemiesX[e]) &&
                    (projectilesY[p] < enemiesY[e] + EnemyHeight) &&
                    (projectilesY[p] + ProjectileHeight > enemiesY[e]))
                {
                    int defeatPoints = 50 * 2;
                    if (enemiesY[e] < 400) playerScore += 150;
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
            
            // Collision with skeletons
            for (int s = 0; s < skeletonCount; s++)
            {
                if ((projectilesX[p] < skeletonsX[s] + SkeletonWidth) &&
                    (projectilesX[p] + ProjectileWidth > skeletonsX[s]) &&
                    (projectilesY[p] < skeletonsY[s] + SkeletonHeight) &&
                    (projectilesY[p] + ProjectileHeight > skeletonsY[s]))
                {
                    int defeatPoints = 75 * 2;
                    if (!skeletonOnGround[s]) playerScore += 150;
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
            
            // Level 2: Collision with Invisible Man and Chelnov
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
        }

        // Check if level is complete
       // Check if level is complete
       // Check if level is complete
bool allEnemiesDefeated = false;
if (currentLevel == 1)
{
    allEnemiesDefeated = (enemyCount == 0 && skeletonCount == 0 && capturedCount == 0 && projectileCount == 0);
}
else if (currentLevel == 2)
{
    // For Level 2 with wave spawning, only complete when ALL waves are done
    if (useWaveSpawning)
    {
        // Level complete only if: all waves spawned AND all enemies defeated
        allEnemiesDefeated = (currentWave >= maxWaves && 
                              enemyCount == 0 && skeletonCount == 0 && 
                              invisibleCount == 0 && chelnovCount == 0 && 
                              capturedCount == 0 && chelnovProjCount == 0 && 
                              projectileCount == 0);
    }
    else
    {
        // Original behavior if wave spawning is disabled
        allEnemiesDefeated = (enemyCount == 0 && skeletonCount == 0 && 
                              invisibleCount == 0 && chelnovCount == 0 && 
                              capturedCount == 0 && chelnovProjCount == 0 && 
                              projectileCount == 0);
    }
}
            
            // DEBUG - Print counts every 60 frames
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
                if (levelNoDamage) playerScore += 1500;
                if (levelTimer < 30.0f) playerScore += 2000;
                else if (levelTimer < 45.0f) playerScore += 1000;
                else if (levelTimer < 60.0f) playerScore += 500;
            }
            else if (currentLevel == 2)
            {
                playerScore += 2000;
                if (levelNoDamage) playerScore += 2500;
                if (levelTimer < 60.0f) playerScore += 3000;
                else if (levelTimer < 90.0f) playerScore += 1500;
                else if (levelTimer < 120.0f) playerScore += 750;
            }
            
            if (speedMultiplier == 1.5f) playerScore += 500;
            else if (vacuumPower == 1.2f) playerScore += 500;
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
                            
                            // MODIFIED: Enable wave spawning
                            useWaveSpawning = true;
                            currentWave = 0;
                            waveTimer = 0.0f;
                            for (int i = 0; i < 4; i++) waveSpawned[i] = false;
                            
                            // Generate level WITHOUT spawning all enemies
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
                                false); // DON'T spawn all enemies at once!
                            
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
                else
                    stageBonusText.setString("Level 2 Complete! You Win!");
                window.draw(stageBonusText);
                
                stageScoreText.setString("Score: " + to_string(playerScore));
                window.draw(stageScoreText);
                
                if (currentLevel == 1)
                    nextLevelText.setString("Press ENTER for Level 2");
                else
                    nextLevelText.setString("Press ENTER to Play Again");
                window.draw(nextLevelText);
                
                // Draw level clear celebration player image
                float clearSpriteX = screen_x / 2 - (levelClearTex.getSize().x * 3.0f) / 2;
                float clearSpriteY = screen_y / 2 + 50;
                levelClearSprite.setPosition(clearSpriteX, clearSpriteY);
                window.draw(levelClearSprite);
                
                window.display();
            }
        }
        
        if (restartGame) break;

        // Game over logic
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
        
        if (restartGame) break; 

        // Rendering
      if (currentLevel == 1){
display_level(window, lvl, bgTex, bgSprite, blockTexture, blockSprite,
slopeLeftTexture, slopeLeftSprite, slopeRightTexture, slopeRightSprite,slopeLeftMirrorTexture, slopeLeftMirrorSprite, slopeRightMirrorTexture, slopeRightMirrorSprite,
height, width, cell_size);
}
else if(currentLevel == 2)
{
display_level(window, lvl, bgTex2, bgSprite2, blockTexture2, blockSprite2,
slopeLeftTexture, slopeLeftSprite, slopeRightTexture, slopeRightSprite,slopeLeftMirrorTexture, slopeLeftMirrorSprite, slopeRightMirrorTexture, slopeRightMirrorSprite,
height, width, cell_size);
}

float Xoffset = (64 * scale - PlayerWidth) / 2.0f;
        float Yoffset = (64 * scale - PlayerHeight);

        PlayerSprite.setPosition(player_x - Xoffset, player_y - Yoffset);
        window.draw(PlayerSprite);

        // Draw vacuum
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

        // Collision box
        RectangleShape collBox;
        collBox.setSize(Vector2f(PlayerWidth, PlayerHeight));
        collBox.setPosition(player_x, player_y);
        collBox.setFillColor(Color::Transparent);
        collBox.setOutlineColor(Color::Red);
        collBox.setOutlineThickness(2);
        window.draw(collBox);

        // Draw ghosts with walking/sucking animation
        for (int i = 0; i < enemyCount; i++)
        {
            // Check if this ghost is being sucked
            if (enemyIsCaught[i])
            {
                // Play sucking animation
                ghostSuckCounter[i]++;
                if (ghostSuckCounter[i] >= ghostSuckSpeed)
                {
                    ghostSuckCounter[i] = 0;
                    ghostSuckFrame[i]++;
                    if (ghostSuckFrame[i] >= 4)
                        ghostSuckFrame[i] = 0;
                }
                EnemySprite.setTexture(ghostSuckTex[ghostSuckFrame[i]], true);
            }
            else
            {
                // Play walking animation
                ghostAnimCounter[i]++;
                if (ghostAnimCounter[i] >= ghostAnimSpeed)
                {
                    ghostAnimCounter[i] = 0;
                    ghostAnimFrame[i]++;
                    if (ghostAnimFrame[i] >= 4)
                        ghostAnimFrame[i] = 0;
                }
                EnemySprite.setTexture(ghostWalkTex[ghostAnimFrame[i]], true);
                
                // Reset suck animation
                ghostSuckFrame[i] = 0;
                ghostSuckCounter[i] = 0;
            }
            
            // Flip sprite based on direction
            int texW = EnemySprite.getTexture()->getSize().x;
            int texH = EnemySprite.getTexture()->getSize().y;
            
            if (enemyDirection[i] == 1) // Moving right - flip
                EnemySprite.setTextureRect(IntRect(texW, 0, -texW, texH));
            else // Moving left - normal
                EnemySprite.setTextureRect(IntRect(0, 0, texW, texH));
            
            EnemySprite.setPosition(enemiesX[i], enemiesY[i]);
            window.draw(EnemySprite);
        }

        // Draw skeletons with walking/sucking animation
        for (int i = 0; i < skeletonCount; i++)
        {
            // Check if this skeleton is being sucked
            if (skeletonIsCaught[i])
            {
                // Play sucking animation
                skeletonSuckCounter[i]++;
                if (skeletonSuckCounter[i] >= skeletonSuckSpeed)
                {
                    skeletonSuckCounter[i] = 0;
                    skeletonSuckFrame[i]++;
                    if (skeletonSuckFrame[i] >= 4)
                        skeletonSuckFrame[i] = 0;
                }
                SkeletonSprite.setTexture(skeletonSuckTex[skeletonSuckFrame[i]], true);
            }
            else
            {
                // Play walking animation
                skeletonAnimCounter[i]++;
                if (skeletonAnimCounter[i] >= skeletonAnimSpeed)
                {
                    skeletonAnimCounter[i] = 0;
                    skeletonAnimFrame[i]++;
                    if (skeletonAnimFrame[i] >= 4)
                        skeletonAnimFrame[i] = 0;
                }
                SkeletonSprite.setTexture(skeletonWalkTex[skeletonAnimFrame[i]], true);
                
                // Reset suck animation
                skeletonSuckFrame[i] = 0;
                skeletonSuckCounter[i] = 0;
            }

            int texW = SkeletonSprite.getTexture()->getSize().x;
            int texH = SkeletonSprite.getTexture()->getSize().y;

            if (skeletonDirection[i] == 1)
                SkeletonSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
            else
                SkeletonSprite.setTextureRect(IntRect(0, 0, texW, texH));

            float skeletonScale = 2.0f;
            float XoffsetSkeleton = (64 * skeletonScale - SkeletonWidth) / 2.0f;
            float YoffsetSkeleton = (64 * skeletonScale - SkeletonHeight);

            SkeletonSprite.setPosition(skeletonsX[i] - XoffsetSkeleton, skeletonsY[i] - YoffsetSkeleton);
            window.draw(SkeletonSprite);
        }
        
        // Draw Level 2 enemies with animations
        if (currentLevel == 2)
        {
            // Draw Invisible Man with walking/sucking animation
            for (int i = 0; i < invisibleCount; i++)
            {
                if (invisibleIsVisible[i])
                {
                    if (invisibleIsCaught[i])
                    {
                        // Play sucking animation
                        invisibleSuckCounter[i]++;
                        if (invisibleSuckCounter[i] >= invisibleSuckSpeed)
                        {
                            invisibleSuckCounter[i] = 0;
                            invisibleSuckFrame[i]++;
                            if (invisibleSuckFrame[i] >= 4)
                                invisibleSuckFrame[i] = 0;
                        }
                        InvisibleSprite.setTexture(invisibleSuckTex[invisibleSuckFrame[i]], true);
                    }
                    else
                    {
                        // Play walking animation
                        invisibleAnimCounter[i]++;
                        if (invisibleAnimCounter[i] >= invisibleAnimSpeed)
                        {
                            invisibleAnimCounter[i] = 0;
                            invisibleAnimFrame[i]++;
                            if (invisibleAnimFrame[i] >= 4)
                                invisibleAnimFrame[i] = 0;
                        }
                        InvisibleSprite.setTexture(invisibleWalkTex[invisibleAnimFrame[i]], true);
                        
                        invisibleSuckFrame[i] = 0;
                        invisibleSuckCounter[i] = 0;
                    }
                    
                    int texW = InvisibleSprite.getTexture()->getSize().x;
                    int texH = InvisibleSprite.getTexture()->getSize().y;
                    
                    if (invisibleDirection[i] == 1)
                        InvisibleSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
                    else
                        InvisibleSprite.setTextureRect(IntRect(0, 0, texW, texH));
                    
                    InvisibleSprite.setPosition(invisiblesX[i], invisiblesY[i]);
                    window.draw(InvisibleSprite);
                }
            }
            
            // Draw Chelnov with walking/sucking animation
            for (int i = 0; i < chelnovCount; i++)
            {
                if (chelnovIsCaught[i])
                {
                    // Play sucking animation
                    chelnovSuckCounter[i]++;
                    if (chelnovSuckCounter[i] >= chelnovSuckSpeed)
                    {
                        chelnovSuckCounter[i] = 0;
                        chelnovSuckFrame[i]++;
                        if (chelnovSuckFrame[i] >= 4)
                            chelnovSuckFrame[i] = 0;
                    }
                    ChelnovSprite.setTexture(chelnovSuckTex[chelnovSuckFrame[i]], true);
                }
                else
                {
                    // Play walking animation
                    chelnovAnimCounter[i]++;
                    if (chelnovAnimCounter[i] >= chelnovAnimSpeed)
                    {
                        chelnovAnimCounter[i] = 0;
                        chelnovAnimFrame[i]++;
                        if (chelnovAnimFrame[i] >= 4)
                            chelnovAnimFrame[i] = 0;
                    }
                    ChelnovSprite.setTexture(chelnovWalkTex[chelnovAnimFrame[i]], true);
                    
                    chelnovSuckFrame[i] = 0;
                    chelnovSuckCounter[i] = 0;
                }
                
                int texW = ChelnovSprite.getTexture()->getSize().x;
                int texH = ChelnovSprite.getTexture()->getSize().y;
                
                if (chelnovDirection[i] == 1)
                    ChelnovSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
                else
                    ChelnovSprite.setTextureRect(IntRect(0, 0, texW, texH));
                
                ChelnovSprite.setPosition(chelnovsX[i], chelnovsY[i]);
                window.draw(ChelnovSprite);
            }
            
            for (int i = 0; i < chelnovProjCount; i++)
            {
                if (chelnovProjActive[i])
                {
                    chelnovProjSprite.setPosition(chelnovProjX[i], chelnovProjY[i]);
                    window.draw(chelnovProjSprite);
                }
            }
        }
        
        // Draw projectiles
        for (int p = 0; p < projectileCount; p++)
        {
            if (!projectileActive[p]) continue;
            
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
            if (!powerupActive[i]) continue;
            
            float floatOffset = sin(powerupAnimTimer[i] * 3.0f) * 5.0f;
            
            switch(powerupType[i])
            {
                case 1: powerupSprite.setTexture(speedPowerupTex); break;
                case 2: powerupSprite.setTexture(rangePowerupTex); break;
                case 3: powerupSprite.setTexture(powerPowerupTex); break;
                case 4: powerupSprite.setTexture(lifePowerupTex); break;
            }
            
            powerupSprite.setPosition(powerupsX[i], powerupsY[i] + floatOffset);
            window.draw(powerupSprite);
        }
        
        // UI
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

        // NEW: Wave display for Level 2
        if (currentLevel == 2 && useWaveSpawning)
        {
            Text waveText("Wave: " + to_string(currentWave + 1) + "/" + to_string(maxWaves), font, 35);
            waveText.setFillColor(Color::Red);
            waveText.setPosition(screen_x / 2 - 100, 60);
            window.draw(waveText);
            
            // Show countdown between waves
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
        if (hasSpeedBoost) activeEffects += "SPEED ";
        if (hasRangeBoost) activeEffects += "RANGE ";
        if (hasPowerBoost) activeEffects += "POWER ";

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

    } // End of game loop
 
    lvlMusic.stop();
    lvl2Music.stop();
    for (int i = 0; i < height; i++)
    {
        delete[] lvl[i];
    }
    delete[] lvl;
    
} // End of playagain loop

return 0;
}