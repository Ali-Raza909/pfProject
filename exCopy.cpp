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
            PlayerSprite.setTextureRect(IntRect(texW, 0, -texW, texH)); //-ve width flips the sprite horizontally
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

// This function draws levels. It does so based on '#' and '-' and 'e' and 's' and '/' and '\'
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

// This functions controls player movement. It has constraints based on if there is a solid block after it
void playerMovement(char **lvl, float &player_x, float player_y,
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

// This function handles vacuum for phase 1
void handleVacuum(RenderWindow &window, Sprite &vacSprite,                      // vacuum sprite and window to draw in it
                  Texture &texHorz, Texture &texVert,                           // textures of vacuum image
                  float player_x, float player_y,                               // player position. We draw there
                  int PlayerWidth, int PlayerHeight, int vacDir, bool isActive, // player width and height to calc it's center, vaccum direction based on keys
                  float *enemiesX, float *enemiesY, int &enemyCount,            // enemies refers to ghost here
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
        vacSprite.setPosition(pX + PlayerWidth, pCenterY - beamThick / 2);
        vacSprite.setTextureRect(IntRect(0, 0, beamReach, beamThick));

        vacHitbox = IntRect(pX + PlayerWidth, pCenterY - beamThick / 2, beamReach * 2, beamThick * 2);
    }
    else if (vacDir == 1) // LEFT
    {
        vacSprite.setTexture(texHorz, true);
        // Position: Exact Left Edge minus beam length.
        vacSprite.setPosition(pX - (beamReach * 2), pCenterY - beamThick / 2);
        // Draw backwards (Visual only)
        vacSprite.setTextureRect(IntRect(beamReach, 0, -beamReach, beamThick));

        // Hitbox starts to the left of the player
        vacHitbox = IntRect(pX - (beamReach * 2), pCenterY - beamThick / 2, beamReach * 2, beamThick * 2);
    }
    else if (vacDir == 2) // UP
    {
        vacSprite.setTexture(texVert, true);
        // X: Center Horizontal. Y: Exact Top Edge minus beam length.
        vacSprite.setPosition(pCenterX - beamThick / 2, pY - (beamReach * 2));
        vacSprite.setTextureRect(IntRect(0, 0, beamThick, beamReach));

        vacHitbox = IntRect(pCenterX - beamThick / 2, pY - (beamReach * 2), beamThick * 2, beamReach * 2);
    }
    else if (vacDir == 3) // DOWN
    {
        vacSprite.setTexture(texVert, true);
        // X: Center Horizontal. Y: Exact Bottom Edge.
        vacSprite.setPosition(pCenterX - beamThick / 2, pY + PlayerHeight);
        vacSprite.setTextureRect(IntRect(0, 0, beamThick, beamReach));

        vacHitbox = IntRect(pCenterX - beamThick / 2, pY + PlayerHeight, beamThick * 2, beamReach * 2);
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
            float pullSpeed = 250.0f * vacuumPower * dt; // Pull speed affected by power

            if (enemiesX[i] < pCenterX)
                enemiesX[i] += pullSpeed;
            else
                enemiesX[i] -= pullSpeed;

            if (enemiesY[i] < pCenterY)
                enemiesY[i] += pullSpeed;
            else
                enemiesY[i] -= pullSpeed;

            // CAPTURE CHECK (RESTORED TO ORIGINAL DISTANCE)
            float dx = (float)(pCenterX - (enemiesX[i] + 30));
            float dy = (float)(pCenterY - (enemiesY[i] + 30));
            float dist = sqrt(dx * dx + dy * dy);

            if (dist < 50.0f * vacuumPower)
            {
                inventory[capturedCount] = enemyType;
                capturedCount++;

                // Award capture points ONLY during logic pass
                if (!drawOnly)
                {
                    int capturePoints = 0;
                    if (enemyType == 1)
                        capturePoints = 50; // Ghost
                    else if (enemyType == 2)
                        capturePoints = 75; // Skeleton
                    else if (enemyType == 3)
                        capturePoints = 150; // Invisible Man
                    else if (enemyType == 4)
                        capturePoints = 200; // Chelnov

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

// This function handles vacuum for phase 2
void handleVacuumPhase2(RenderWindow &window, Sprite &vacSprite,
                        Texture &texHorz, Texture &texVert,
                        float player_x, float player_y,
                        int PlayerWidth, int PlayerHeight, int vacDir, bool isActive,
                        float *enemiesX, float *enemiesY, int &enemyCount,
                        int *&inventory, int &capturedCount, // Passed by reference for dynamic update
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

    // 1. Visuals & Hitbox (Simplified for Phase 2 context)
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
    // Direction Logic
    if (vacDir == 0)
    { // RIGHT
        vacSprite.setTexture(texHorz, true);
        vacSprite.setPosition(pX + PlayerWidth, pCenterY - beamThick / 2);
        vacSprite.setTextureRect(IntRect(0, 0, beamReach, beamThick));
        vacHitbox = IntRect(pX + PlayerWidth, pCenterY - beamThick / 2, beamReach * 2, beamThick * 2);
    }
    else if (vacDir == 1)
    { // LEFT
        vacSprite.setTexture(texHorz, true);
        vacSprite.setPosition(pX - (beamReach * 2), pCenterY - beamThick / 2);
        vacSprite.setTextureRect(IntRect(beamReach, 0, -beamReach, beamThick));
        vacHitbox = IntRect(pX - (beamReach * 2), pCenterY - beamThick / 2, beamReach * 2, beamThick * 2);
    }
    else if (vacDir == 2)
    { // UP
        vacSprite.setTexture(texVert, true);
        vacSprite.setPosition(pCenterX - beamThick / 2, pY - (beamReach * 2));
        vacSprite.setTextureRect(IntRect(0, 0, beamThick, beamReach));
        vacHitbox = IntRect(pCenterX - beamThick / 2, pY - (beamReach * 2), beamThick * 2, beamReach * 2);
    }
    else if (vacDir == 3)
    { // DOWN
        vacSprite.setTexture(texVert, true);
        vacSprite.setPosition(pCenterX - beamThick / 2, pY + PlayerHeight);
        vacSprite.setTextureRect(IntRect(0, 0, beamThick, beamReach));
        vacHitbox = IntRect(pCenterX - beamThick / 2, pY + PlayerHeight, beamThick * 2, beamReach * 2);
    }

    if (drawOnly)
    {
        if (showSprite)
            window.draw(vacSprite);
        return;
    }

    // 2. Logic Pass
    for (int i = 0; i < enemyCount; i++)
    {
        IntRect enemyRect((int)enemiesX[i], (int)enemiesY[i], 60, 60);

        if (vacHitbox.intersects(enemyRect))
        {
            enemyIsCaught[i] = true;

            // Pull Logic
            float pullSpeed = 250.0f * vacuumPower * dt;
            if (enemiesX[i] < pCenterX)
                enemiesX[i] += pullSpeed;
            else
                enemiesX[i] -= pullSpeed;
            if (enemiesY[i] < pCenterY)
                enemiesY[i] += pullSpeed;
            else
                enemiesY[i] -= pullSpeed;

            // Capture Check
            float dx = (float)(pCenterX - (enemiesX[i] + 30));
            float dy = (float)(pCenterY - (enemiesY[i] + 30));
            float dist = sqrt(dx * dx + dy * dy);

            if (dist < 50.0f * vacuumPower)
            {
                // === PHASE 2 DYNAMIC RESIZING (Exact Fit) [cite: 50] ===
                // 1. Create new array size + 1
                int *newArr = new int[capturedCount + 1];

                // 2. Copy old data
                for (int k = 0; k < capturedCount; k++)
                {
                    newArr[k] = inventory[k];
                }

                // 3. Add new item
                newArr[capturedCount] = enemyType;

                // 4. Delete old array
                if (inventory != nullptr)
                {
                    delete[] inventory;
                }

                // 5. Point to new array
                inventory = newArr;
                capturedCount++;
                // ============================================

                if (!drawOnly)
                {
                    // Basic scoring logic (Phase 2 points can be added here)
                    addScore(playerScore, comboStreak, comboTimer, 100,
                             false, multiKillCount, multiKillTimer, dt);
                }

                // Remove enemy from world
                enemiesX[i] = enemiesX[enemyCount - 1];
                enemiesY[i] = enemiesY[enemyCount - 1];
                enemyCount--;
                i--;
            }
        }
    }
}

void spawnPowerup(float *powerupsX, float *powerupsY, int *powerupType,
                  bool *powerupActive, float *powerupAnimTimer,
                  int &powerupCount, int maxPowerups, char **lvl,
                  int mapWidth, int mapHeight, int cellSize)
{
    if (powerupCount >= maxPowerups)
        return;

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
    if (!onGround)
        return;

    int feet_row = (int)(player_y + PlayerHeight) / cell_size;
    int feet_col_left = (int)(player_x) / cell_size;
    int feet_col_right = (int)(player_x + PlayerWidth) / cell_size;

    char tile_left = get_tile(lvl, feet_row, feet_col_left, height, width);
    char tile_right = get_tile(lvl, feet_row, feet_col_right, height, width);

    float slideSpeedX = 80.0f; // Horizontal slide speed
    float slideSpeedY = 80.0f; // Vertical slide speed (going down)

    bool onSlope = false;
    int slideDirectionX = 0; // -1 = left, 1 = right

    // Down-right slope (\) using 'l' and 'r' - slide right and down
    if (tile_left == 'l' || tile_right == 'l' || tile_left == 'r' || tile_right == 'r')
    {
        onSlope = true;
        slideDirectionX = 1; // Slide right
    }
    // Down-left slope (/) using 'L' and 'R' - slide left and down
    else if (tile_left == 'L' || tile_right == 'L' || tile_left == 'R' || tile_right == 'R')
    {
        onSlope = true;
        slideDirectionX = -1; // Slide left
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

    // --- 4. GENERATE HORIZONTAL PLATFORMS ---
    int minPlatformLength = 3;
    int platformRows[] = {2, 4, 6, 8, 9}; // 10->9
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

    // --- 5. ENSURE GUARANTEED PLATFORMS (WITH GAPS) ---
    bool hasTopLeft = false;
    for (int j = 1; j <= 4; j++)
        if (lvl[2][j] == '-')
            hasTopLeft = true;

    if (!hasTopLeft)
    {
        // Create platform with a gap (only columns 1-2, leave 3-4 empty)
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
        // Create platform with a gap (only last 2 columns, leave others empty)
        for (int j = platWidth - 4; j <= platWidth - 3; j++)
            if (lvl[1][j] == ' ')
                lvl[1][j] = '-';
    }

    // Bottom platforms with gaps
    for (int j = 1; j <= 3; j++) // Reduced from 5 to 3
        if (lvl[9][j] == ' ')
            lvl[9][j] = '-';

    for (int j = platWidth - 5; j <= platWidth - 3; j++) // Reduced range
        if (lvl[9][j] == ' ')
            lvl[9][j] = '-';

    // --- 6. DEBUG OUTPUT ---
    cout << "=== Level 2 Design Generated ===" << endl;
    cout << "Slant direction: " << (direction == 0 ? "Down-Right (\\)" : "Down-Left (/)") << endl;
    cout << "Slant start: row " << randTopRow << ", col " << randTopCol << endl;
    cout << "Slant length: " << slantLength << " tiles" << endl;
}

// This function spawns enemies in level two in 4 waves based on waveNumber
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

    case 2: // Wave 3 - 3 Skeletons + 2 Chelnovs + 2 Invisible Men
    {
        // Spawn 3 Skeletons
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

        // Spawn 2 Chelnovs
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

        // Spawn 2 Invisible Men
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

    case 3: // Wave 4 - BOSS WAVE: 2 Chelnovs + 1 Invisible Man
    {
        // Spawn 2 Chelnovs
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

        // Spawn 1 Invisible Man
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

// MODIFIED FUNCTION: Generate Level 2 map with optional enemy spawning
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

        // Spawn 3 invisible men - using explicit float casts
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

        // Spawn 4 chelnovs - using explicit float casts
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
        const float multiKillWindow = 1.0f; // 1 second window for multi-kills

        // Level tracking variables
        int currentLevel = 1;
        int selectedStartLevel = 1; // NEW: Track which level player selected from menu
        bool showStageClear = false;

        // NEW: Character type flag for Phase 2 (360 rotation for yellow)
        bool isYellowCharacter = false;

        // NEW: Wave spawning system variables
        bool useWaveSpawning = false;
        int currentWave = 0;
        int maxWaves = 4;
        float waveTimer = 0.0f;
        float timeBetweenWaves = 5.0f; // 5 seconds between waves
        bool waveSpawned[4] = {false, false, false, false};

        // ============================================================================
        // BOSS LEVEL (LEVEL 3) VARIABLES
        // ============================================================================

        // Boss variables
        int bossHealth = 6; // Boss starts with 6 health
        int maxBossHealth = 6;
        float bossX = 0;
        float bossY = 0;
        int bossWidth = 200;
        int bossHeight = 180;
        bool bossIsAngry = false; // Becomes true when health <= 2
        bool bossDefeated = false;

        // Minion variables (dynamically spawned by boss)
        int maxMinions = 20;
        float *minionsX = NULL;
        float *minionsY = NULL;
        float *minionSpeed = NULL;
        int *minionDirection = NULL;
        float *minionVelocityY = NULL;
        bool *minionOnGround = NULL;
        bool *minionIsCaught = NULL;
        bool *minionFollowingPlayer = NULL; // For angry mode
        int minionCount = 0;
        int minionWidth = 48;
        int minionHeight = 48;
        float minionSpawnTimer = 0.0f;
        float minionSpawnInterval = 3.0f; // Spawn minions every 3 seconds

        // Tentacle variables (dynamic array that resizes)
        int maxTentacles = 10;
        float *tentaclesX = NULL;
        float *tentaclesY = NULL;
        int *tentacleWidth = NULL;
        int *tentacleHeight = NULL;
        float *tentacleTimer = NULL;    // How long tentacle has been active
        float *tentacleDuration = NULL; // How long tentacle will stay
        bool *tentacleActive = NULL;
        int tentacleCount = 0;
        float tentacleSpawnTimer = 0.0f;
        float tentacleSpawnInterval = 4.0f; // Check to spawn tentacle every 4 seconds

        // Level 3 specific dimensions (1.5x scaling)
        int level3Height = 21;   // 14 * 1.5 = 21
        int level3Width = 30;    // 20 * 1.5 = 30
        int level3CellSize = 42; // 64 / 1.5 ≈ 42 to fit same screen

        // Boss level map (separate from regular lvl)
        char **bossLvl = NULL;

        // ============================================================================
        // POT AND CLOUD VARIABLES (Phase 2 - DynamEnemies system)
        // The pot sits on a cloud and spawns enemies dynamically.
        // Pot is destroyed after 4 projectile hits, then boss appears.
        // Cloud becomes a platform after pot is destroyed.
        // ============================================================================
        bool potActive = true; // Pot is active until destroyed
        int potHealth = 4;     // Pot takes 4 hits to destroy
        int potMaxHealth = 4;
        float potX = 0;
        float potY = 0;
        int potWidth = 80;
        int potHeight = 80;
        float potEnemySpawnTimer = 0.0f;
        float potEnemySpawnInterval = 4.0f; // Spawn enemy every 4 seconds
        bool potDestroyed = false;          // Visual feedback when destroyed
        float potDestroyTimer = 0.0f;       // Timer for destruction animation

        // Cloud variables (moves up and down, carries the pot)
        float cloudX = 0;
        float cloudY = 0;
        int cloudWidth = 150;
        int cloudHeight = 50;
        float cloudMinY = 150.0f;     // Top bound for cloud movement
        float cloudMaxY = 700.0f;     // Bottom bound for cloud movement
        float cloudSpeed = 40.0f;     // Cloud movement speed
        int cloudDirection = 1;       // 1 = moving down, -1 = moving up
        bool cloudIsPlatform = false; // Becomes true after pot is destroyed

        // Boss appearance flag (boss only appears after pot is destroyed)
        bool bossAppeared = false;

        // ============================================================================
        // DYNAMIC ENEMY ARRAYS FOR POT-SPAWNED ENEMIES (Phase 2)
        // These are separate from minions - pot spawns ghost/skeleton/invisible/chelnov
        // Uses dynamic memory allocation with manual resizing
        // ============================================================================
        float *potEnemiesX = NULL;
        float *potEnemiesY = NULL;
        float *potEnemySpeed = NULL;
        int *potEnemyDirection = NULL;
        float *potEnemyVelocityY = NULL;
        bool *potEnemyOnGround = NULL;
        bool *potEnemyIsCaught = NULL;
        int *potEnemyType = NULL; // 1=ghost, 2=skeleton, 3=invisible, 4=chelnov
        int potEnemyCount = 0;
        int potEnemyCapacity = 0; // Current allocated capacity

        // Additional behavior arrays for pot enemies (skeleton jump, invisible visibility, chelnov shooting)
        float *potEnemyJumpTimer = NULL;
        bool *potEnemyShouldJump = NULL;
        int *potEnemyStableFrames = NULL;
        bool *potEnemyIsVisible = NULL;        // For invisible man
        float *potEnemyVisibilityTimer = NULL; // For invisible man
        float *potEnemyTeleportTimer = NULL;   // For invisible man
        float *potEnemyShootTimer = NULL;      // For chelnov
        bool *potEnemyIsShooting = NULL;       // For chelnov

        // Pot enemy projectiles (for chelnov)
        const int maxPotEnemyProjectiles = 20;
        float potEnemyProjX[20];
        float potEnemyProjY[20];
        int potEnemyProjDirection[20];
        bool potEnemyProjActive[20];
        int potEnemyProjCount = 0;

        // ============================================================================
        // DYNAMIC CAPTURED ENEMIES ARRAY (Phase 2 requirement)
        // Memory allocation resizes on every capture/release
        // ============================================================================
        int *dynamicCapturedEnemies = NULL;
        int dynamicCapturedCount = 0;
        int dynamicCapturedCapacity = 0;

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
                        // YELLOW CHARACTER - has 360 rotation in Phase 2
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
                        // GREEN CHARACTER - keeps original 90 degree rotation
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

        // ============================================================================
        // LEVEL SELECTION MENU
        // ============================================================================
        Text levelSelectTitle("SELECT LEVEL", font, 90);
        levelSelectTitle.setFillColor(Color::Cyan);
        levelSelectTitle.setPosition(320, 120);

        Text level1Option("Press 1 - Level 1 (Classic)", font, 50);
        level1Option.setFillColor(Color::White);
        level1Option.setPosition(280, 300);

        Text level2Option("Press 2 - Level 2 (Slopes & Waves)", font, 50);
        level2Option.setFillColor(Color::White);
        level2Option.setPosition(200, 400);

        Text level3Option("Press 3 - Boss Level (Octopus)", font, 50);
        level3Option.setFillColor(Color(255, 100, 100)); // Reddish - Boss level
        level3Option.setPosition(180, 500);

        Text backOption("Press ESC to go back", font, 40);
        backOption.setFillColor(Color::Red);
        backOption.setPosition(380, 650);

        // Selection indicator for arrow key navigation
        int menuSelectedLevel = 1;
        Text selectionIndicator(">", font, 50);
        selectionIndicator.setFillColor(Color::Yellow);

        bool levelSelected = false;

        // Level selection happens here
        while (window.isOpen() && !levelSelected)
        {
            Event e;
            while (window.pollEvent(e))
            {
                if (e.type == Event::Closed)
                    window.close();

                if (e.type == Event::KeyPressed)
                {
                    // Direct number key selection
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
                    // Arrow key navigation
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
                    // Enter to confirm arrow selection
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

            // Update selection indicator position
            if (menuSelectedLevel == 1)
                selectionIndicator.setPosition(240, 300);
            else if (menuSelectedLevel == 2)
                selectionIndicator.setPosition(160, 400);
            else if (menuSelectedLevel == 3)
                selectionIndicator.setPosition(140, 500);

            // Highlight selected option
            level1Option.setFillColor(menuSelectedLevel == 1 ? Color::Yellow : Color::White);
            level2Option.setFillColor(menuSelectedLevel == 2 ? Color::Yellow : Color::White);
            level3Option.setFillColor(menuSelectedLevel == 3 ? Color::Yellow : Color(255, 100, 100));

            // Draw the menu
            window.clear(Color::Black);
            window.draw(menuBGSprite);
            window.draw(levelSelectTitle);
            window.draw(selectionIndicator);
            window.draw(level1Option);
            window.draw(level2Option);
            window.draw(level3Option);
            window.draw(backOption);

            // Show which character was selected
            Text charInfo("", font, 30);
            charInfo.setFillColor(Color::Green);
            charInfo.setPosition(20, 20);
            if (isYellowCharacter)
                charInfo.setString("Character: Yellow (Fast)");
            else
                charInfo.setString("Character: Green (Strong Vacuum)");
            window.draw(charInfo);

            window.display();
        }

        if (!levelSelected)
            return 0;
        // ============================================================================
        // END OF LEVEL SELECTION MENU
        // ============================================================================

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
            float(screen_y) / bgTex2.getSize().y);

        // BLOCKS FOR LEVEL 2
        blockTexture2.loadFromFile("Data/block2.png"); // new block
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

        // ============================================================================
        // BOSS LEVEL (LEVEL 3) ASSETS
        // ============================================================================

        // Level 3 Background
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

        // Level 3 Block
        Texture blockTexture3;
        Sprite blockSprite3;
        if (!blockTexture3.loadFromFile("Data/block3.PNG"))
        {
            cout << "block3.png missing! Using block1.png as fallback.\n";
            blockTexture3.loadFromFile("Data/block1.png");
        }
        blockSprite3.setTexture(blockTexture3);

        // Level 3 Music
        Music lvl3Music;
        if (!lvl3Music.openFromFile("Data/mus3.ogg"))
        {
            cout << "mus3.ogg missing! Using mus.ogg as fallback.\n";
            lvl3Music.openFromFile("Data/mus.ogg");
        }
        lvl3Music.setVolume(25);
        lvl3Music.setLoop(true);

        // Octopus Boss Textures
        Texture bossTexture;
        Sprite bossSprite;
        if (!bossTexture.loadFromFile("Data/octopus/Octopus.png"))
        {
            cout << "octopus.png missing!\n";
        }
        bossSprite.setTexture(bossTexture);
        bossSprite.setScale(3.0f, 3.0f);

        // Octopus Boss Angry Texture
        Texture bossAngryTexture;
        if (!bossAngryTexture.loadFromFile("Data/boss/octopus_angry.png"))
        {
            cout << "octopus_angry.png missing! Will use color tint.\n";
        }

        // Tentacle Texture
        Texture tentacleTexture;
        Sprite tentacleSprite;
        if (!tentacleTexture.loadFromFile("Data/octopus/Tentacles.png"))
        {
            cout << "tentacle.png missing!\n";
        }
        tentacleSprite.setTexture(tentacleTexture);
        tentacleSprite.setScale(2.0f, 2.0f);

        // Minion Texture
        Texture minionTexture;
        Sprite minionSprite;
        if (!minionTexture.loadFromFile("Data/octopus/min1.png"))
        {
            cout << "minion.png missing! Using ghost as fallback.\n";
            minionTexture.loadFromFile("Data/ghost.png");
        }
        minionSprite.setTexture(minionTexture);
        minionSprite.setScale(1.5f, 1.5f);

        // Minion Roll Textures (for projectile state)
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

        // ============================================================================
        // POT AND CLOUD TEXTURES (Phase 2 - DynamEnemies system)
        // ============================================================================
        Texture potTexture;
        Sprite potSprite;
        if (!potTexture.loadFromFile("Data/pot.png"))
        {
            cout << "pot.png missing!\n";
        }
        potSprite.setTexture(potTexture);
        potSprite.setScale(2.0f, 2.0f);

        Texture cloudTexture;
        Sprite cloudSprite;
        if (!cloudTexture.loadFromFile("Data/cloud.png"))
        {
            cout << "cloud.png missing!\n";
        }
        cloudSprite.setTexture(cloudTexture);
        cloudSprite.setScale(2.5f, 1.5f); // Stretch cloud horizontally

        // ============================================================================
        // END OF BOSS LEVEL ASSETS
        // ============================================================================

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

        // Phase 2 Dynamic Variables
        dynamicCapturedEnemies = nullptr;
        dynamicCapturedCount = 0;
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

        // ============================================================================
        // INITIALIZE LEVEL BASED ON MENU SELECTION
        // ============================================================================
        if (currentLevel == 1)
        {
            // --- LEVEL 1 INITIALIZATION ---
            cout << "Initializing Level 1..." << endl;

            // Level 1 layout (original)
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

            // Floor and Sides
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

            // Enemy spawn markers
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

            // Initialize ghosts from markers
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

            // Initialize skeletons from markers
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

            // Set Level 1 capacity
            MAX_CAPACITY = 3;

            // Start Level 1 music
            lvlMusic.play();
        }
        else if (currentLevel == 2)
        {
            // --- LEVEL 2 INITIALIZATION ---
            cout << "Initializing Level 2..." << endl;

            // Generate Level 2 map (don't spawn all enemies - waves will do it)
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
                              false); // false = don't spawn all enemies, use wave system

            // Set Level 2 capacity
            MAX_CAPACITY = 5;

            // Reset wave system
            useWaveSpawning = true;
            currentWave = 0;
            waveTimer = 0.0f;
            for (int w = 0; w < 4; w++)
                waveSpawned[w] = false;

            // Start Level 2 music
            lvl2Music.play();

            cout << "Level 2 initialized with wave spawning system" << endl;
        }
        else if (currentLevel == 3)
        {
            // ============================================================================
            // BOSS LEVEL (LEVEL 3) INITIALIZATION
            // Phase 2: Pot on Cloud spawns enemies first, then Boss appears. Boss is octopus
            // ============================================================================
            cout << "Initializing Boss Level (Level 3)..." << endl;

            // Allocate dynamic arrays for minions (spawned by boss AFTER pot is destroyed)

            minionsX = NULL;
            minionsY = NULL;
            minionSpeed = NULL;
            minionDirection = NULL;
            minionVelocityY = NULL;
            minionOnGround = NULL;
            minionIsCaught = NULL;
            minionFollowingPlayer = NULL;
            minionCount = 0;

            // Allocate dynamic arrays for tentacles
            tentaclesX = NULL;
            tentaclesY = NULL;
            tentacleWidth = NULL;
            tentacleHeight = NULL;
            tentacleTimer = NULL;
            tentacleDuration = NULL;
            tentacleActive = NULL;
            tentacleCount = 0;

            // ============================================================================
            // POT AND CLOUD INITIALIZATION (Phase 2)
            // Pot sits on cloud at top-center, spawns enemies until destroyed
            // ============================================================================
            potActive = true;
            potHealth = potMaxHealth;
            potDestroyed = false;
            potDestroyTimer = 0.0f;
            potEnemySpawnTimer = 0.0f;

            // Position cloud at top-center of screen
            cloudX = screen_x / 2 - cloudWidth / 2;
            cloudY = cloudMinY;
            cloudDirection = 1; // Start moving down
            cloudIsPlatform = false;

            // Position pot on top of cloud
            potX = cloudX + cloudWidth / 2 - potWidth / 2;
            potY = cloudY - potHeight;

            // Initialize pot enemy arrays (start with 0 capacity - will grow dynamically)
            potEnemyCount = 0;
            potEnemyCapacity = 0;
            potEnemiesX = NULL;
            potEnemiesY = NULL;
            potEnemySpeed = NULL;
            potEnemyDirection = NULL;
            potEnemyVelocityY = NULL;
            potEnemyOnGround = NULL;
            potEnemyIsCaught = NULL;
            potEnemyType = NULL;

            // Boss does NOT appear until pot is destroyed
            bossAppeared = false;
            bossHealth = maxBossHealth;
            bossIsAngry = false;
            bossDefeated = false;
            bossX = screen_x / 2 - bossWidth / 2;
            bossY = -bossHeight; // Start off-screen, will move down when appearing

            // Reset timers
            minionSpawnTimer = 0.0f;
            tentacleSpawnTimer = 0.0f;

            // ============================================================================
            // BOSS LEVEL MAP CREATION - FIXED LAYOUT
            // Floor properly positioned, side walls complete
            // ============================================================================
            bossLvl = new char *[level3Height];
            for (int i = 0; i < level3Height; i++)
            {
                bossLvl[i] = new char[level3Width];
                for (int j = 0; j < level3Width; j++)
                    bossLvl[i][j] = ' ';
            }

            // Calculate proper floor row (should be visible on screen)
            // screen_y = 950, level3CellSize = 42
            // We want floor at y = 850, so row = 850/42 = 20
            // But level3Height = 21, so floor at row 18 gives y = 18*42 = 756
            int floorRow = level3Height - 2; // Row 19 (y = 19*42 = 798)

            // Floor at bottom - FULL WIDTH
            for (int j = 0; j < level3Width; j++)
                bossLvl[floorRow][j] = '#';

            // Side walls - FULL HEIGHT up to floor
            for (int i = 0; i <= floorRow; i++)
            {
                bossLvl[i][0] = '#';
                bossLvl[i][level3Width - 1] = '#';
            }

            // Platforms for player to jump on - adjusted positions
            // Left platform (row 14, y ≈ 588)
            for (int j = 2; j < 8; j++)
                bossLvl[floorRow - 5][j] = '-';

            // Right platform (row 14, y ≈ 588)
            for (int j = level3Width - 8; j < level3Width - 2; j++)
                bossLvl[floorRow - 5][j] = '-';

            // Middle platform (row 16, y ≈ 672)
            for (int j = 11; j < 19; j++)
                bossLvl[floorRow - 3][j] = '-';

            // Upper left platform (row 10, y ≈ 420)
            for (int j = 3; j < 10; j++)
                bossLvl[floorRow - 9][j] = '-';

            // Upper right platform (row 10, y ≈ 420)
            for (int j = level3Width - 10; j < level3Width - 3; j++)
                bossLvl[floorRow - 9][j] = '-';

            // Top middle platform (row 7, y ≈ 294)
            for (int j = 10; j < 20; j++)
                bossLvl[floorRow - 12][j] = '-';

            // Player starting position - on the floor, properly calculated
            player_x = screen_x / 2 - PlayerWidth / 2;
            player_y = (floorRow * level3CellSize) - PlayerHeight - 5; // Just above floor

            // Initialize dynamic captured enemies array for Phase 2
            dynamicCapturedCount = 0;
            dynamicCapturedCapacity = 0;
            dynamicCapturedEnemies = NULL;

            // Initialize pot enemy projectile array
            potEnemyProjCount = 0;
            for (int i = 0; i < maxPotEnemyProjectiles; i++)
            {
                potEnemyProjActive[i] = false;
            }

            // No capacity limit in boss level (dynamic capture)
            MAX_CAPACITY = 999;
            MAX_CAPACITY = 999;

            // Stop other music, start boss music
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
        // ============================================================================
        // END OF LEVEL INITIALIZATION
        // ============================================================================

        // Spawn initial powerups (works for both levels)
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

            // ============================================================================
            // BOSS LEVEL (LEVEL 3) GAME LOGIC
            // Phase 2: Pot spawns enemies first, then Boss appears after pot destroyed
            // ============================================================================
            if (currentLevel == 3)
            {
                // ============================================================================
                // PHASE 1: POT AND CLOUD LOGIC (before boss appears)
                // ============================================================================
                if (potActive && !potDestroyed)
                {
                    // Update movement of cloud up and down. -1 = up and 1 = down
                    cloudY += cloudSpeed * cloudDirection * dt;

                    // Reverse direction at top and bottom
                    if (cloudY >= cloudMaxY)
                    {
                        cloudY = cloudMaxY;
                        cloudDirection = -1; // Move up
                    }
                    else if (cloudY <= cloudMinY)
                    {
                        cloudY = cloudMinY;
                        cloudDirection = 1; // Move down
                    }

                    // Update pot position to stay on cloud. Pot on center of cloud
                    potX = cloudX + cloudWidth / 2 - potWidth / 2;
                    potY = cloudY - potHeight;

                    // Spawn enemies on pot every 4 secs
                    potEnemySpawnTimer += dt;
                    if (potEnemySpawnTimer >= potEnemySpawnInterval)
                    {
                        potEnemySpawnTimer = 0.0f;

                        // Dynamically resize pot enemy arrays (add 1 capacity)
                        int newCapacity = potEnemyCapacity + 1;
                        float *newX = new float[newCapacity];
                        float *newY = new float[newCapacity];
                        float *newSpeed = new float[newCapacity];
                        int *newDir = new int[newCapacity];
                        float *newVelY = new float[newCapacity];
                        bool *newGround = new bool[newCapacity];
                        bool *newCaught = new bool[newCapacity];
                        int *newType = new int[newCapacity];
                        // New behavior arrays
                        float *newJumpTimer = new float[newCapacity];
                        bool *newShouldJump = new bool[newCapacity];
                        int *newStableFrames = new int[newCapacity];
                        bool *newIsVisible = new bool[newCapacity];
                        float *newVisTimer = new float[newCapacity];
                        float *newTeleTimer = new float[newCapacity];
                        float *newShootTimer = new float[newCapacity];
                        bool *newIsShooting = new bool[newCapacity];

                        // Copy existing data
                        for (int i = 0; i < potEnemyCount; i++)
                        {
                            newX[i] = potEnemiesX[i];
                            newY[i] = potEnemiesY[i];
                            newSpeed[i] = potEnemySpeed[i];
                            newDir[i] = potEnemyDirection[i];
                            newVelY[i] = potEnemyVelocityY[i];
                            newGround[i] = potEnemyOnGround[i];
                            newCaught[i] = potEnemyIsCaught[i];
                            newType[i] = potEnemyType[i];
                            // Copy behavior data
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
                        }

                        // Delete old arrays
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

                        // Assign new arrays
                        potEnemiesX = newX;
                        potEnemiesY = newY;
                        potEnemySpeed = newSpeed;
                        potEnemyDirection = newDir;
                        potEnemyVelocityY = newVelY;
                        potEnemyOnGround = newGround;
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
                        potEnemyCapacity = newCapacity;

                        // Add new enemy - random type (1=ghost, 2=skeleton, 3=invisible, 4=chelnov)
                        int enemyTypeToSpawn = 1 + (rand() % 4);

                        potEnemiesX[potEnemyCount] = potX + potWidth / 2 - 30; // Spawn from pot center
                        potEnemiesY[potEnemyCount] = potY + potHeight / 2;
                        potEnemySpeed[potEnemyCount] = 40.0f + (rand() % 30);
                        potEnemyDirection[potEnemyCount] = (rand() % 2 == 0) ? -1 : 1;
                        potEnemyVelocityY[potEnemyCount] = 0;
                        potEnemyOnGround[potEnemyCount] = false;
                        potEnemyIsCaught[potEnemyCount] = false;
                        potEnemyType[potEnemyCount] = enemyTypeToSpawn;
                        // Initialize behavior values
                        potEnemyJumpTimer[potEnemyCount] = 0.0f;
                        potEnemyShouldJump[potEnemyCount] = false;
                        potEnemyStableFrames[potEnemyCount] = 0;
                        potEnemyIsVisible[potEnemyCount] = true; // Invisible man starts visible
                        potEnemyVisibilityTimer[potEnemyCount] = 0.0f;
                        potEnemyTeleportTimer[potEnemyCount] = 0.0f;
                        potEnemyShootTimer[potEnemyCount] = 0.0f;
                        potEnemyIsShooting[potEnemyCount] = false;
                        potEnemyCount++;

                        cout << "Pot spawned enemy type " << enemyTypeToSpawn << "! Total pot enemies: " << potEnemyCount << endl;
                    }

                    // --- POT COLLISION WITH PLAYER ---
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

                // --- POT DESTRUCTION ANIMATION ---
                if (potDestroyed && !bossAppeared) // after pot destroyed, spawn octopus
                {
                    potDestroyTimer += dt;

                    // After 1.5 seconds, boss appears
                    if (potDestroyTimer >= 1.5f)
                    {
                        bossAppeared = true;
                        cloudIsPlatform = true; // Cloud becomes a platform
                        bossX = screen_x / 2 - bossWidth / 2;
                        bossY = 50; // Boss appears at top
                        cout << "Boss (octopus) has appeared" << endl;
                    }
                }

                // --- UPDATE POT ENEMIES (gravity, movement, and type-specific behaviors) ---
                int floorRowForEnemies = level3Height - 2;
                float floorYForEnemies = floorRowForEnemies * level3CellSize;

                for (int pe = 0; pe < potEnemyCount; pe++)
                {
                    if (potEnemyIsCaught[pe])
                    {
                        capturedCount++;
                        continue;
                    }

                    // Get enemy dimensions based on type
                    int enemyH = 60, enemyW = 72; // Default ghost
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

                    // ============================================================
                    // TYPE-SPECIFIC BEHAVIORS
                    // ============================================================

                    // --- SKELETON JUMPING BEHAVIOR (Type 2) ---
                    if (potEnemyType[pe] == 2)
                    {
                        if (potEnemyOnGround[pe])
                            potEnemyStableFrames[pe]++;
                        else
                            potEnemyStableFrames[pe] = 0;

                        potEnemyJumpTimer[pe] += dt;

                        // Random chance to jump when stable on ground
                        if (potEnemyOnGround[pe] && potEnemyStableFrames[pe] > 60 && !potEnemyShouldJump[pe])
                        {
                            if (rand() % 100 < 2) // 2% chance each frame
                            {
                                potEnemyShouldJump[pe] = true;
                                potEnemyJumpTimer[pe] = 0.0f;
                            }
                        }

                        // Execute jump
                        if (potEnemyShouldJump[pe] && potEnemyOnGround[pe] && potEnemyStableFrames[pe] > 10)
                        {
                            potEnemyVelocityY[pe] = jumpStrength;
                            potEnemyOnGround[pe] = false;
                            potEnemyShouldJump[pe] = false;
                            potEnemyStableFrames[pe] = 0;
                        }
                    }

                    // --- INVISIBLE MAN BEHAVIOR (Type 3) ---
                    if (potEnemyType[pe] == 3)
                    {
                        // Toggle visibility every 3 seconds
                        potEnemyVisibilityTimer[pe] += dt;
                        if (potEnemyVisibilityTimer[pe] >= 3.0f)
                        {
                            potEnemyVisibilityTimer[pe] = 0.0f;
                            potEnemyIsVisible[pe] = !potEnemyIsVisible[pe];
                        }

                        // Teleport every 5 seconds
                        potEnemyTeleportTimer[pe] += dt;
                        if (potEnemyTeleportTimer[pe] >= 5.0f)
                        {
                            potEnemyTeleportTimer[pe] = 0.0f;
                            // Teleport to random position on floor
                            potEnemiesX[pe] = level3CellSize + (rand() % (screen_x - 2 * level3CellSize - enemyW));
                            potEnemiesY[pe] = floorYForEnemies - enemyH - 50;
                            potEnemyVelocityY[pe] = 0;
                        }
                    }

                    // --- CHELNOV SHOOTING BEHAVIOR (Type 4) ---
                    if (potEnemyType[pe] == 4)
                    {
                        potEnemyShootTimer[pe] += dt;

                        // Shoot every 4 seconds
                        if (potEnemyShootTimer[pe] >= 4.0f)
                        {
                            potEnemyShootTimer[pe] = 0.0f;
                            potEnemyIsShooting[pe] = true;

                            // Spawn projectile
                            if (potEnemyProjCount < maxPotEnemyProjectiles)
                            {
                                potEnemyProjX[potEnemyProjCount] = potEnemiesX[pe];
                                potEnemyProjY[potEnemyProjCount] = potEnemiesY[pe] + enemyH / 2;
                                potEnemyProjDirection[potEnemyProjCount] = potEnemyDirection[pe];
                                potEnemyProjActive[potEnemyProjCount] = true;
                                potEnemyProjCount++;
                            }
                        }

                        // Reset shooting state after 0.5 seconds
                        if (potEnemyIsShooting[pe])
                        {
                            if (potEnemyShootTimer[pe] >= 0.5f)
                                potEnemyIsShooting[pe] = false;
                        }
                    }

                    // ============================================================
                    // PHYSICS (gravity and floor collision)
                    // ============================================================

                    // Apply gravity
                    potEnemyVelocityY[pe] += gravity * dt;
                    if (potEnemyVelocityY[pe] > terminal_Velocity)
                        potEnemyVelocityY[pe] = terminal_Velocity;

                    float newY = potEnemiesY[pe] + potEnemyVelocityY[pe] * dt;

                    // Floor collision
                    if (newY + enemyH >= floorYForEnemies)
                    {
                        newY = floorYForEnemies - enemyH;
                        potEnemyVelocityY[pe] = 0;
                        potEnemyOnGround[pe] = true;
                    }
                    else
                    {
                        potEnemyOnGround[pe] = false;
                    }

                    // Cloud platform collision (when cloud is a platform)
                    if (cloudIsPlatform && !potEnemyOnGround[pe])
                    {
                        if (potEnemiesX[pe] + enemyW > cloudX && potEnemiesX[pe] < cloudX + cloudWidth)
                        {
                            if (newY + enemyH >= cloudY && potEnemiesY[pe] + enemyH <= cloudY + 10)
                            {
                                newY = cloudY - enemyH;
                                potEnemyVelocityY[pe] = 0;
                                potEnemyOnGround[pe] = true;
                            }
                        }
                    }

                    potEnemiesY[pe] = newY;

                    // Horizontal movement when on ground
                    if (potEnemyOnGround[pe])
                    {
                        potEnemiesX[pe] += potEnemySpeed[pe] * potEnemyDirection[pe] * dt;

                        // Bounce off walls
                        if (potEnemiesX[pe] <= level3CellSize)
                        {
                            potEnemiesX[pe] = level3CellSize;
                            potEnemyDirection[pe] = 1;
                        }
                        else if (potEnemiesX[pe] + enemyW >= screen_x - level3CellSize)
                        {
                            potEnemiesX[pe] = screen_x - level3CellSize - enemyW;
                            potEnemyDirection[pe] = -1;
                        }
                    }

                    // --- POT ENEMY COLLISION WITH PLAYER ---
                    // Invisible man only damages when visible
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

                // ============================================================================
                // UPDATE POT ENEMY PROJECTILES (Chelnov fireballs)
                // ============================================================================
                for (int pp = 0; pp < potEnemyProjCount; pp++)
                {
                    if (!potEnemyProjActive[pp])
                        continue;

                    // Move projectile
                    potEnemyProjX[pp] += 150.0f * potEnemyProjDirection[pp] * dt;

                    // Remove if off screen
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

                    // Collision with player
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

                            // Remove projectile
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

                // ============================================================================
                // CLOUD PLATFORM COLLISION FOR PLAYER (after pot is destroyed)
                // ============================================================================
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

                // ============================================================================
                // PHASE 2: BOSS LOGIC (after pot is destroyed and boss has appeared)
                // ============================================================================
                if (bossAppeared && !bossDefeated)
                {
                    // Update spawn timers
                    minionSpawnTimer += dt;
                    tentacleSpawnTimer += dt;

                    // Check if boss is angry (health <= 2)
                    if (bossHealth <= 2 && !bossIsAngry)
                    {
                        bossIsAngry = true;
                        minionSpawnInterval = 2.0f; // Spawn faster when angry

                        // Make all existing minions follow player
                        for (int m = 0; m < minionCount; m++)
                        {
                            minionFollowingPlayer[m] = true;
                        }

                        cout << "BOSS IS ANGRY! Minions will now follow the player!" << endl;
                    }

                    // --- SPAWN MINIONS (from boss) ---
                    if (minionSpawnTimer >= minionSpawnInterval && minionCount < maxMinions)
                    // Inside: if (minionSpawnTimer >= minionSpawnInterval && minionCount < maxMinions)
                    {
                        minionSpawnTimer = 0.0f;

                        int minionsToSpawn = 1 + rand() % 3;
                        if (bossIsAngry)
                            minionsToSpawn += 1;

                        for (int m = 0; m < minionsToSpawn && minionCount < maxMinions; m++)
                        {
                            // === PHASE 2 DYNAMIC RESIZING (Exact Fit)  ===
                            // 1. Allocate new arrays of size + 1
                            int newSize = minionCount + 1;
                            float *newX = new float[newSize];
                            float *newY = new float[newSize];
                            float *newSpeed = new float[newSize];
                            int *newDir = new int[newSize];
                            float *newVelY = new float[newSize];
                            bool *newGround = new bool[newSize];
                            bool *newCaught = new bool[newSize];
                            bool *newFollow = new bool[newSize];

                            // 2. Copy existing data
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

                            // 3. Delete old arrays (Free memory)
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

                            // 4. Point to new arrays
                            minionsX = newX;
                            minionsY = newY;
                            minionSpeed = newSpeed;
                            minionDirection = newDir;
                            minionVelocityY = newVelY;
                            minionOnGround = newGround;
                            minionIsCaught = newCaught;
                            minionFollowingPlayer = newFollow;

                            // 5. Add new minion at the end
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

                    // --- SPAWN TENTACLES ---
                    if (tentacleSpawnTimer >= tentacleSpawnInterval)
                    {
                        tentacleSpawnTimer = 0.0f;

                        // 60% chance to spawn a tentacle
                        if (rand() % 100 < 60 && tentacleCount < maxTentacles)
                        {
                            // === PHASE 2 DYNAMIC RESIZING (Exact Fit) [cite: 23, 43] ===
                            // 1. Allocate new arrays of size + 1
                            int newSize = tentacleCount + 1;
                            float *newX = new float[newSize];
                            float *newY = new float[newSize];
                            int *newW = new int[newSize];
                            int *newH = new int[newSize];
                            float *newTimer = new float[newSize];
                            float *newDur = new float[newSize];
                            bool *newActive = new bool[newSize];

                            // 2. Copy existing data
                            for (int i = 0; i < tentacleCount; i++)
                            {
                                newX[i] = tentaclesX[i];
                                newY[i] = tentaclesY[i];
                                newW[i] = tentacleWidth[i];
                                newH[i] = tentacleHeight[i];
                                newTimer[i] = tentacleTimer[i];
                                newDur[i] = tentacleDuration[i];
                                newActive[i] = tentacleActive[i];
                            }

                            // 3. Delete old arrays (Free memory)
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

                            // 4. Point to new arrays
                            tentaclesX = newX;
                            tentaclesY = newY;
                            tentacleWidth = newW;
                            tentacleHeight = newH;
                            tentacleTimer = newTimer;
                            tentacleDuration = newDur;
                            tentacleActive = newActive;

                            // 5. Add new tentacle data at the end [cite: 42]
                            tentaclesX[tentacleCount] = 100 + rand() % (screen_x - 200);
                            tentaclesY[tentacleCount] = 200 + rand() % (screen_y - 400);
                            tentacleWidth[tentacleCount] = 40 + rand() % 30;
                            tentacleHeight[tentacleCount] = 100 + rand() % 80;
                            tentacleTimer[tentacleCount] = 0.0f;
                            tentacleDuration[tentacleCount] = 2.0f + (rand() % 40) / 10.0f; // 2-6 seconds
                            tentacleActive[tentacleCount] = true;

                            tentacleCount++;
                            cout << "Tentacle spawned (Dynamic)! Total: " << tentacleCount << endl;
                        }
                    }

                    // --- UPDATE TENTACLES ---
                    for (int t = 0; t < tentacleCount; t++)
                    {
                        if (tentacleActive[t])
                        {
                            tentacleTimer[t] += dt;

                            // Remove tentacle when duration expires
                            if (tentacleTimer[t] >= tentacleDuration[t])
                            {
                                // Shift remaining tentacles
                                for (int j = t; j < tentacleCount - 1; j++)
                                {
                                    tentaclesX[j] = tentaclesX[j + 1];
                                    tentaclesY[j] = tentaclesY[j + 1];
                                    tentacleWidth[j] = tentacleWidth[j + 1];
                                    tentacleHeight[j] = tentacleHeight[j + 1];
                                    tentacleTimer[j] = tentacleTimer[j + 1];
                                    tentacleDuration[j] = tentacleDuration[j + 1];
                                    tentacleActive[j] = tentacleActive[j + 1];
                                }
                                tentacleCount--;
                                t--;
                            }
                        }
                    }

                    // --- UPDATE MINIONS ---
                    int minionFloorRow = level3Height - 2;
                    float minionFloorY = minionFloorRow * level3CellSize;

                    for (int m = 0; m < minionCount; m++)
                    {
                        if (minionIsCaught[m])
                            continue;

                        // Apply gravity
                        minionVelocityY[m] += gravity * dt;
                        if (minionVelocityY[m] > terminal_Velocity)
                            minionVelocityY[m] = terminal_Velocity;

                        float newMinionY = minionsY[m] + minionVelocityY[m] * dt;

                        // Floor collision
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

                        // Horizontal movement
                        if (minionFollowingPlayer[m] && minionOnGround[m])
                        {
                            // Follow player when angry
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
                            // Normal patrol movement
                            minionsX[m] += minionSpeed[m] * minionDirection[m] * dt;

                            // Bounce off walls
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

                        // --- MINION COLLISION WITH PLAYER ---
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

                    // --- TENTACLE COLLISION WITH PLAYER ---
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

                    // --- BOSS COLLISION WITH PLAYER ---
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
            // ============================================================================
            // END OF BOSS LEVEL GAME LOGIC
            // ============================================================================

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
            for (int i = 0; i < maxEnemyCount; i++)
                enemyIsCaught[i] = false;
            for (int i = 0; i < maxSkeletonCount; i++)
                skeletonIsCaught[i] = false;
            for (int i = 0; i < maxInvisibleCount; i++)
                invisibleIsCaught[i] = false;
            for (int i = 0; i < maxChelnovCount; i++)
                chelnovIsCaught[i] = false;

            // Vacuum input
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

            // Release direction
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
            // --- REPLACE THE EXISTING 'E' KEY BLOCK WITH THIS ---

            if (Keyboard::isKeyPressed(Keyboard::E) && !eKeyPressed)
            {
                eKeyPressed = true;

                // === PHASE 2 SHOOTING (Level 3+) ===
                if (currentLevel >= 3)
                {
                    if (dynamicCapturedCount > 0)
                    {
                        // 1. Get the last enemy captured (LIFO)
                        int enemyTypeToRelease = dynamicCapturedEnemies[dynamicCapturedCount - 1];

                        // 2. Standard Projectile Spawn Logic
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

                        // 3. DYNAMIC SHRINKING (Waste no memory)
                        dynamicCapturedCount--;

                        if (dynamicCapturedCount == 0)
                        {
                            delete[] dynamicCapturedEnemies;
                            dynamicCapturedEnemies = nullptr; // Completely free memory
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
                // === PHASE 1 SHOOTING (Levels 1 & 2) ===
                else
                {
                    if (capturedCount > 0 && projectileCount < MAX_PROJECTILES)
                    {
                        capturedCount--;
                        int enemyTypeToRelease = capturedEnemies[capturedCount];
                        // ... (Copy your existing Phase 1 projectile spawn logic here) ...
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

            // Vacuum Burst - R key
            static bool rKeyPressed = false;
            if (Keyboard::isKeyPressed(Keyboard::R) && !rKeyPressed)
            {
                rKeyPressed = true;
                if (capturedCount > 0 && !burstModeActive)
                {
                    if (capturedCount >= 3)
                        playerScore += 300;
                    burstModeActive = true;
                    burstFrameCounter = 0;
                    burstReleaseDirection = releaseDirection;
                    burstPlayerFacing = facing;
                }
            }
            if (!Keyboard::isKeyPressed(Keyboard::R))
                rKeyPressed = false;

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

            // Boss Level (Level 3) vacuum handling for minions
            if (currentLevel == 3 && minionCount > 0)
            {
                // Use Phase 2 function (Dynamic Pointer)
                handleVacuumPhase2(window, vacSprite, vacTexHorz, vacTexVert,
                                   player_x, player_y, PlayerWidth, PlayerHeight, vacDirection, isVacuuming,
                                   minionsX, minionsY, minionCount,
                                   dynamicCapturedEnemies, dynamicCapturedCount, // <-- Dynamic vars
                                   5,                                            // Minion Type
                                   vacFlickerTimer, showVacSprite, dt, minionIsCaught, false, vacuumPower,
                                   playerScore, comboStreak, comboTimer, multiKillCount, multiKillTimer, hasRangeBoost);
            }

            // ============================================================================
            // BOSS LEVEL (LEVEL 3) VACUUM HANDLING FOR POT ENEMIES
            // Custom inline vacuum check since pot enemies have different types
            // ============================================================================
            // ============================================================================
            // BOSS LEVEL (LEVEL 3) VACUUM HANDLING FOR POT ENEMIES
            // CORRECTED LOGIC
            // ============================================================================
            if (currentLevel == 3 && potEnemyCount > 0 && isVacuuming)
            {

                float pCenterX = player_x + PlayerWidth / 2;
                float pCenterY = player_y + PlayerHeight / 2;
                float beamReach = hasRangeBoost ? 180.0f : 120.0f;
                float beamThick = 60.0f;

                // Calculate vacuum hitbox based on direction
                float vacLeft, vacTop, vacRight, vacBottom;
                if (vacDirection == 0) // RIGHT
                {
                    vacLeft = player_x + PlayerWidth;
                    vacTop = pCenterY - beamThick;
                    vacRight = vacLeft + beamReach * 2;
                    vacBottom = pCenterY + beamThick;
                }
                else if (vacDirection == 1) // LEFT
                {
                    vacRight = player_x;
                    vacTop = pCenterY - beamThick;
                    vacLeft = vacRight - beamReach * 2;
                    vacBottom = pCenterY + beamThick;
                }
                else if (vacDirection == 2) // UP
                {
                    vacLeft = pCenterX - beamThick;
                    vacBottom = player_y;
                    vacRight = pCenterX + beamThick;
                    vacTop = vacBottom - beamReach * 2;
                }
                else // DOWN
                {
                    vacLeft = pCenterX - beamThick;
                    vacTop = player_y + PlayerHeight;
                    vacRight = pCenterX + beamThick;
                    vacBottom = vacTop + beamReach * 2;
                }

                for (int pe = 0; pe < potEnemyCount; pe++)
                {
                    // FIX 1: Reset the caught flag at start of frame so logic runs continuously
                    potEnemyIsCaught[pe] = false;

                    // Get enemy dimensions based on type
                    int enemyW = 72, enemyH = 60;
                    if (potEnemyType[pe] == 2)
                    {
                        enemyW = 72;
                        enemyH = 92;
                    }
                    else if (potEnemyType[pe] == 3)
                    {
                        enemyW = 60;
                        enemyH = 80;
                    }
                    else if (potEnemyType[pe] == 4)
                    {
                        enemyW = 60;
                        enemyH = 90;
                    }

                    // Check if enemy is in vacuum hitbox
                    if (potEnemiesX[pe] + enemyW > vacLeft && potEnemiesX[pe] < vacRight &&
                        potEnemiesY[pe] + enemyH > vacTop && potEnemiesY[pe] < vacBottom)
                    {
                        potEnemyIsCaught[pe] = true;

                        // Pull towards player
                        float pullSpeed = 250.0f * vacuumPower * dt;
                        if (potEnemiesX[pe] < pCenterX)
                            potEnemiesX[pe] += pullSpeed;
                        else
                            potEnemiesX[pe] -= pullSpeed;
                        if (potEnemiesY[pe] < pCenterY)
                            potEnemiesY[pe] += pullSpeed;
                        else
                            potEnemiesY[pe] -= pullSpeed;

                        // Check if close enough to capture
                        float dx = pCenterX - (potEnemiesX[pe] + enemyW / 2);
                        float dy = pCenterY - (potEnemiesY[pe] + enemyH / 2);
                        float dist = sqrt(dx * dx + dy * dy);

                        // FIX 2: Check MAX_CAPTURED_ARRAY (5) to prevent memory corruption!
                        // REPLACE THE OLD "if (dist < ...)" BLOCK WITH THIS:

                        if (dist < 50.0f * vacuumPower) // REMOVED THE LIMIT CAP
                        {
                            // === 1. PHASE 2 DYNAMIC CAPTURE ===
                            // Resize the dynamic array to fit exactly one more enemy
                            int *newArr = new int[dynamicCapturedCount + 1];

                            // Copy existing enemies
                            for (int k = 0; k < dynamicCapturedCount; k++)
                            {
                                newArr[k] = dynamicCapturedEnemies[k];
                            }

                            // Add the new enemy (Preserving its specific type!)
                            newArr[dynamicCapturedCount] = potEnemyType[pe];

                            // Delete old array and update pointer
                            if (dynamicCapturedEnemies != NULL)
                                delete[] dynamicCapturedEnemies;
                            dynamicCapturedEnemies = newArr;
                            dynamicCapturedCount++; // Update the Phase 2 counter

                            // === 2. SCORING ===
                            int capturePoints = 50;
                            if (potEnemyType[pe] == 2)
                                capturePoints = 75;
                            else if (potEnemyType[pe] == 3)
                                capturePoints = 150;
                            else if (potEnemyType[pe] == 4)
                                capturePoints = 200;

                            addScore(playerScore, comboStreak, comboTimer, capturePoints, false, multiKillCount, multiKillTimer, dt);

                            // === 3. REMOVE ENEMY (CRITICAL: KEEP THIS SHIFTING LOGIC) ===
                            // This part you already had is correct and necessary for Pot Enemies
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

                                // Behavior arrays (These would break if you used the generic function)
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
                            pe--; // Adjust index since we removed an enemy

                            cout << "Captured pot enemy! Total Dynamic: " << dynamicCapturedCount << endl;
                        }
                    }
                }
            }
            // ============================================================================
            // END OF POT ENEMY VACUUM HANDLING
            // ============================================================================

            // Powerup collection
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

            // Player collision and physics - use appropriate level map
            if (currentLevel == 3)
            {
                // Boss Level uses bossLvl map with different dimensions
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

            // Player gravity - use appropriate level map
            if (currentLevel == 3)
            {
                player_gravity(bossLvl, offset_y, velocityY, onGround, gravity, terminal_Velocity, player_x, player_y, level3CellSize, PlayerHeight, PlayerWidth, level3Height, level3Width, dt);
            }
            else
            {
                player_gravity(lvl, offset_y, velocityY, onGround, gravity, terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth, height, width, dt);
            }

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

                // Use appropriate level map and dimensions for projectile physics
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

                // Collision with ghosts
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

                // Collision with skeletons
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

                // ============================================================================
                // BOSS LEVEL (LEVEL 3) PROJECTILE COLLISIONS
                // Phase 2: First check pot collision, then boss collision (if pot destroyed)
                // ============================================================================
                if (currentLevel == 3)
                {
                    bool projectileRemoved = false;

                    // --- PROJECTILE COLLISION WITH POT (Phase 1 - before boss appears) ---
                    if (potActive && !potDestroyed && !projectileRemoved)
                    {
                        if ((projectilesX[p] < potX + potWidth) &&
                            (projectilesX[p] + ProjectileWidth > potX) &&
                            (projectilesY[p] < potY + potHeight) &&
                            (projectilesY[p] + ProjectileHeight > potY))
                        {
                            // HIT THE POT!
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

                            // Remove the projectile
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

                    // --- PROJECTILE COLLISION WITH POT ENEMIES ---
                    if (!projectileRemoved)
                    {
                        for (int pe = 0; pe < potEnemyCount && !projectileRemoved; pe++)
                        {
                            if (potEnemyIsCaught[pe])
                                continue;

                            // Get enemy dimensions based on type
                            int enemyW = 72, enemyH = 60; // Default ghost
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
                                // Defeat pot enemy - points based on type
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

                                // Remove pot enemy by shifting array (dynamic resize down)
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

                    // --- PROJECTILE COLLISION WITH BOSS HEAD (Phase 2 - after boss appears) ---
                    if (bossAppeared && !bossDefeated && !projectileRemoved)
                    {
                        // Only hits the upper portion of the boss (the head)
                        float bossHeadY = bossY;
                        float bossHeadHeight = bossHeight * 0.5f; // Head is top half

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

                            // Remove the projectile
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

                    // --- PROJECTILE COLLISION WITH TENTACLES ---
                    // When projectile hits tentacle, minion is REBORN at that spot
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
                                // MINION REBORN AT THIS SPOT!
                                if (minionCount < maxMinions)
                                {
                                    // === DYNAMIC RESIZING FOR REBIRTH ===
                                    // 1. Allocate new arrays of size + 1
                                    int newSize = minionCount + 1;
                                    float *newX = new float[newSize];
                                    float *newY = new float[newSize];
                                    float *newSpeed = new float[newSize];
                                    int *newDir = new int[newSize];
                                    float *newVelY = new float[newSize];
                                    bool *newGround = new bool[newSize];
                                    bool *newCaught = new bool[newSize];
                                    bool *newFollow = new bool[newSize];

                                    // 2. Copy existing data
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

                                    // 3. Delete old arrays
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

                                    // 4. Point to new arrays
                                    minionsX = newX;
                                    minionsY = newY;
                                    minionSpeed = newSpeed;
                                    minionDirection = newDir;
                                    minionVelocityY = newVelY;
                                    minionOnGround = newGround;
                                    minionIsCaught = newCaught;
                                    minionFollowingPlayer = newFollow;

                                    // 5. Add new minion at the projectile's location
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

                                // Remove the projectile
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

                    // --- PROJECTILE COLLISION WITH MINIONS (defeats them) ---
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
                // ============================================================================
                // END OF BOSS LEVEL PROJECTILE COLLISIONS
                // ============================================================================
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
            else if (currentLevel == 3)
            {
                // Boss level complete when boss is defeated
                allEnemiesDefeated = bossDefeated;
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

                                // MODIFIED: Enable wave spawning
                                useWaveSpawning = true;
                                currentWave = 0;
                                waveTimer = 0.0f;
                                for (int i = 0; i < 4; i++)
                                    waveSpawned[i] = false;

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
                                // TRANSITION TO BOSS LEVEL (LEVEL 3)
                                currentLevel = 3;

                                // Allocate dynamic arrays for minions
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

                                // Allocate dynamic arrays for tentacles
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

                                // Create boss level map
                                bossLvl = new char *[level3Height];
                                for (int i = 0; i < level3Height; i++)
                                {
                                    bossLvl[i] = new char[level3Width];
                                    for (int j = 0; j < level3Width; j++)
                                        bossLvl[i][j] = ' ';
                                }

                                // Boss level layout
                                for (int j = 0; j < level3Width; j++)
                                    bossLvl[level3Height - 3][j] = '#';

                                for (int i = 0; i < level3Height - 3; i++)
                                {
                                    bossLvl[i][0] = '#';
                                    bossLvl[i][level3Width - 1] = '#';
                                }

                                // Platforms
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

                                // Initialize boss
                                bossX = screen_x / 2 - bossWidth / 2;
                                bossY = 50;
                                bossHealth = maxBossHealth;
                                bossIsAngry = false;
                                bossDefeated = false;

                                minionSpawnTimer = 0.0f;
                                tentacleSpawnTimer = 0.0f;

                                // Player position
                                player_x = screen_x / 2 - PlayerWidth / 2;
                                player_y = (level3Height - 4) * level3CellSize;
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
                                // GAME COMPLETE - Boss defeated!
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

            if (restartGame)
                break;

            // Rendering
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
                // ============================================================================
                // BOSS LEVEL RENDERING
                // Phase 2: Draw Pot/Cloud first, then Boss (only if pot destroyed)
                // ============================================================================

                // Draw background
                window.draw(bgSprite3);

                // Draw boss level tiles
                jumpStrength = -180.f;
                for (int i = 0; i < level3Height; i++)
                {
                    for (int j = 0; j < level3Width; j++)
                    {
                        if (bossLvl[i][j] == '#' || bossLvl[i][j] == '-')
                        {
                            blockSprite3.setPosition(j * level3CellSize, i * level3CellSize);
                            // Scale block to fit smaller cell size
                            float blockScale = (float)level3CellSize / 64.0f;
                            blockSprite3.setScale(blockScale, blockScale);
                            window.draw(blockSprite3);
                        }
                    }
                }

                // ============================================================================
                // DRAW CLOUD (always visible - becomes platform after pot destroyed)
                // ============================================================================
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

                // ============================================================================
                // DRAW POT (only if not destroyed)
                // ============================================================================
                if (potActive && !potDestroyed)
                {
                    potSprite.setPosition(potX, potY);
                    window.draw(potSprite);

                    // Draw Pot Health Bar
                    RectangleShape potHealthBarBG;
                    potHealthBarBG.setSize(Vector2f(100, 15));
                    potHealthBarBG.setPosition(potX + potWidth / 2 - 50, potY - 25);
                    potHealthBarBG.setFillColor(Color(50, 50, 50));
                    window.draw(potHealthBarBG);

                    RectangleShape potHealthBar;
                    float potHealthPercent = (float)potHealth / potMaxHealth;
                    potHealthBar.setSize(Vector2f(100 * potHealthPercent, 15));
                    potHealthBar.setPosition(potX + potWidth / 2 - 50, potY - 25);
                    potHealthBar.setFillColor(Color(255, 165, 0)); // Orange
                    window.draw(potHealthBar);

                    // Draw pot hitbox (debug)
                    RectangleShape potBox;
                    potBox.setSize(Vector2f(potWidth, potHeight));
                    potBox.setPosition(potX, potY);
                    potBox.setFillColor(Color::Transparent);
                    potBox.setOutlineColor(Color::Yellow);
                    potBox.setOutlineThickness(2);
                    window.draw(potBox);
                }

                // Draw pot destruction effect
                if (potDestroyed && !bossAppeared)
                {
                    // Flash effect during destruction
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

                // ============================================================================
                // DRAW POT ENEMIES (ghost, skeleton, invisible, chelnov spawned by pot)
                // ============================================================================
                for (int pe = 0; pe < potEnemyCount; pe++)
                {
                    if (potEnemyIsCaught[pe])
                        continue;

                    // Draw enemy based on type
                    int texW, texH;
                    switch (potEnemyType[pe])
                    {
                    case 1: // Ghost
                        EnemySprite.setTexture(EnemyTexture);
                        EnemySprite.setPosition(potEnemiesX[pe], potEnemiesY[pe]);
                        texW = EnemyTexture.getSize().x;
                        texH = EnemyTexture.getSize().y;
                        if (potEnemyDirection[pe] == 1)
                            EnemySprite.setTextureRect(IntRect(texW, 0, -texW, texH));
                        else
                            EnemySprite.setTextureRect(IntRect(0, 0, texW, texH));
                        window.draw(EnemySprite);
                        break;

                    case 2: // Skeleton
                        SkeletonSprite.setTexture(SkeletonTexture);
                        SkeletonSprite.setPosition(potEnemiesX[pe], potEnemiesY[pe]);
                        texW = SkeletonTexture.getSize().x;
                        texH = SkeletonTexture.getSize().y;
                        if (potEnemyDirection[pe] == 1)
                            SkeletonSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
                        else
                            SkeletonSprite.setTextureRect(IntRect(0, 0, texW, texH));
                        window.draw(SkeletonSprite);
                        break;

                    case 3: // Invisible Man - only draw when visible
                        if (potEnemyIsVisible[pe])
                        {
                            InvisibleSprite.setTexture(InvisibleTexture);
                            InvisibleSprite.setPosition(potEnemiesX[pe], potEnemiesY[pe]);
                            texW = InvisibleTexture.getSize().x;
                            texH = InvisibleTexture.getSize().y;
                            if (potEnemyDirection[pe] == 1)
                                InvisibleSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
                            else
                                InvisibleSprite.setTextureRect(IntRect(0, 0, texW, texH));
                            window.draw(InvisibleSprite);
                        }
                        break;
                        ;
                        break;

                    case 4: // Chelnov
                        ChelnovSprite.setTexture(ChelnovTexture);
                        ChelnovSprite.setPosition(potEnemiesX[pe], potEnemiesY[pe]);
                        texW = ChelnovTexture.getSize().x;
                        texH = ChelnovTexture.getSize().y;
                        if (potEnemyDirection[pe] == 1)
                            ChelnovSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
                        else
                            ChelnovSprite.setTextureRect(IntRect(0, 0, texW, texH));
                        window.draw(ChelnovSprite);
                        break;
                    }
                }

                // ============================================================================
                // DRAW POT ENEMY PROJECTILES (Chelnov fireballs)
                // ============================================================================
                for (int pp = 0; pp < potEnemyProjCount; pp++)
                {
                    if (!potEnemyProjActive[pp])
                        continue;

                    chelnovProjSprite.setPosition(potEnemyProjX[pp], potEnemyProjY[pp]);
                    window.draw(chelnovProjSprite);
                }

                // ============================================================================
                // DRAW BOSS (Octopus) - only after pot is destroyed and boss has appeared
                // ============================================================================
                if (bossAppeared && !bossDefeated)
                {
                    if (bossIsAngry)
                    {
                        // Try to use angry texture, or tint red if not available
                        if (bossAngryTexture.getSize().x > 0)
                            bossSprite.setTexture(bossAngryTexture);
                    }
                    else
                    {
                        bossSprite.setTexture(bossTexture);
                    }
                    bossSprite.setPosition(bossX, bossY);
                    window.draw(bossSprite);

                    // Draw Boss Health Bar
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

                    // Draw Boss Health Text
                    Text bossHealthText("BOSS: " + to_string(bossHealth) + "/" + to_string(maxBossHealth), font, 20);
                    bossHealthText.setFillColor(Color::White);
                    bossHealthText.setPosition(screen_x / 2 - 50, 45);
                    window.draw(bossHealthText);
                }

                // Draw Tentacles (only when boss has appeared)
                if (bossAppeared)
                {
                    for (int t = 0; t < tentacleCount; t++)
                    {
                        if (!tentacleActive[t])
                            continue;

                        // Scale tentacle sprite
                        float tentScaleX = (float)tentacleWidth[t] / tentacleTexture.getSize().x;
                        float tentScaleY = (float)tentacleHeight[t] / tentacleTexture.getSize().y;
                        tentacleSprite.setScale(tentScaleX, tentScaleY);
                        tentacleSprite.setPosition(tentaclesX[t], tentaclesY[t]);
                        window.draw(tentacleSprite);

                        // Draw tentacle hitbox (debug)
                        RectangleShape tentBox;
                        tentBox.setSize(Vector2f(tentacleWidth[t], tentacleHeight[t]));
                        tentBox.setPosition(tentaclesX[t], tentaclesY[t]);
                        tentBox.setFillColor(Color::Transparent);
                        tentBox.setOutlineColor(Color::Magenta);
                        tentBox.setOutlineThickness(1);
                        window.draw(tentBox);
                    }

                    // Draw Minions (spawned by boss)
                    for (int m = 0; m < minionCount; m++)
                    {
                        if (minionIsCaught[m])
                            continue;

                        minionSprite.setPosition(minionsX[m], minionsY[m]);

                        // Flip sprite based on direction
                        int texW = minionTexture.getSize().x;
                        int texH = minionTexture.getSize().y;
                        if (minionDirection[m] == -1)
                            minionSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
                        else
                            minionSprite.setTextureRect(IntRect(0, 0, texW, texH));

                        window.draw(minionSprite);
                    }
                }

                // Draw "ANGRY!" warning when boss is angry
                if (bossAppeared && bossIsAngry && !bossDefeated)
                {
                    Text angryText("!! BOSS ANGRY - MINIONS FOLLOWING !!", font, 30);
                    angryText.setFillColor(Color::Red);
                    angryText.setPosition(screen_x / 2 - 250, 80);
                    window.draw(angryText);
                }

                // Draw phase instruction text
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

            // Draw ghosts
            for (int i = 0; i < enemyCount; i++)
            {
                EnemySprite.setPosition(enemiesX[i], enemiesY[i]);
                window.draw(EnemySprite);
            }

            // Draw skeletons
            for (int i = 0; i < skeletonCount; i++)
            {
                skeletonAnimCounter[i]++;
                if (skeletonAnimCounter[i] >= skeletonAnimSpeed)
                {
                    skeletonAnimCounter[i] = 0;
                    skeletonAnimFrame[i]++;
                    if (skeletonAnimFrame[i] >= 4)
                        skeletonAnimFrame[i] = 0;
                }

                SkeletonSprite.setTexture(skeletonWalkTex[skeletonAnimFrame[i]], true);

                int texW = skeletonWalkTex[skeletonAnimFrame[i]].getSize().x;
                int texH = skeletonWalkTex[skeletonAnimFrame[i]].getSize().y;

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

            // Draw Level 2 enemies
            if (currentLevel == 2)
            {
                for (int i = 0; i < invisibleCount; i++)
                {
                    if (invisibleIsVisible[i])
                    {
                        InvisibleSprite.setPosition(invisiblesX[i], invisiblesY[i]);
                        window.draw(InvisibleSprite);
                    }
                }

                for (int i = 0; i < chelnovCount; i++)
                {
                    ChelnovSprite.setPosition(chelnovsX[i], chelnovsY[i]);
                    int texW = ChelnovTexture.getSize().x;
                    int texH = ChelnovTexture.getSize().y;
                    if (chelnovDirection[i] == 1)
                        ChelnovSprite.setTextureRect(IntRect(texW, 0, -texW, texH));
                    else
                        ChelnovSprite.setTextureRect(IntRect(0, 0, texW, texH));
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
            if (currentLevel == 3) capturedCount = dynamicCapturedCount;
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

        } // End of game loop

        lvlMusic.stop();
        lvl2Music.stop();
        lvl3Music.stop();

        for (int i = 0; i < height; i++)
        {
            delete[] lvl[i];
        }
        delete[] lvl;

        // Cleanup Boss Level dynamic arrays
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

        // Cleanup pot enemy arrays (Phase 2 dynamic arrays)
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
        // Cleanup pot enemy behavior arrays
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

        // Cleanup dynamic captured enemies array (Phase 2)
        if (dynamicCapturedEnemies != NULL)
        {
            delete[] dynamicCapturedEnemies;
            dynamicCapturedEnemies = NULL;
        }

    } // End of playagain loop

    return 0;
}
