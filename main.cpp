#include <raylib.h>
#include <stdlib.h>
#include <time.h>
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
        // player->dest_rect.x += 100.0 * GetFrameTime();
        // GetFrameTime() to have consistent movement rate whenever we have varying frame rates in game
    }
    if (IsKeyDown(KEY_LEFT))
    {
        player->vel.x = -100.0;
        player->vel.y = 0;
        
        // player->dest_rect.x -= 100.0 * GetFrameTime();
    }
    if (IsKeyDown(KEY_UP))
    {
        player->vel.y = -100.0;
        player->vel.x = 0;
        
        // player->dest_rect.y -= 100.0 * GetFrameTime();
    }
    if (IsKeyDown(KEY_DOWN))
    {
        player->vel.y = 100.0;
        player->vel.x = 0;

        // player->dest_rect.y += 100.0 * GetFrameTime();
    }
}
void move_by_mouse(Sprite *player)
{

    Vector2 mousePos = GetMousePosition();
    Vector2 mousedelta = GetMouseDelta();
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) )
    {
        // player->dest_rect.x += mousedelta.x * 10000 * GetFrameTime();
        // player->dest_rect.y += mousedelta.y * 10000 * GetFrameTime();
        player->vel.x = mousePos.x;
        // player->vel.y = mousedelta.y*100;

        if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)){
            apply_velocity(player);
        }

        DrawText("jjjjjj", 20, 20, 20, BLACK);
        // if (player->vel.x>200){
        //     player->vel.x = 200; // you can change the number later to fit the game
        // }
        // if (player->vel.y>200){
        //     player->vel.y = 200;
        // }
    }
}


void apply_velocity(Sprite * player){

    player->dest_rect.x += player->vel.x* GetFrameTime();
    player->dest_rect.y += player->vel.y* GetFrameTime();
}
int main()
{

    InitWindow(900, 900, "Bouncy ball game"); // initialize the application

    // Texture2D ball_texture = LoadTexture("assets/balls/blue/expresionless.png");
    Texture2D smile_ball_texture = LoadTexture("assets/balls/blue/smile.png");
    Texture2D angry_ball_texture = LoadTexture("assets/balls/blue/angry.png");

    Sprite player = {
        smile_ball_texture,
        {
            100.0,
            100.0,
            60.0,
            60.0,
        }};

    // Random initial velocity between 0 and 99
    // player.vel.x = rand() % 100;
    // player.vel.y = rand() % 100;

    while (!WindowShouldClose()) // run app
    {
        // SetTargetFPS(60)  ;
        BeginDrawing();
        ClearBackground(WHITE);
        move_player(&player);
        move_by_mouse(&player);
        apply_velocity(&player);
        // DrawText("It works!", 20, 20, 20, BLACK);
        // DrawTexture(ball_texture, 200.0, 10.0, WHITE);
        player.texture = smile_ball_texture;
        if (player.dest_rect.x <= 0 || player.dest_rect.x + player.dest_rect.width >= 900 || player.dest_rect.y <= 0 || player.dest_rect.y + player.dest_rect.height >= 900)
        {
            player.texture = angry_ball_texture;
        }

        DrawTexturePro(player.texture, {0.0, 0.0, (float)player.texture.width, (float)player.texture.height}, player.dest_rect, {0, 0}, 0, RAYWHITE);
        EndDrawing();
    }
    UnloadTexture(smile_ball_texture);
    UnloadTexture(angry_ball_texture);
    CloseWindow(); // close app

    return 0;
}
