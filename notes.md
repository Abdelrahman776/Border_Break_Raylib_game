https://github.com/raysan5/raylib/wiki/Working-on-Windows

command on windows to build :

g++ main.cpp -o main.exe -I include -L lib -lraylib -lopengl32 -lgdi32 -lwinmm

<!-- gcc -o raylibhelloworld.exe raylibhelloworld.c -lraylib -lgdi32 -lwinmm -->

echo. > file.txt






Sprite player = {
    ball_texture,
    { 0.0, 0.0, 60.0, 60.0 }
};
You initialize your player sprite with:

the texture you loaded

a rectangle: x=0, y=0, width=60, height=60 → so it will be drawn at the top-left corner, size 60x60.

✅ DrawTexturePro(...)

DrawTexturePro(
    player.texture, // full image
    { 0.0, 0.0, (float)player.texture.width, (float)player.texture.height }, // source rect: full image
    player.dest_rect, // destination rectangle (on screen)
    { 0, 0 }, // origin (pivot) for rotation/scaling
    0, // rotation in degrees
    RAYWHITE // tint (white = normal)
);
| Parameter       | Meaning                                                                 |
|----------------|-------------------------------------------------------------------------|
| `source`       | The part of the texture to draw (here: the whole texture)               |
| `dest`         | Where to draw it and at what size (scaled to 60x60 at (0,0))            |
| `origin`       | Pivot point for rotation/scaling (here: top-left corner)                |
| `rotation`     | Rotation angle (0 = no rotation)                                        |
| `tint`         | Color filter (RAYWHITE = unchanged)                                     |




// DrawText(TextFormat("deltaX: %f , deltaY: %f ", GetMouseDelta().x, GetMouseDelta().y), 20, 20, 20, BLACK);

        // DrawText("It works!", 20, 20, 20, BLACK);
        // DrawTexture(ball_texture, 200.0, 10.0, WHITE);