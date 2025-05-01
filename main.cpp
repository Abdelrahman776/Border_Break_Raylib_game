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
void apply_velocity(Sprite *player);

void move_player(Sprite *player)
{
    // player->vel.x = 0.0;
    // player->vel.y = 0.0;

    if (IsKeyDown(KEY_RIGHT))
    {
        player->vel.x = 100.0;
        player->vel.y = 0;
    }
    if (IsKeyDown(KEY_LEFT))
    {
        player->vel.x = -100.0;
        player->vel.y = 0;
    }
    if (IsKeyDown(KEY_UP))
    {
        player->vel.y = -100.0;
        player->vel.x = 0;
    }
    if (IsKeyDown(KEY_DOWN))
    {
        player->vel.y = 100.0;
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
        float speedMultiplier = 2.0f; // Adjust this to control sensitivity
        if (swipeDist > 0)
        {
            player->vel.x = (swipeVector.x / swipeDist) * swipeDist * speedMultiplier;
            player->vel.y = (swipeVector.y / swipeDist) * swipeDist * speedMultiplier;
        }
    }

    // Apply the velocity
    apply_velocity(player);
}

void apply_velocity(Sprite *player)
{

    // GetFrameTime() to have consistent movement rate whenever we have varying frame rates in game
    player->dest_rect.x += player->vel.x * GetFrameTime();
    player->dest_rect.y += player->vel.y * GetFrameTime();
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

    while (!WindowShouldClose()) // run app
    {
        // SetTargetFPS(60)  ;
        BeginDrawing();
        ClearBackground(WHITE);
        move_player(&player);
        move_by_mouse(&player);
        // DrawText(TextFormat("deltaX: %f , deltaY: %f ", GetMouseDelta().x, GetMouseDelta().y), 20, 20, 20, BLACK);

        // DrawText("It works!", 20, 20, 20, BLACK);
        // DrawTexture(ball_texture, 200.0, 10.0, WHITE);
        player.texture = smile_ball_texture;
        if (player.dest_rect.x <= 0 || player.dest_rect.x + player.dest_rect.width >= WindowX || player.dest_rect.y <= 0 || player.dest_rect.y + player.dest_rect.height >= WindowY)
        {
            player.texture = angry_ball_texture;
        }

        if (IsKeyDown(KEY_SPACE))
        {
            // player.vel.x = 0;
            // player.vel.y = 0;
            player.vel.x = (rand() % 401) - 200;//random from -200 to 200
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
