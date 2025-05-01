#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
// srand(time(NULL)); // Seed with current time

typedef struct Sprite
{
    Texture2D texture;
    Rectangle dest_rect;
    Vector2 vel;
} Sprite;

// void apply_velocity_nocol(Sprite *player)
// {
//     player->dest_rect.x += player->vel.x * GetFrameTime();
//     player->dest_rect.y += player->vel.y * GetFrameTime();
// }

void apply_velocity(Sprite *player)
{

    // GetFrameTime() to have consistent movement rate whenever we have varying frame rates in game
    // apply velocity with frame time
    player->dest_rect.x += player->vel.x * GetFrameTime();
    player->dest_rect.y += player->vel.y * GetFrameTime();

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

    // apply friction as enerygy loss
    player->vel.x = (player->vel.x) - (player->vel.x * 0.0001f); // when it becomes zero it doesn't turn direction
    player->vel.y = (player->vel.y) - (player->vel.y * 0.0001f);
}

void move_player(Sprite *player)
{
    // player->vel.x = 0.0;
    // player->vel.y = 0.0;

    if (IsKeyPressed(KEY_RIGHT))
    {
        player->vel.x = 400.0;
        player->vel.y = 0;
    }
    if (IsKeyPressed(KEY_LEFT))
    {
        player->vel.x = -400.0;
        player->vel.y = 0;
    }
    if (IsKeyPressed(KEY_UP))
    {
        player->vel.y = -400.0;
        player->vel.x = 0;
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        player->vel.y = 400.0;
        player->vel.x = 0;
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

Vector4 draw_winhole()
{
    bool randchoice = ((rand() % 2) == 0); // rand between 0 and 1
    bool randchoice1 = ((rand() % 2) == 0);

    float randborderx = (rand() % (GetScreenWidth() - 200)) + 100;
    float randbordery = (rand() % (GetScreenHeight() - 200)) + 100;

    int hole_length = 266; // 110% of sprite width 60

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

int main()
{
    const int WindowX = 900;
    const int WindowY = 900;
    InitWindow(WindowX, WindowY, "Bouncy ball game"); // initialize the application

    Texture2D smile_ball_texture = LoadTexture("assets/balls/blue/smile.png");
    Texture2D angry_ball_texture = LoadTexture("assets/balls/blue/angry.png");

    Sprite player = {
        smile_ball_texture,
        {
            WindowX / 2.0f - player.dest_rect.width / 2.0f,
            WindowY / 2.0f - player.dest_rect.height / 2.0f,
            60.0,
            60.0,
        }};

    // Random initial velocity between -200 to 200 for x and y
    player.vel.x = (rand() % 401) - 200;
    player.vel.y = (rand() % 401) - 200;
    bool winhole_drawn = false;
    Vector4 winhole = {};
    const char *msg = "YOU WON!";
    const int fontsize = 80;
    const int msgwidth = MeasureText(msg, fontsize);

    bool gameend = false;
    while (!WindowShouldClose()) // run app
    {

        // SetTargetFPS(60)  ;
        move_player(&player);
        move_by_mouse(&player);
        apply_velocity(&player);

        BeginDrawing();
        ClearBackground(WHITE);
        // Draw border lines
        DrawLineEx({0, 0}, {(float)WindowX, 0}, 20, RED);                           // Top
        DrawLineEx({0, 0}, {0, (float)WindowY}, 20, RED);                           // Left
        DrawLineEx({(float)WindowX, 0}, {(float)WindowX, (float)WindowY}, 20, RED); // Right
        DrawLineEx({0, (float)WindowY}, {(float)WindowX, (float)WindowY}, 20, RED); // Bottom

        // drawing winning hole

        if (!winhole_drawn)
        {
            winhole = draw_winhole();
            winhole_drawn = true;
        }
        DrawLineEx({winhole.w, winhole.x}, {winhole.y, winhole.z}, 20, BLUE);

        player.texture = smile_ball_texture;
        // hitting the borders
        if (player.dest_rect.x <= 20 || player.dest_rect.x + player.dest_rect.width >= WindowX - 20 || player.dest_rect.y <= 20 || player.dest_rect.y + player.dest_rect.height >= WindowY - 20)
        {
            player.texture = angry_ball_texture;
        }

        // ////////////////////////wining conditionals///////////////////////////////////////////////////////////

        // Check if winhole is horizontal (y coordinates are same)
        if (winhole.x == winhole.z)
        {
            // Horizontal line collision - check if sprite is fully within hole width
            if ((player.dest_rect.x >= winhole.w) &&
                (player.dest_rect.x + player.dest_rect.width <= winhole.y) &&
                (player.dest_rect.y + player.dest_rect.height >= winhole.x - 10) &&
                (player.dest_rect.y <= winhole.x + 10))
            {

                DrawText(msg, ((WindowX - msgwidth) / 2), (WindowY / 2) - 100, fontsize, BLACK);
                // gameend = true;
            }
        }
        // Check if winhole is vertical (x coordinates are same)
        else if (winhole.w == winhole.y)
        {
            // Vertical line collision - check if sprite is fully within hole height
            if ((player.dest_rect.y >= winhole.x) &&
                (player.dest_rect.y + player.dest_rect.height <= winhole.z) &&
                (player.dest_rect.x + player.dest_rect.width >= winhole.w - 10) &&
                (player.dest_rect.x <= winhole.w + 10))
            {

                DrawText(msg, ((WindowX - msgwidth) / 2), (WindowY / 2) - 100, fontsize, BLACK);
                // gameend = true;
            }
        }
        if (IsKeyDown(KEY_SPACE))
        {
            // player.vel.x = 0;
            // player.vel.y = 0;
            player.vel.x = (rand() % 401) - 200; // random from -200 to 200
            player.vel.y = (rand() % 401) - 200;
            player.dest_rect.x = WindowX / 2.0f - player.dest_rect.width / 2.0f;
            player.dest_rect.y = WindowY / 2.0f - player.dest_rect.height / 2.0f;

            
        }
         

       
        DrawTexturePro(player.texture, {0.0, 0.0, (float)player.texture.width, (float)player.texture.height}, player.dest_rect, {0, 0}, 0, RAYWHITE);

        EndDrawing();
    }
    UnloadTexture(smile_ball_texture);
    UnloadTexture(angry_ball_texture);
    CloseWindow(); // close app

    return 0;
}
