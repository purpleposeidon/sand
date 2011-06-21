

#include <SDL/SDL.h>
#include <iostream>
#include <assert.h>

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




class SandGrid {
private:
  CellGrid a, b;
  CellGrid &now, &next;
  bool parity;

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

public:
  SandGrid() : now(a), next(b), parity(false) {}

  void draw(SDL_Surface *surface) {
    for (int x = 0; x < grid_size; x++) {
      for (int y = 0; y < grid_size; y++) {
        SDL_Rect rect;
        rect.x = x*block_pixel_size;
        rect.y = y*block_pixel_size;
        rect.w = block_pixel_size;
        rect.h = block_pixel_size;
        SDL_FillRect(surface, &rect, CellData::color(next.get(x, y)));
      }
    }

    SDL_UpdateRect(surface, 0, 0, 0, 0); //updates entire screen. Economical!
  }

  void update() {
    memset(&next, 0, sizeof(CellGrid));
    for (int x = 0; x < grid_size; x++) {
      for (int y = 0; y < grid_size; y++) {
        CellType now_cell = now.get(x, y);
        CellType next_cell = now_cell; //By default, blocks carry over
        if (now_cell == AIR) continue;
        if (now_cell == SAND) {
          if (now.get(x, y+1) == AIR) {
            //fall down
            next.set(x, y+1, SAND);
            next_cell = AIR;
          }
        }
        next.set(x, y, next_cell);
      }
    }
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

  SDL_Surface *screen = SDL_SetVideoMode(screen_size, screen_size, 0, 0);
  
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



