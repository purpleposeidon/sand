

#include <iostream>
#include <deque>
#include <stack>
#include <algorithm>
#include <assert.h>

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#include "CellData.h"
#include "common.h"

using namespace std;




class CellGrid {
private:
  CellType grid[grid_size][grid_size];
  inline bool in_bounds(int x, int y) {
    if (x < 0 || y < 0 || x >= grid_size || y >= grid_size) {
      return false;
    }
    return true;
  }

public:
  CellGrid() {
    for (int x = 0; x < grid_size; x++) {
      for (int y = 0; y < grid_size; y++) {
        grid[x][y] = AIR;
      }
    }
  }

  CellType get(int x, int y, CellType default_type = ROCK) {
    if (in_bounds(x, y)) {
      return grid[x][y];
    }
    return default_type;
  }

  void set(int x, int y, CellType c) {
    if (in_bounds(x, y)) {
      grid[x][y] = c;
    }
  }

};


struct Coord {
  int x, y;
  Coord(int X, int Y) : x(X), y(Y) {}
};


class FluidSimulator {
private:
  CellGrid src;
  deque<Coord> exposed;
  stack<Coord> branch;

  inline void add(int x, int y) {
    if (src.get(x, y, AIR) == EXPOSED_WATER) {
      exposed.push_back(Coord(x, y));
      src.set(x, y, AIR);
    }
  }

  void flood_fill(int x, int y) {
    //Use scanline flood fill aglorithm to locate EXPOSED_WATER edges
    branch.push(Coord(x, y));
    
    while (branch.size()) {
      Coord top = branch.top();
      branch.pop();
      x = top.x, y = top.y;
      //jump to end
      int y1 = y;
      while (src.get(x, y1, AIR) == INACTIVE_WATER) y1--;
      add(x, y1);
      y1++;

      bool span_left = false, span_right = false;
      while (y1 < grid_size && src.get(x, y1, AIR) == INACTIVE_WATER) {
        src.set(x, y1, ROCK);
        add(x-1, y1);
        add(x+1, y1);

        if (!span_left && src.get(x-1, y1, AIR) == INACTIVE_WATER) {
          branch.push(Coord(x-1, y1));
          span_left = true;
        }
        else if (span_left && src.get(x-1, y1, AIR) != INACTIVE_WATER) {
          span_left = false;
        }

        if (!span_right && src.get(x+1, y1, AIR) == INACTIVE_WATER) {
          branch.push(Coord(x+1, y1));
          span_right = true;
        }
        else if (span_right && src.get(x+1, y1, AIR) != INACTIVE_WATER) {
          span_right = false;
        }

        y++;
      }
      add(x, y1);
    }
  }

public:
  void run(CellGrid &orig_grid) {
    src = orig_grid; //Make a copy
    for (int x = 0; x < grid_size; x++) {
      for (int y = 0; y < grid_size; y++) {
        if (orig_grid.get(x, y) == EXPOSED_WATER) {
          exposed.clear();
          flood_fill(x, y);
          sort(exposed.begin(), exposed.end(), height_sorter);
          int s = exposed.size();
          if (s) cout << "Found: " << s << endl;
        }
      }
    }
  }

  static bool height_sorter(Coord a, Coord b) {
    return a.y < b.y;
  }
};



class SandGrid {
private:
  CellGrid a, b;
  CellGrid &now, &next;
  bool parity;
  FluidSimulator fluid_sim;

  void toggle_parity() {
    parity = !parity;
    if (parity) {
      now = b;
      next = a;
    }
    else {
      now = a;
      next = b;
    }
  }

  bool touches_air(int x, int y) {
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        if (dx != 0 && dy != 0) {
          //Don't check diagonals
          continue;
        }
        if (now.get(x+dx, y+dy, ROCK) == AIR) {
          return true;
        }
      }
    }
    return false;
  }
public:
  SandGrid() : now(a), next(b), parity(false) {}

  void draw(SDL_Surface *surface) {
    for (int x = 0; x < grid_size; x++) {
      for (int y = 0; y < grid_size; y++) {
        SDL_Rect rect;
        rect.x = x*block_pixel_size+1;
        rect.y = y*block_pixel_size+1;
        rect.w = block_pixel_size;
        rect.h = block_pixel_size;
        SDL_FillRect(surface, &rect, CellData::color(next.get(x, y)));
      }
    }
    rectangleRGBA(surface, /*dimensions*/ 0, 0, screen_size+1, screen_size+1, /*color*/ 0x80, 0x80, 0x80, 0xFF);

    SDL_UpdateRect(surface, 0, 0, 0, 0); //updates entire screen. Economical!
  }

  void update() {
    memset(&next, 0, sizeof(CellGrid));
    for (int x = 0; x < grid_size; x++) {
      for (int y = 0; y < grid_size; y++) {
        CellType now_cell = now.get(x, y);
        CellType next_cell = now_cell; //By default, blocks carry over
        switch (next_cell) {
          case AIR: continue;
          case BAD_CELL_TYPE:
          case CELL_TYPE_COUNT:
            next_cell = ROCK;
            break;
          case SAND:
            if (now.get(x, y+1) == AIR) {
              //fall down
              next.set(x, y+1, SAND);
              next_cell = AIR;
            }
            break;
          case INACTIVE_WATER:
            if (touches_air(x, y)) {
              next_cell = EXPOSED_WATER;
            }
            break;
          case EXPOSED_WATER:
            if (!touches_air(x, y)) {
              next_cell = INACTIVE_WATER;
            }
            if (now.get(x, y+1, ROCK) == AIR) {
              //fall down :O
              next.set(x, y+1, EXPOSED_WATER);
              next_cell = AIR;
            }
            else if (now.get(x-1, y, ROCK) == AIR
                && now.get(x-1, y+1, ROCK) == AIR) {
              //spill over
              next.set(x-1, y+1, EXPOSED_WATER);
              next_cell = AIR;
            }
            else if (now.get(x+1, y, ROCK) == AIR
                && now.get(x+1, y+1, ROCK) == AIR) {
              //spill over
              next.set(x+1, y+1, EXPOSED_WATER);
              next_cell = AIR;
            }
            break;
          case ROCK: break; //BORING
        }
        next.set(x, y, next_cell);
      }
    }
    fluid_sim.run(now);
    toggle_parity();
  }

  CellType get(int x, int y) {
    return now.get(x, y);
  }

  CellType get(int x, int y, CellType default_type) {
    return now.get(x, y, default_type);
  }

  void set(int x, int y, CellType cell_type) {
    now.set(x, y, cell_type);
  }

  void mouse_set(CellType cell_type) {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    set(mouse_x/block_pixel_size, mouse_y/block_pixel_size, cell_type);
  }
};




void sdl_error() {
  cerr << SDL_GetError() << endl;
  exit(-1);
}


Uint32 draw_timer_callback(Uint32 interval, void *param) {
  SDL_Event event;
  event.type = SDL_USEREVENT;
  SDL_PushEvent(&event);
  return interval;
}


void app_loop(SDL_Surface *screen) {
  SandGrid grid;
  SDL_Event event;
  CellType place_type = SAND;

  SDL_AddTimer(50, draw_timer_callback, NULL); //triggers a draw event every 50 ms

  while (SDL_WaitEvent(&event)) {
    switch (event.type) {
      case SDL_KEYDOWN:
        {
          if (event.key.keysym.unicode == L'q') {
            return;
          }
          place_type = CellData::lookup(event.key.keysym.unicode);
          if (place_type != BAD_CELL_TYPE) {
            grid.mouse_set(place_type);
          }
        }
        break;

      case SDL_MOUSEMOTION:
      case SDL_MOUSEBUTTONDOWN:
        {
          Uint8 mouse_button = 0;

          if (event.type == SDL_MOUSEBUTTONDOWN) {
            mouse_button = event.button.button;
          }
          else {
            mouse_button = event.motion.state;
          }
          if (mouse_button == SDL_BUTTON_LEFT) {
            //Use the previous type
            grid.mouse_set(place_type);
          }
          else if (mouse_button == SDL_BUTTON_RIGHT
              || mouse_button == SDL_BUTTON_MIDDLE) {
            grid.mouse_set(AIR);
          }
        }
        break;

      case SDL_USEREVENT:
        grid.update();
        grid.draw(screen);
        break;

      case SDL_QUIT:
        return;
    }
  }
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
    sdl_error();
  }

  SDL_Surface *screen = SDL_SetVideoMode(screen_size+2, screen_size+2, 0, 0);
  
  if (screen == NULL) {
    sdl_error();
  }

  CellData::init_color(screen);
  SDL_EnableUNICODE(1);
  SDL_WM_SetCaption("sand", "sand");
  atexit(SDL_Quit);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_INTERVAL, SDL_DEFAULT_REPEAT_INTERVAL);

  app_loop(screen);

  return 0;
}



