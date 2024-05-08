/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the MIT
 * license.  You should have received a copy of the MIT license with
 * this file.  If not, please write to: aryadev@aryadevchavali.com.

 * Created: 2024-05-08
 * Author: Aryadev Chavali
 * Description: Entrypoint
 */

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <raylib.h>
#include <sstream>
#include <string>
#include <vector>

#define WIDTH  800
#define HEIGHT 600

long mod(long a, long b)
{
  return (a % b + b) % b;
}

enum class Type
{
  EMPTY,
  WALL,
  FRUIT,
};

enum class Direction
{
  UP    = 1,
  LEFT  = 2,
  RIGHT = -2,
  DOWN  = -1
};

struct Point
{
  int x, y;

  bool operator==(const Point &p) const
  {
    return x == p.x && y == p.y;
  }

  Point operator+(const Point &p) const
  {
    return {x + p.x, y + p.y};
  }

  Point operator-(const Point &p) const
  {
    return {x - p.x, y - p.y};
  }

  Point operator*(const Point &p) const
  {
    return {x * p.x, y * p.y};
  }

  Point operator*(int m) const
  {
    return {x * m, y * m};
  }

  Point() : x{0}, y{0}
  {}

  Point(int x, int y) : x{x}, y{y}
  {}

  Point(Direction dir)
  {
    switch (dir)
    {
    case Direction::UP:
      *this = Point{0, -1};
      break;
    case Direction::DOWN:
      *this = Point{0, 1};
      break;
    case Direction::LEFT:
      *this = Point{-1, 0};
      break;
    case Direction::RIGHT:
      *this = Point{1, 0};
      break;
    }
  }
};

template <size_t a, size_t b>
struct State
{
  static constexpr double mx          = WIDTH / a;
  static constexpr double my          = HEIGHT / b;
  static constexpr double square_size = mx < my ? mx : my;

  struct Player
  {
    std::vector<Point> points;
  } player;

  Type grid[a][b];

  bool is_player(size_t x, size_t y) const
  {
    return std::find(player.points.begin(), player.points.end(),
                     Point{(int)x, (int)y}) != player.points.end();
  }

  double rescale(size_t grid, bool smaller, double translation) const
  {
    double x = grid * square_size;
    if (smaller)
      x += translation;
    return x;
  }

  void draw_grid() const
  {
    for (size_t x = 0; x < a; ++x)
    {
      double x_ = rescale(x, HEIGHT < WIDTH, (WIDTH - HEIGHT) / 2);
      for (size_t y = 0; y < b; ++y)
      {
        double y_ = rescale(y, WIDTH < HEIGHT, (HEIGHT - WIDTH) / 2);

        if (grid[x][y] == Type::EMPTY)
          DrawRectangleLines(x_, y_, square_size, square_size, WHITE);
        else if (grid[x][y] == Type::WALL)
          DrawRectangle(x_, y_, square_size, square_size, WHITE);
        else if (grid[x][y] == Type::FRUIT)
          DrawRectangle(x_, y_, square_size, square_size, RED);
      }
    }

    for (const auto p : player.points)
    {
      double x_ = rescale(p.x, HEIGHT < WIDTH, (WIDTH - HEIGHT) / 2);
      double y_ = rescale(p.y, WIDTH < HEIGHT, (HEIGHT - WIDTH) / 2);
      DrawRectangle(x_, y_, square_size, square_size, GREEN);
    }
  }

  std::vector<Point>::iterator player_head(void)
  {
    return player.points.begin();
  }

  bool update_player_head(Point dir)
  {
    auto head          = player_head();
    Point old_position = *head;
    Point new_position = old_position;
    new_position.y += dir.y;
    new_position.x += dir.x;
    new_position.x = mod(new_position.x, a);
    new_position.y = mod(new_position.y, b);

    if (is_player(new_position.x, new_position.y) ||
        grid[new_position.x][new_position.y] == Type::WALL)
      return true;

    *head = new_position;

    for (size_t i = 1; i < player.points.size(); ++i)
    {
      Point cpy        = player.points[i];
      player.points[i] = old_position;
      old_position     = cpy;
    }
    return false;
  }

  void player_fruit_collision(void)
  {
    const auto point = player_head();
    if (grid[point->x][point->y] == Type::FRUIT)
    {
      grid[point->x][point->y]          = Type::EMPTY;
      std::vector<Point>::iterator last = player.points.end() - 1;
      player.points.push_back({last->x + 1, last->y});
    }
  }

  void reset(void)
  {
    player.points.clear();
    player.points.push_back({a / 2, b / 2});
    memset(grid, 0, sizeof(Type) * a * b);
  }
};

namespace chrono = std::chrono;

struct Time
{
  size_t hours, minutes, seconds;

  Time(size_t s)
  {
    hours = s / 3600;
    s -= (hours * 3600);
    minutes = s / 60;
    s -= (minutes * 60);
    seconds = s;
  }

  std::string to_str(void)
  {
    std::stringstream s;
    if (hours < 10)
      s << "0";
    s << std::to_string(hours) + ":";
    if (minutes < 10)
      s << "0";
    s << std::to_string(minutes) + ":";
    if (seconds < 10)
      s << "0";
    s << std::to_string(seconds);
    return s.str();
  }
};

using Clock = chrono::steady_clock;
int main(void)
{
  constexpr size_t X = 50, Y = 50;
  State<X, Y> state;
  state.player.points.push_back({15, 15});
  for (size_t i = 0; i < X; ++i)
    for (size_t j = 0; j < Y; ++j)
      state.grid[i][j] = Type::FRUIT;

  Direction dir = Direction::LEFT;
  InitWindow(WIDTH, HEIGHT, "snek");
  int fps = 10;
  SetTargetFPS(fps);

  chrono::time_point<Clock> beg{Clock::now()};
  while (!WindowShouldClose())
  {
    state.player_fruit_collision();

    if (IsKeyDown(KEY_J) && (Direction)(-((int)dir)) != Direction::DOWN)
      dir = Direction::DOWN;
    else if (IsKeyDown(KEY_K) && (Direction)(-((int)dir)) != Direction::UP)
      dir = Direction::UP;
    else if (IsKeyDown(KEY_H) && (Direction)(-((int)dir)) != Direction::LEFT)
      dir = Direction::LEFT;
    else if (IsKeyDown(KEY_L) && (Direction)(-((int)dir)) != Direction::RIGHT)
      dir = Direction::RIGHT;
    else if (IsKeyDown(KEY_EQUAL))
    {
      ++fps;
      if (fps > 60)
        fps = 60;
      SetTargetFPS(fps);
    }
    else if (IsKeyDown(KEY_MINUS))
    {
      --fps;
      if (fps < 0)
        fps = 1;
      SetTargetFPS(fps);
    }

    bool collide = state.update_player_head(dir);
    if (collide)
    {
      state.reset();
      beg = Clock::now();
    }

    BeginDrawing();
    ClearBackground(BLACK);
    state.draw_grid();
    DrawText("TIME", 0, 0, 25, YELLOW);
    DrawText(
        Time(chrono::duration_cast<chrono::seconds>(Clock::now() - beg).count())
            .to_str()
            .c_str(),
        0, 25, 25, YELLOW);

    std::stringstream ss;
    ss << "Score: ";
    ss << state.player.points.size() - 1;
    DrawText(ss.str().c_str(), 0, 50, 20, YELLOW);

    EndDrawing();
  }
  CloseWindow();
  return 0;
}
