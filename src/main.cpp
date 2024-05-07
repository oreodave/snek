/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the MIT
 * license.  You should have received a copy of the MIT license with
 * this file.  If not, please write to: aryadev@aryadevchavali.com.

 * Created: 2024-05-08
 * Author: Aryadev Chavali
 * Description: Entrypoint
 */

#include <raylib.h>

#define WIDTH  800
#define HEIGHT 600

int main(void)
{
  InitWindow(WIDTH, HEIGHT, "snek");
  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTriangle({400, 0}, {100, 300}, {700, 300}, RED);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
