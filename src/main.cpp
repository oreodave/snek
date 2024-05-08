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
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
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

  Point operator%(const Point &p) const
  {
    return {(int)mod(x, p.x), (int)mod(y, p.y)};
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

  enum class Layout
  {
    UNLIMITED,
    WALLS,
    WALLED_GARDEN
  } layout;

  struct Player
  {
    Direction dir;
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

        DrawRectangleLines(x_, y_, square_size, square_size, WHITE);

        if (grid[x][y] == Type::WALL)
          DrawRectangle(x_, y_, square_size, square_size, WHITE);
        else if (grid[x][y] == Type::FRUIT)
          DrawCircle(x_ + square_size / 2, y_ + square_size / 2,
                     square_size / 2, RED);
      }
    }

    double x_ =
        rescale(player.points.begin()->x, HEIGHT < WIDTH, (WIDTH - HEIGHT) / 2);
    double y_ =
        rescale(player.points.begin()->y, WIDTH < HEIGHT, (HEIGHT - WIDTH) / 2);
    DrawRectangle(x_, y_, square_size, square_size, YELLOW);
    // Eyes based on direction
    double x_eye_1 = x_;
    double y_eye_1 = y_;
    double x_eye_2 = x_;
    double y_eye_2 = y_;
    switch (player.dir)
    {
    case Direction::UP:
      x_eye_2 += (9.0 / 10) * square_size;
      break;
    case Direction::DOWN:
      y_eye_1 += (9.0 / 10) * square_size;
      y_eye_2 += (9.0 / 10) * square_size;
      x_eye_2 += (9.0 / 10) * square_size;
      break;
    case Direction::RIGHT:
      x_eye_1 += (9.0 / 10) * square_size;
      x_eye_2 += (9.0 / 10) * square_size;
      y_eye_2 += (9.0 / 10) * square_size;
      break;
    case Direction::LEFT:
      y_eye_2 += (9.0 / 10) * square_size;
      break;
    }

    DrawRectangle(x_eye_1, y_eye_1, square_size / 10, square_size / 10, RED);
    DrawRectangle(x_eye_2, y_eye_2, square_size / 10, square_size / 10, RED);
    for (size_t i = 1; i < player.points.size(); ++i)
    {
      const auto &p = player.points[i];
      x_            = rescale(p.x, HEIGHT < WIDTH, (WIDTH - HEIGHT) / 2);
      y_            = rescale(p.y, WIDTH < HEIGHT, (HEIGHT - WIDTH) / 2);
      DrawCircle(x_ + square_size / 2, y_ + square_size / 2, square_size / 2,
                 GREEN);
    }
  }

  std::vector<Point>::iterator player_head(void)
  {
    return player.points.begin();
  }

  bool update_player_head()
  {
    auto head          = player_head();
    Point old_position = *head;
    Point new_position = (old_position + Point{player.dir}) % Point{a, b};

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

  void make_rand_fruit(void)
  {
    size_t x = rand() % a;
    size_t y = rand() % b;
    while (grid[x][y] == Type::WALL || is_player(x, y))
    {
      x = rand() % a;
      y = rand() % b;
    }
    grid[x][y] = Type::FRUIT;
  }

  void make_rand_wall(void)
  {
    size_t x = rand() % a;
    size_t y = rand() % b;
    while (grid[x][y] == Type::FRUIT || is_player(x, y))
    {
      x = rand() % a;
      y = rand() % b;
    }
    grid[x][y] = Type::WALL;
  }

  void player_fruit_collision()
  {
    const auto point = player_head();
    if (grid[point->x][point->y] == Type::FRUIT)
    {
      // If only one point currently then put it anywhere
      grid[point->x][point->y] = Type::EMPTY;
      if (player.points.size() == 1)
      {
        player.points.push_back((Point{player.dir} * -1) + *point);
      }
      // Otherwise look at the last two points, calculate a direction
      // vector then make the new one
      else
      {
        auto last_1 = player.points.end() - 1;
        auto last_2 = player.points.end() - 2;
        player.points.push_back(*last_1 + (*last_1 - *last_2));
      }
    }
  }

  void reset(void)
  {
    player.points.clear();
    player.points.push_back({a / 2, b / 2});
    player.dir = Direction::LEFT;
    memset(grid, 0, sizeof(Type) * a * b);
    switch (layout)
    {
    case Layout::UNLIMITED:
      break;
    case Layout::WALLS: {
      size_t i = 0;
      for (size_t j = 0; j < b; ++j)
        grid[i][j] = Type::WALL;
      i = a - 1;
      for (size_t j = 0; j < b; ++j)
        grid[i][j] = Type::WALL;
      size_t j = 0;
      for (i = 0; i < a; ++i)
        grid[i][j] = Type::WALL;
      j = b - 1;
      for (i = 0; i < a; ++i)
        grid[i][j] = Type::WALL;
      break;
    }
    case Layout::WALLED_GARDEN: {
      size_t i = 0;
      for (size_t j = 0; j < b; ++j)
        if (j > (b * 2 / 3) || j < (b / 3))
          grid[i][j] = Type::WALL;
      i = a - 1;
      for (size_t j = 0; j < b; ++j)
        if (j > (b * 2 / 3) || j < (b / 3))
          grid[i][j] = Type::WALL;
      size_t j = 0;
      for (i = 0; i < a; ++i)
        if (i > (a * 2 / 3) || i < (a / 3))
          grid[i][j] = Type::WALL;
      j = b - 1;
      for (i = 0; i < a; ++i)
        if (i > (a * 2 / 3) || i < (a / 3))
          grid[i][j] = Type::WALL;
      break;
    }
    }
  }
};

namespace chrono = std::chrono;
using Clock      = chrono::steady_clock;

struct Timer
{
  double (*delta)(size_t);
  chrono::time_point<Clock> prev;

  Timer(double (*delta_fn)(size_t)) : delta{delta_fn}, prev{Clock::now()}
  {}

  bool triggered(size_t player_size)
  {
    chrono::time_point<Clock> current = Clock::now();
    if (chrono::duration_cast<chrono::milliseconds>(current - prev).count() >
        delta(player_size))
    {
      prev = current;
      return true;
    }
    return false;
  }
};

template <size_t min, size_t max, size_t max_score>
constexpr auto make_delta()
{
  return [](size_t player_size)
  {
    return max - ((max - min) * (player_size < max_score
                                     ? (double)player_size / (double)max_score
                                     : 1));
  };
}

template <size_t X, size_t Y>
void wall_layout(State<X, Y> &state)
{
  state.reset();
  size_t i = 0;
  for (size_t j = 0; j < Y; ++j)
    state.grid[i][j] = Type::WALL;
  i = X - 1;
  for (size_t j = 0; j < Y; ++j)
    state.grid[i][j] = Type::WALL;
  size_t j = 0;
  for (i = 0; i < X; ++i)
    state.grid[i][j] = Type::WALL;
  j = Y - 1;
  for (i = 0; i < X; ++i)
    state.grid[i][j] = Type::WALL;
}

int main(void)
{
  srand(time(NULL));
  constexpr size_t X = 20, Y = 20;
  State<X, Y> state;
  state.reset();

  InitWindow(WIDTH, HEIGHT, "snek");
  SetTargetFPS(60);

  constexpr size_t update_max_score = 50;
  constexpr size_t wall_min_score   = 40;

  Timer update_timer{make_delta<80, 300, update_max_score>()};
  Timer fruit_timer{make_delta<1000, 5000, update_max_score>()};
  Timer wall_timer{make_delta<5000, 10000, 100>()};

  chrono::time_point<Clock> time_start{Clock::now()};
  chrono::time_point<Clock> time_cur{Clock::now()};

  Direction dir = Direction::LEFT;
  bool paused   = false;
  bool failed   = false;
  bool details  = false;
  while (!WindowShouldClose())
  {
    if (IsKeyPressed(KEY_P))
    {
      paused = !paused;
    }
    else if (IsKeyPressed(KEY_ENTER))
    {
      if (failed)
      {
        state.reset();
        time_start = Clock::now();
        failed     = false;
        paused     = false;
      }
    }
    else if (IsKeyPressed(KEY_GRAVE))
      details = !details;
    else if (IsKeyPressed(KEY_ONE))
    {
      state.layout = State<X, Y>::Layout::UNLIMITED;
      state.reset();
      time_start = Clock::now();
      time_cur   = time_start;
    }
    else if (IsKeyPressed(KEY_TWO))
    {
      state.layout = State<X, Y>::Layout::WALLS;
      state.reset();
      time_start = Clock::now();
      time_cur   = time_start;
    }
    else if (IsKeyPressed(KEY_THREE))
    {
      state.layout = State<X, Y>::Layout::WALLED_GARDEN;
      state.reset();
      time_start = Clock::now();
      time_cur   = time_start;
    }
    if (!paused && !failed)
    {
      time_cur  = Clock::now();
      bool fast = false;
      if (IsKeyPressed(KEY_J))
        dir = Direction::DOWN;
      else if (IsKeyPressed(KEY_K))
        dir = Direction::UP;
      else if (IsKeyPressed(KEY_H))
        dir = Direction::LEFT;
      else if (IsKeyPressed(KEY_L))
        dir = Direction::RIGHT;
      else if (IsKeyDown(KEY_SPACE))
        fast = true;

      if (update_timer.triggered(fast ? update_max_score
                                      : state.player.points.size()))
      {
        if (!(state.player.points.size() > 1 &&
              (((Point{dir} + (*state.player.points.begin())) % Point{X, Y}) ==
               (*(state.player.points.begin() + 1)))))
          state.player.dir = dir;
        bool collide = state.update_player_head();
        if (collide)
        {
          failed = true;
        }
        state.player_fruit_collision();
      }

      if (fruit_timer.triggered(state.player.points.size()))
        state.make_rand_fruit();

      if (state.player.points.size() > wall_min_score &&
          wall_timer.triggered(state.player.points.size()))
        state.make_rand_wall();
    }

    BeginDrawing();
    ClearBackground(BLACK);
    state.draw_grid();

    size_t seconds =
        chrono::duration_cast<chrono::seconds>(time_cur - time_start).count();
    size_t hours   = seconds / 3600;
    seconds        = seconds % 3600;
    size_t minutes = seconds / 60;
    seconds        = seconds % 60;

    std::stringstream ss;
    ss << (hours < 10 ? "0" : "") << hours << ":" << (minutes < 10 ? "0" : "")
       << minutes << ":" << (seconds < 10 ? "0" : "") << seconds;
    DrawText(ss.str().c_str(), 0, 0, 25, YELLOW);

    ss.str("");
    ss << "Score: ";
    ss << state.player.points.size() - 1;
    DrawText(ss.str().c_str(), 0, 30, 20, YELLOW);

    if (details)
    {
      ss.str("");
      ss << "Next: " << fruit_timer.delta(state.player.points.size()) / 1000
         << "s";
      DrawText(ss.str().c_str(), 0, 80, 18, YELLOW);

      ss.str("");
      ss << 1 / (update_timer.delta(state.player.points.size()) / 1000)
         << " f/s";
      DrawText(ss.str().c_str(), 0, 100, 20, YELLOW);
    }

    if (failed)
    {
      size_t x_top = state.rescale(1, (HEIGHT < WIDTH), (WIDTH - HEIGHT) / 2);
      size_t y_top = state.rescale(1, (WIDTH < HEIGHT), (HEIGHT - WIDTH) / 2);
      size_t x_size =
          state.rescale(X - 1, (HEIGHT < WIDTH), (WIDTH - HEIGHT) / 2) - x_top;
      size_t y_size =
          state.rescale(Y - 1, (WIDTH < HEIGHT), (HEIGHT - WIDTH) / 2) - y_top;
      DrawRectangle(x_top, y_top, x_size, y_size, GRAY);
      DrawText("GAME OVER", x_top + x_size / 5, y_top + y_size / 3, x_size / 10,
               RED);
    }

    EndDrawing();
  }
  CloseWindow();
  return 0;
}
