#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// srand(time(NULL)); // Seed with current time
float gameStartTime = 0.0f;
int touchCount = 0;
bool scoreCalculated = false;
int finalScore = 0;

typedef struct Sprite
{
    Texture2D texture;
    Rectangle dest_rect;
    Vector2 vel;
} Sprite;

typedef struct Obstacle
{
    Rectangle bounds;
    Vector2 velocity;
    bool is_deadly;
    bool is_bouncy;
    Color color;
} Obstacle;

typedef struct Achievement
{
    const char *name;
    const char *description;
    bool unlocked;
    Color color;
} Achievement;

#define MAX_OBSTACLES 5
#define MAX_ACHIEVEMENTS 3

Achievement achievements[MAX_ACHIEVEMENTS] = {
    {"Speed Runner", "Complete level in under 10 seconds", false, GREEN},
    {"Perfect Exit", "Exit through hole with score above 5000", false, GOLD},
    {"Obstacle Master", "Avoid all deadly obstacles", false, PURPLE}};

bool hasAvoidedDeadlyObstacles = true; // Track if player has hit any deadly obstacles

typedef enum GameScreen
{
    START_SCREEN = 0,
    SELECT_LEVEL_SCREEN,
    LEVEL1_SCREEN,
    LEVEL2_SCREEN
} GameScreen;

// Check if player passes through the win hole
bool check_win_condition(Sprite *player, Vector4 winhole)
{
    // Calculate player center
    float playerCenterX = player->dest_rect.x + player->dest_rect.width / 2.0f;
    float playerCenterY = player->dest_rect.y + player->dest_rect.height / 2.0f;
    // After studying draw_winhole, I see that winhole is a Vector4 where:
    // x,y = startpoint coordinates
    // z,w = endpoint coordinates

    // Determine if the win hole is horizontal or vertical by checking if its on a border
    // For horizontal (top or bottom borders)
    if ((winhole.y == 0 && winhole.w == 0) ||
        (winhole.y == GetScreenHeight() && winhole.w == GetScreenHeight()))
    {
        // Hole is on top or bottom edge
        // Check if player is between the X range of the hole
        bool inXRange = (playerCenterX >= fmin(winhole.x, winhole.z) &&
                         playerCenterX <= fmax(winhole.x, winhole.z));

        // Check if player is at the edge where the hole is
        if (winhole.y == 0) // Top edge
        {
            // Player is touching top edge
            return inXRange && (player->dest_rect.y <= 10.0f);
        }
        else // Bottom edge
        {
            // Player is touching bottom edge
            return inXRange && (player->dest_rect.y + player->dest_rect.height >= GetScreenHeight() - 10.0f);
        }
    }
    // For vertical holes (left or right borders)
    else if ((winhole.x == 0 && winhole.z == 0) ||
             (winhole.x == GetScreenWidth() && winhole.z == GetScreenWidth()))
    {
        // Hole is on left or right edge
        // Check if player is between the Y range of the hole
        bool inYRange = (playerCenterY >= fmin(winhole.y, winhole.w) &&
                         playerCenterY <= fmax(winhole.y, winhole.w));

        // Check if player is at the edge where the hole is
        if (winhole.x == 0) // Left edge
        {
            // Player is touching left edge
            return inYRange && (player->dest_rect.x <= 10.0f);
        }
        else // Right edge
        {
            // Player is touching right edge
            return inYRange && (player->dest_rect.x + player->dest_rect.width >= GetScreenWidth() - 10.0f);
        }
    }

    return false;
}
// void apply_velocity_nocol(Sprite *player)
// {
//     player->dest_rect.x += player->vel.x * GetFrameTime();
//     player->dest_rect.y += player->vel.y * GetFrameTime();
// }
void apply_velocity(Sprite *player, Vector4 winhole, bool *gameWon)
{
    // Get current position before movement
    float prevX = player->dest_rect.x;
    float prevY = player->dest_rect.y;
    // GetFrameTime() to have consistent movement rate whenever we have varying frame rates in game
    // apply velocity with frame time
    player->dest_rect.x += player->vel.x * GetFrameTime();
    player->dest_rect.y += player->vel.y * GetFrameTime();

    // Get player bounds
    float playerLeft = player->dest_rect.x;
    float playerRight = player->dest_rect.x + player->dest_rect.width;
    float playerTop = player->dest_rect.y;
    float playerBottom = player->dest_rect.y + player->dest_rect.height;

    // Check if player is passing through the win hole
    bool passingThroughHole = false;

    // For horizontal holes (top or bottom borders)
    if ((winhole.y == 0 && winhole.w == 0) ||
        (winhole.y == GetScreenHeight() && winhole.w == GetScreenHeight()))
    {
        // Hole is on top or bottom edge
        float holeLeft = fmin(winhole.x, winhole.z);
        float holeRight = fmax(winhole.x, winhole.z);

        // Check if player's horizontal position overlaps with hole
        if (playerRight >= holeLeft && playerLeft <= holeRight)
        {
            // Check if player is crossing top or bottom border
            if ((winhole.y == 0 && playerTop <= 0) ||
                (winhole.y == GetScreenHeight() && playerBottom >= GetScreenHeight()))
            {
                passingThroughHole = true;

                // If this is the first time passing through, set game as won
                if (!(*gameWon))
                {
                    *gameWon = true;
                }
            }
        }
    }
    // For vertical holes (left or right borders)
    else if ((winhole.x == 0 && winhole.z == 0) ||
             (winhole.x == GetScreenWidth() && winhole.z == GetScreenWidth()))
    {
        // Hole is on left or right edge
        float holeTop = fmin(winhole.y, winhole.w);
        float holeBottom = fmax(winhole.y, winhole.w);

        // Check if player's vertical position overlaps with hole
        if (playerBottom >= holeTop && playerTop <= holeBottom)
        {
            // Check if player is crossing left or right border
            if ((winhole.x == 0 && playerLeft <= 0) ||
                (winhole.x == GetScreenWidth() && playerRight >= GetScreenWidth()))
            {
                passingThroughHole = true;

                // If this is the first time passing through, set game as won
                if (!(*gameWon))
                {
                    *gameWon = true;
                }
            }
        }
    }

    // If not passing through hole, apply normal border collisions
    if (!passingThroughHole)
    {
        // Border collision checks
        if (player->dest_rect.x <= 0)
        {
            player->dest_rect.x = 0;
            player->vel.x *= -0.9f; // Bounce with some energy loss
        }
        if (player->dest_rect.x + player->dest_rect.width >= GetScreenWidth())
        {
            player->dest_rect.x = GetScreenWidth() - player->dest_rect.width;
            player->vel.x *= -0.9f;
        }
        if (player->dest_rect.y <= 0)
        {
            player->dest_rect.y = 0;
            player->vel.y *= -0.9f;
        }
        if (player->dest_rect.y + player->dest_rect.height >= GetScreenHeight())
        {
            player->dest_rect.y = GetScreenHeight() - player->dest_rect.height;
            player->vel.y *= -0.9f;
        }
    }
    // If player has completely moved off screen, respawn in center
    else if (playerRight < 0 || playerLeft > GetScreenWidth() ||
             playerBottom < 0 || playerTop > GetScreenHeight())
    {
        // Player has completely exited - respawn in center with new random velocity
        // no player dissapear
        // player->dest_rect.x = GetScreenWidth() / 2.0f - player->dest_rect.width / 2.0f;
        // player->dest_rect.y = GetScreenHeight() / 2.0f - player->dest_rect.height / 2.0f;
        player->vel.x = 0;
        player->vel.y = 0;
    }

    // Apply friction as energy loss
    player->vel.x = (player->vel.x) - (player->vel.x * 0.0001f);
    player->vel.y = (player->vel.y) - (player->vel.y * 0.0001f);
}

void move_player(Sprite *player)
{
    if (IsKeyPressed(KEY_RIGHT))
    {
        player->vel.x = 400.0;
        player->vel.y = 0;
        touchCount++;
    }
    if (IsKeyPressed(KEY_LEFT))
    {
        player->vel.x = -400.0;
        player->vel.y = 0;
        touchCount++;
    }
    if (IsKeyPressed(KEY_UP))
    {
        player->vel.y = -400.0;
        player->vel.x = 0;
        touchCount++;
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        player->vel.y = 400.0;
        player->vel.x = 0;
        touchCount++;
    }
}

void move_by_mouse(Sprite *player)
{
    static Vector2 mouseStartPos = {0.0f, 0.0f};
    static bool isDragging = false;
    Vector2 mousePos = GetMousePosition();
    // Check if mouse is clicked on the ball
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        (mousePos.x >= player->dest_rect.x - 10) &&
        (mousePos.x <= 10 + (player->dest_rect.x + player->dest_rect.width)) &&
        (mousePos.y >= player->dest_rect.y - 10) &&
        (mousePos.y <= 10 + (player->dest_rect.y + player->dest_rect.height)))
    {
        isDragging = true;
        mouseStartPos = mousePos;
        // Reset velocity when starting new swipe
        // player->vel.x = 0;
        // player->vel.y = 0;
    }

    // Track the swipe while dragging
    if (isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        // Draw a line to visualize the swipe direction
        DrawLineEx(mouseStartPos, mousePos, 2, RED);
    }

    // When mouse is released, apply velocity based on swipe
    if (isDragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        isDragging = false;
        touchCount++;

        // Calculate swipe vector
        Vector2 swipeVector = {
            mousePos.x - mouseStartPos.x,
            mousePos.y - mouseStartPos.y};

        // Calculate swipe distance for magnitude
        float swipeDist = sqrt(swipeVector.x * swipeVector.x + swipeVector.y * swipeVector.y);

        // Normalize and apply velocity based on swipe distance
        float speedMultiplier = 5.0f; // Adjust this to control sensitivity
        if (swipeDist > 0)
        {
            player->vel.x = (swipeVector.x / swipeDist) * swipeDist * speedMultiplier;
            player->vel.y = (swipeVector.y / swipeDist) * swipeDist * speedMultiplier;
        }
    }

    // Apply the velocity
    // apply_velocity(player);
}

void update_obstacle(Obstacle *obstacle)
{
    obstacle->bounds.x += obstacle->velocity.x * GetFrameTime();
    obstacle->bounds.y += obstacle->velocity.y * GetFrameTime();

    // Bounce off walls
    if (obstacle->bounds.x <= 0 || obstacle->bounds.x + obstacle->bounds.width >= GetScreenWidth())
    {
        obstacle->velocity.x *= -1;
    }
    if (obstacle->bounds.y <= 0 || obstacle->bounds.y + obstacle->bounds.height >= GetScreenHeight())
    {
        obstacle->velocity.y *= -1;
    }
}

// Update the check_obstacle_collision function
bool check_obstacle_collision(Sprite *player, Obstacle *obstacle)
{
    if (CheckCollisionRecs(player->dest_rect, obstacle->bounds))
    {
        if (obstacle->is_deadly)
        {
            // Reset player position
            player->dest_rect.x = GetScreenWidth() / 2.0f - player->dest_rect.width / 2.0f;
            player->dest_rect.y = GetScreenHeight() / 2.0f - player->dest_rect.height / 2.0f;
            player->vel.x = (rand() % 401) - 200;
            player->vel.y = (rand() % 401) - 200;
            hasAvoidedDeadlyObstacles = false; // Player hit a deadly obstacle
            return true;
        }
        else if (obstacle->is_bouncy)
        {
            // Bounce with extra force
            player->vel.x *= -1.5f;
            player->vel.y *= -1.5f;
        }
    }
    return false;
}

Vector4 draw_winhole(float hole_length)
{
    bool randchoice = ((rand() % 2) == 0); // rand between 0 and 1
    bool randchoice1 = ((rand() % 2) == 0);
    float randborderx = (rand() % (GetScreenWidth() - 200)) + 100;
    float randbordery = (rand() % (GetScreenHeight() - 200)) + 100;

    // int hole_length = 66; // 110% of sprite width 60
    hole_length = hole_length * 1.1;

    Vector2 startpoint;
    Vector2 endpoint;

    if (randchoice)
    {
        startpoint = {randborderx, randchoice1 * static_cast<float>(GetScreenHeight())};

        if (startpoint.y == 0)
        {
            endpoint.x = startpoint.x + hole_length;
            endpoint.y = 0;
        }
        else if (startpoint.y == GetScreenHeight())
        {
            endpoint.x = startpoint.x + hole_length;
            endpoint.y = GetScreenHeight();
        }
    }
    else
    {
        startpoint = {randchoice1 * static_cast<float>(GetScreenWidth()), randbordery};

        if (startpoint.x == 0)
        {
            endpoint.x = 0;
            endpoint.y = startpoint.y + hole_length;
        }
        else if (startpoint.x == static_cast<float>(GetScreenWidth()))
        {
            endpoint.x = GetScreenWidth();
            endpoint.y = startpoint.y + hole_length;
        }
    }
    Vector4 startendpoint = {startpoint.x, startpoint.y, endpoint.x, endpoint.y};
    return startendpoint;
}

void updateAchievements(float gameTime, int score, bool hasAvoidedDeadlyObstacles)
{
    // Speed Runner achievement
    if (gameTime <= 10.0f)
    {
        achievements[0].unlocked = true;
    }

    // Perfect Exit achievement
    if (score >= 5000)
    {
        achievements[1].unlocked = true;
    }

    // Obstacle Master achievement
    if (hasAvoidedDeadlyObstacles)
    {
        achievements[2].unlocked = true;
    }
}

void drawAchievements(int startX, int startY)
{
    for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
    {
        const char *status = achievements[i].unlocked ? "✓" : "×";
        Color statusColor = achievements[i].unlocked ? GREEN : RED;

        // Draw achievement box
        DrawRectangle(startX, startY + i * 30, 300, 25, Fade(DARKGRAY, 0.5f));

        // Draw achievement name and description
        DrawText(TextFormat("%s %s", status, achievements[i].name),
                 startX + 5, startY + i * 30 + 5, 20, statusColor);
    }
}

int main()
{
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    const int WindowX = 1920;
    const int WindowY = 1080; // Border Break

    InitWindow(WindowX, WindowY, "Bouncy ball Escape the hole game"); // initialize the application
    SetTargetFPS(60);
    InitAudioDevice();

    Texture2D smile_ball_texture = LoadTexture("assets/balls/blue/smile.png");
    Texture2D angry_ball_texture = LoadTexture("assets/balls/blue/angry.png");
    Texture2D smile_ball_texture2 = LoadTexture("assets/balls/red/smile.png");
    Texture2D angry_ball_texture2 = LoadTexture("assets/balls/red/angry.png");
    Texture2D tansparent_texture = LoadTexture("assets/balls/blue/transparent.png");
    Texture2D start_screen = LoadTexture("assets/start_screen.png");
    Texture2D levels_screen = LoadTexture("assets/levels_screen.png");
    Texture2D background_screen = LoadTexture("assets/fire_land.jpg");
    Music start_screen_song = LoadMusicStream("assets/sounds/start_screen_song.mp3");
    Music levels_screen_song = LoadMusicStream("assets/sounds/fire_sound.mp3");
    Sound hit_wall = LoadSound("assets/sounds/hit_wall.mp3");
    Sound win_level = LoadSound("assets/sounds/win_level.mp3");
    Sound hurt_sound = LoadSound("assets/sounds/hurt.mp3"); // Load the new hurt sound
    Sprite player = {
        smile_ball_texture,
        {
            WindowX / 2.0f - 30.0f, // Properly center based on width (60.0)
            WindowY / 2.0f - 30.0f, // Properly center based on height (60.0)
            60.0,
            60.0,
        }};

    bool x = false;
    // Random initial velocity between -200 to 200 for x and y
    srand(time(NULL)); // Seed the random number generator
    player.vel.x = (rand() % 401) - 200;
    player.vel.y = (rand() % 401) - 200;

    // Add after player initialization
    Obstacle obstacles[MAX_OBSTACLES];

    // Initialize obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++)
    {
        obstacles[i].bounds.width = 40;
        obstacles[i].bounds.height = 40;
        obstacles[i].bounds.x = GetRandomValue(100, WindowX - 100);
        obstacles[i].bounds.y = GetRandomValue(100, WindowY - 100);
        obstacles[i].velocity.x = GetRandomValue(-200, 200);
        obstacles[i].velocity.y = GetRandomValue(-200, 200);
        obstacles[i].is_deadly = (i % 2 == 0); // Alternate between deadly and bouncy
        obstacles[i].is_bouncy = !obstacles[i].is_deadly;
        obstacles[i].color = obstacles[i].is_deadly ? RED : YELLOW;
    }

    bool winhole_drawn = false;
    Vector4 winhole = {};
    const char *msg = "YOU WON!";
    const char *restart_msg = "Double tap to go to level select";
    const int fontsize = 80;
    const int small_fontsize = 30;
    const int msgwidth = MeasureText(msg, fontsize);
    const int restart_width = MeasureText(restart_msg, small_fontsize);
    bool gamewon = false;
    bool showWinMessage = false;
    float winMessageTimer = 0.0f;
    const float winMessageDuration = 5.0f;
    // Show win message for 2 seconds
    GameScreen currentScreen = START_SCREEN;

    PlayMusicStream(start_screen_song); // Play music once at start

    while (!WindowShouldClose()) // run app
    {
        if (currentScreen == START_SCREEN)
        {
            UpdateMusicStream(start_screen_song); // Only update the stream
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                StopMusicStream(start_screen_song); // Stop music when leaving start screen
                currentScreen = SELECT_LEVEL_SCREEN;
            }
        }
        else if (currentScreen == SELECT_LEVEL_SCREEN)
        {
            PlayMusicStream(levels_screen_song);   // Play music once at start
            UpdateMusicStream(levels_screen_song); // Play music once at start

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && (GetMousePosition().x <= GetScreenWidth() / 2))
            {
                StopMusicStream(levels_screen_song); // Stop music when leaving levels screen

                currentScreen = LEVEL1_SCREEN;
                gameStartTime = GetTime(); // START TIMER
                touchCount = 0;
                scoreCalculated = false;
                finalScore = 0;
                hasAvoidedDeadlyObstacles = true;
                for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
                {
                    achievements[i].unlocked = false;
                }
            }
            else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && (GetMousePosition().x >= GetScreenWidth() / 2))

            {
                StopMusicStream(levels_screen_song); // Stop music when leaving levels screen
                currentScreen = LEVEL2_SCREEN;
                gameStartTime = GetTime(); // START TIMER
                touchCount = 0;
                scoreCalculated = false;
                finalScore = 0;
                hasAvoidedDeadlyObstacles = true;
                for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
                {
                    achievements[i].unlocked = false;
                }
            }
        }
        else if ((currentScreen == LEVEL1_SCREEN || currentScreen == LEVEL2_SCREEN) && IsKeyPressed(KEY_ENTER))
        {
            break; // Exit when ENTER pressed on a level screen
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (currentScreen)
        {
        case START_SCREEN:
            DrawTexture(start_screen, 0, 0, RAYWHITE);

            break;

        case SELECT_LEVEL_SCREEN:
            // DRAW_FULLSCREEN(selectLevelImage);
            DrawTexture(levels_screen, 0, 0, RAYWHITE);
            break;

        case LEVEL1_SCREEN:
            // DRAW_FULLSCREEN(lvl1Image);
            DrawText("LEVEL 1",
                     GetScreenWidth() / 2 - MeasureText("LEVEL 1", 20) / 2,
                     GetScreenHeight() - 40, 20, DARKBLUE);
            //////////////////////////////////////////////////////////////////////////////////////////////////////

            // Process movement - even if game is won, we still allow movement for pass-through
            move_player(&player);
            move_by_mouse(&player);

            // Apply velocity with the updated function that checks for passing through hole
            apply_velocity(&player, winhole, &gamewon);

            // If game was just won, start showing win message
            if (gamewon && !showWinMessage)
            {
                showWinMessage = true;
                winMessageTimer = 0.0f;
                PlaySound(win_level); // Add this line here
                player.texture = tansparent_texture;
                if (!scoreCalculated)
                {
                    float elapsedTime = GetTime() - gameStartTime;

                    // Score: inverse relationship (lower time and fewer touches = higher score)
                    finalScore = (int)(10000 / (elapsedTime + touchCount + 1)); // +1 to avoid divide by zero
                    if (finalScore > 9999)
                        finalScore = 9999;
                    scoreCalculated = true;

                    updateAchievements(elapsedTime, finalScore, hasAvoidedDeadlyObstacles);
                }
            }

            // Update timer for win message
            if (showWinMessage)
            {
                winMessageTimer += GetFrameTime();
                if (winMessageTimer >= winMessageDuration)
                {
                    // After duration, keep showing message only if player is still on screen
                    if (player.dest_rect.x > -player.dest_rect.width &&
                        player.dest_rect.x < WindowX &&
                        player.dest_rect.y > -player.dest_rect.height &&
                        player.dest_rect.y < WindowY)
                    {
                        showWinMessage = true;
                    }
                    else
                    {
                        showWinMessage = false;
                    }
                }
            }

            // Update player texture based on position
            player.texture = smile_ball_texture;
            if (player.dest_rect.x <= 20 || player.dest_rect.x + player.dest_rect.width >= WindowX - 20 ||
                player.dest_rect.y <= 20 || player.dest_rect.y + player.dest_rect.height >= WindowY - 20)
            {
                PlaySound(hit_wall);
                player.texture = angry_ball_texture;
            }

            // Draw border lines
            DrawLineEx({0, 0}, {(float)WindowX, 0}, 20, RED);                           // Top
            DrawLineEx({0, 0}, {0, (float)WindowY}, 20, RED);                           // Left
            DrawLineEx({(float)WindowX, 0}, {(float)WindowX, (float)WindowY}, 20, RED); // Right
            DrawLineEx({0, (float)WindowY}, {(float)WindowX, (float)WindowY}, 20, RED); // Bottom

            // Drawing winning hole
            if (!winhole_drawn)
            {
                winhole = draw_winhole(player.dest_rect.height);
                winhole_drawn = true;
            }
            DrawLineEx({winhole.x, winhole.y}, {winhole.z, winhole.w}, 20, BLUE);

            // Update and draw obstacles
            for (int i = 0; i < MAX_OBSTACLES; i++)
            {
                if (!gamewon)
                { // Only update obstacles if game isn't won
                    update_obstacle(&obstacles[i]);
                    if (check_obstacle_collision(&player, &obstacles[i]))
                    {
                        PlaySound(hurt_sound); // Play the hurt sound
                    }
                }

                // Level 1: Rectangle obstacles
                DrawRectangleRec(obstacles[i].bounds, obstacles[i].color);
            }

            // Display win message if needed
            if (showWinMessage)
            {
                DrawText(msg, (WindowX - msgwidth) / 2, (WindowY / 2) - 100, fontsize, GREEN);
                DrawText(restart_msg, (WindowX - restart_width) / 2, (WindowY / 2), small_fontsize, BLACK);
                if (scoreCalculated)
                {
                    DrawText(
                        TextFormat("Score: %d", finalScore),
                        GetScreenWidth() / 2 - MeasureText(TextFormat("Score: %d", finalScore), 40) / 2,
                        GetScreenHeight() / 2 + 100,
                        40,
                        GOLD);
                }

                // Display unlocked achievements
                drawAchievements(20, 100);
            }
            // Display score during the game
            DrawText(TextFormat("Score: %d", finalScore), 20, 20, 30, BLACK);

            // Display achievements
            drawAchievements(GetScreenWidth() - 320, 20);

            // Handle restart with space key
            if (IsKeyPressed(KEY_SPACE))
            {
                // Reset the game
                player.vel.x = (rand() % 401) - 200; // random from -200 to 200
                player.vel.y = (rand() % 401) - 200;
                player.dest_rect.x = WindowX / 2.0f - 30.0f;
                player.dest_rect.y = WindowY / 2.0f - 30.0f;

                // Reset game state
                gamewon = false;
                showWinMessage = false;
                winhole_drawn = false;
                gamewon = false;
                showWinMessage = false;
                winMessageTimer = 0.0f;
                gameStartTime = GetTime();
                touchCount = 0;
                scoreCalculated = false;
                finalScore = 0;
                hasAvoidedDeadlyObstacles = true; // Reset deadly obstacle tracking

                // Reset obstacles
                for (int i = 0; i < MAX_OBSTACLES; i++)
                {
                    obstacles[i].bounds.x = GetRandomValue(100, WindowX - 100);
                    obstacles[i].bounds.y = GetRandomValue(100, WindowY - 100);
                    obstacles[i].velocity.x = GetRandomValue(-200, 200);
                    obstacles[i].velocity.y = GetRandomValue(-200, 200);
                }

                // Reset achievements
                for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
                {
                    achievements[i].unlocked = false;
                }
            }

            // Draw the player
            DrawTexturePro(player.texture,
                           {0.0, 0.0, (float)player.texture.width, (float)player.texture.height},
                           player.dest_rect, {0, 0}, 0, RAYWHITE);

            ////////////////////////////////////////////////////////////////////////////////
            break;

        case LEVEL2_SCREEN:
            // DRAW_FULLSCREEN(lvl2Image);
            DrawText("LEVEL 2",
                     GetScreenWidth() / 2 - MeasureText("LEVEL 2", 20) / 2,
                     GetScreenHeight() - 40, 20, DARKGREEN);
            ClearBackground(BLACK);

            DrawTexture(background_screen, 0, 0, RAYWHITE);
            player.texture = smile_ball_texture2;
            player.dest_rect.width = 30.0f;
            player.dest_rect.height = 30.0f;

            //////////////////////////////////////////////////////////////////////////////////////////////////////

            // Process movement - even if game is won, we still allow movement for pass-through
            move_player(&player);
            move_by_mouse(&player);

            // Apply velocity with the updated function that checks for passing through hole
            apply_velocity(&player, winhole, &gamewon);

            // If game was just won, start showing win message
            if (gamewon && !showWinMessage)
            {
                showWinMessage = true;
                winMessageTimer = 0.0f;
                PlaySound(win_level); // Add this line here
                player.texture = tansparent_texture;
                if (!scoreCalculated)
                {
                    float elapsedTime = GetTime() - gameStartTime;

                    // Score: inverse relationship (lower time and fewer touches = higher score)
                    finalScore = (int)(10000 / (elapsedTime + touchCount + 1)); // +1 to avoid divide by zero
                    if (finalScore > 9999)
                        finalScore = 9999;
                    scoreCalculated = true;

                    updateAchievements(elapsedTime, finalScore, hasAvoidedDeadlyObstacles);
                }
            }

            // Update timer for win message
            if (showWinMessage)
            {
                winMessageTimer += GetFrameTime();
                if (winMessageTimer >= winMessageDuration)
                {
                    // After duration, keep showing message only if player is still on screen
                    if (player.dest_rect.x > -player.dest_rect.width &&
                        player.dest_rect.x < WindowX &&
                        player.dest_rect.y > -player.dest_rect.height &&
                        player.dest_rect.y < WindowY)
                    {
                        showWinMessage = true;
                    }
                    else
                    {
                        showWinMessage = false;
                    }
                }
            }

            // Update player texture based on position
            // player.texture = smile_ball_texture2;
            if (player.dest_rect.x <= 20 || player.dest_rect.x + player.dest_rect.width >= WindowX - 20 ||
                player.dest_rect.y <= 20 || player.dest_rect.y + player.dest_rect.height >= WindowY - 20)
            {
                PlaySound(hit_wall);
                player.texture = angry_ball_texture2;
            }

            // Draw border lines
            DrawLineEx({0, 0}, {(float)WindowX, 0}, 20, BLUE);                           // Top
            DrawLineEx({0, 0}, {0, (float)WindowY}, 20, BLUE);                           // Left
            DrawLineEx({(float)WindowX, 0}, {(float)WindowX, (float)WindowY}, 20, BLUE); // Right
            DrawLineEx({0, (float)WindowY}, {(float)WindowX, (float)WindowY}, 20, BLUE); // Bottom

            // Drawing winning hole
            if (!winhole_drawn)
            {
                winhole = draw_winhole(player.dest_rect.height);
                winhole_drawn = true;
            }
            DrawLineEx({winhole.x, winhole.y}, {winhole.z, winhole.w}, 20, RED);

            // Update and draw obstacles
            for (int i = 0; i < MAX_OBSTACLES; i++)
            {
                if (!gamewon)
                { // Only update obstacles if game isn't won
                    update_obstacle(&obstacles[i]);
                    x = check_obstacle_collision(&player, &obstacles[i]);
                }

                // Level 2: Circular obstacles with trail effect
                DrawCircle(
                    obstacles[i].bounds.x + obstacles[i].bounds.width / 2,
                    obstacles[i].bounds.y + obstacles[i].bounds.height / 2,
                    obstacles[i].bounds.width / 2,
                    obstacles[i].color);

                // Draw trail effect
                for (int j = 1; j <= 3; j++)
                {
                    DrawCircle(
                        obstacles[i].bounds.x + obstacles[i].bounds.width / 2 - obstacles[i].velocity.x * GetFrameTime() * j,
                        obstacles[i].bounds.y + obstacles[i].bounds.height / 2 - obstacles[i].velocity.y * GetFrameTime() * j,
                        obstacles[i].bounds.width / 2 * (1.0f - j * 0.2f),
                        Fade(obstacles[i].color, 0.5f - j * 0.1f));
                }
            }

            // Display win message if needed
            if (showWinMessage)
            {
                DrawText(msg, (WindowX - msgwidth) / 2, (WindowY / 2) - 100, fontsize, GREEN);

                DrawText(restart_msg, (WindowX - restart_width) / 2, (WindowY / 2), small_fontsize, WHITE);
                if (scoreCalculated)
                {
                    DrawText(
                        TextFormat("Score: %d", finalScore),
                        GetScreenWidth() / 2 - MeasureText(TextFormat("Score: %d", finalScore), 40) / 2,
                        GetScreenHeight() / 2 + 100,
                        40,
                        GOLD);
                }

                // Display unlocked achievements
                drawAchievements(20, 100);
            }
            // Display score during the game
            DrawText(TextFormat("Score: %d", finalScore), 20, 20, 30, WHITE);

            // Display achievements
            drawAchievements(GetScreenWidth() - 320, 20);

            // Handle restart with space key
            if (IsKeyPressed(KEY_SPACE))
            {
                // Reset the game
                player.vel.x = (rand() % 401) - 200; // random from -200 to 200
                player.vel.y = (rand() % 401) - 200;
                player.dest_rect.x = WindowX / 2.0f - 30.0f;
                player.dest_rect.y = WindowY / 2.0f - 30.0f;

                // Reset game state
                gamewon = false;
                showWinMessage = false;
                winhole_drawn = false;
                gamewon = false;
                showWinMessage = false;
                winMessageTimer = 0.0f;
                gameStartTime = GetTime();
                touchCount = 0;
                scoreCalculated = false;
                finalScore = 0;
                hasAvoidedDeadlyObstacles = true; // Reset deadly obstacle tracking

                // Reset obstacles
                for (int i = 0; i < MAX_OBSTACLES; i++)
                {
                    obstacles[i].bounds.x = GetRandomValue(100, WindowX - 100);
                    obstacles[i].bounds.y = GetRandomValue(100, WindowY - 100);
                    obstacles[i].velocity.x = GetRandomValue(-200, 200);
                    obstacles[i].velocity.y = GetRandomValue(-200, 200);
                }

                // Reset achievements
                for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
                {
                    achievements[i].unlocked = false;
                }
            }

            // Draw the player
            DrawTexturePro(player.texture,
                           {0.0, 0.0, (float)player.texture.width, (float)player.texture.height},
                           player.dest_rect, {0, 0}, 0, RAYWHITE);

            ////////////////////////////////////////////////////////////////////////////////
            break;
        }

        if (IsKeyPressed(KEY_BACK))
        {
            currentScreen = START_SCREEN;
            // Reset the game
            player.vel.x = (rand() % 401) - 200; // random from -200 to 200
            player.vel.y = (rand() % 401) - 200;
            player.dest_rect.x = WindowX / 2.0f - 30.0f;
            player.dest_rect.y = WindowY / 2.0f - 30.0f;

            // Reset game state
            gamewon = false;
            showWinMessage = false;
            winhole_drawn = false;
            gamewon = false;
            showWinMessage = false;
            winMessageTimer = 0.0f;
            gameStartTime = GetTime();
            touchCount = 0;
            scoreCalculated = false;
            finalScore = 0;
            hasAvoidedDeadlyObstacles = true; // Reset deadly obstacle tracking

            // Reset obstacles
            for (int i = 0; i < MAX_OBSTACLES; i++)
            {
                obstacles[i].bounds.x = GetRandomValue(100, WindowX - 100);
                obstacles[i].bounds.y = GetRandomValue(100, WindowY - 100);
                obstacles[i].velocity.x = GetRandomValue(-200, 200);
                obstacles[i].velocity.y = GetRandomValue(-200, 200);
            }

            // Reset achievements
            for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
            {
                achievements[i].unlocked = false;
            }
        }

        if (x)
        {
            PlaySound(hurt_sound); // Play the hurt sound
            x = false;
        }
        int gesture = GetGestureDetected();
        if (gesture == GESTURE_DOUBLETAP)
        {

            currentScreen = SELECT_LEVEL_SCREEN;
            // Reset the game
            player.vel.x = (rand() % 401) - 200; // random from -200 to 200
            player.vel.y = (rand() % 401) - 200;
            player.dest_rect.x = WindowX / 2.0f - 30.0f;
            player.dest_rect.y = WindowY / 2.0f - 30.0f;

            // Reset game state
            gamewon = false;
            showWinMessage = false;
            winhole_drawn = false;
            gamewon = false;
            showWinMessage = false;
            winMessageTimer = 0.0f;
            gameStartTime = GetTime();
            touchCount = 0;
            scoreCalculated = false;
            finalScore = 0;
            hasAvoidedDeadlyObstacles = true; // Reset deadly obstacle tracking

            // Reset obstacles
            for (int i = 0; i < MAX_OBSTACLES; i++)
            {
                obstacles[i].bounds.x = GetRandomValue(100, WindowX - 100);
                obstacles[i].bounds.y = GetRandomValue(100, WindowY - 100);
                obstacles[i].velocity.x = GetRandomValue(-200, 200);
                obstacles[i].velocity.y = GetRandomValue(-200, 200);
            }

            // Reset achievements
            for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
            {
                achievements[i].unlocked = false;
            }
        }

        EndDrawing();
    }

    UnloadTexture(smile_ball_texture);
    UnloadTexture(angry_ball_texture);
    UnloadTexture(tansparent_texture);
    UnloadTexture(start_screen);
    UnloadTexture(levels_screen);
    CloseAudioDevice();
    StopMusicStream(start_screen_song);
    UnloadMusicStream(start_screen_song);
    UnloadSound(hit_wall);
    UnloadSound(win_level);
    UnloadSound(hurt_sound); // Unload the hurt sound

    CloseWindow(); // close app

    return 0;
}
