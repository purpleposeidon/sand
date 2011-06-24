

#include <iostream>
#include <iterator>
#include <deque>
#include <stack>
#include <algorithm>
#include <assert.h>

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#include "CellData.h"
#include "common.h"

using namespace std;


struct Coord {
  int x, y;
  Coord(int X, int Y) : x(X), y(Y) {}
  bool operator<(const Coord &b) const {
    if (y == b.y) {
      return x < b.x;
    }
    return y < b.y;
  }

  bool operator==(const Coord&b) const {
    return x == b.x && y == b.y;
  }

  friend std::ostream& operator<<(std::ostream &fd, const Coord &r) {
    fd << r.x << "," << r.y;
    return fd;
  }

  Coord up() { return Coord(x, y+1); }
  Coord down() { return Coord(x, y-1); }
  Coord left() { return Coord(x-1, y); }
  Coord right() { return Coord(x+1, y); }
};




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

  CellType get(Coord p, CellType default_type = ROCK) { return get(p.x, p.y, default_type); }
  void set(Coord p, CellType c) { set(p.x, p.y, c); }

  void draw(SDL_Surface *surface) {
    for (int x = 0; x < grid_size; x++) {
      for (int y = 0; y < grid_size; y++) {
        SDL_Rect rect;
        rect.x = x*block_pixel_size+1;
        rect.y = y*block_pixel_size+1;
        rect.w = block_pixel_size;
        rect.h = block_pixel_size;
        SDL_FillRect(surface, &rect, CellData::color(get(x, y)));
      }
    }
  }

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

  inline void push(int x, int y) {
    branch.push(Coord(x, y));
  }

  inline void pop(int &x, int &y) {
    Coord here = branch.top();
    x = here.x, y = here.y;
    branch.pop();
  }

  void flood_fill(int x, int y) {
    //Use scanline flood fill aglorithm to locate EXPOSED_WATER edges
    //This algorithm is From Somewhere on Siggraph.
    add(x, y);
    //We should be at an EXPOSED_WATER. Find the water we're covering.
    push(x+1, y);
    push(x-1, y);
    push(x, y+1);
    push(x, y-1);
    
    while (branch.size()) {
      pop(x, y);
      //jump to end
      while (src.get(x, y, AIR) == INACTIVE_WATER) y--;
      add(x, y);
      y++;

      bool span_left = false, span_right = false;
      while (y < grid_size && src.get(x, y, AIR) == INACTIVE_WATER) {
        src.set(x, y, ROCK);
        add(x-1, y);
        add(x+1, y);

        if (!span_left && src.get(x-1, y, AIR) == INACTIVE_WATER) {
          push(x-1, y);
          span_left = true;
        }
        else if (span_left && src.get(x-1, y, AIR) != INACTIVE_WATER) {
          span_left = false;
        }

        if (!span_right && src.get(x+1, y, AIR) == INACTIVE_WATER) {
          push(x+1, y);
          span_right = true;
        }
        else if (span_right && src.get(x+1, y, AIR) != INACTIVE_WATER) {
          span_right = false;
        }

        y++;
      }
      add(x, y);
    }
  }

  static bool height_sorter(Coord a, Coord b) {
    static char parity = 0;
    if (a.y == b.y) {
      parity++;
      return (parity % 2) ? a.x < b.y : a.x > b.y;
    }
    return a.y < b.y;
  }

  inline void move_water(CellGrid &grid, Coord move, Coord target) {
    //Try to move water to target
    //Check that it isn't a lame movement
    if (move.y + 1 >= target.y) return; 
    //We need to be considerate of the order we check.
    static char parity = 0;
    parity++;
    bool EVEN = parity % 2, ODD = !EVEN;
    if (grid.get(target.down(), ROCK) == AIR) {
      //(I don't expect this will happen ever?)
      grid.set(target.down(), EXPOSED_WATER);
    }
    else if (EVEN && grid.get(target.left(), ROCK) == AIR) {
      grid.set(target.left(), EXPOSED_WATER);
    }
    else if (EVEN && grid.get(target.right(), ROCK) == AIR) {
      grid.set(target.right(), EXPOSED_WATER);
    }
    else if (ODD && grid.get(target.left(), ROCK) == AIR) {
      grid.set(target.left(), EXPOSED_WATER);
    }
    else if (ODD && grid.get(target.right(), ROCK) == AIR) {
      grid.set(target.right(), EXPOSED_WATER);
    }
    else if (grid.get(target.up(), ROCK) == AIR) {
      grid.set(target.up(), EXPOSED_WATER);
    }
    else {
      cout << "Didn't move any water!" << endl;
      return; //Didn't work
    }
    cout << "Water moved!" << endl;
    grid.set(move, AIR); //Did work
  }
public:
  void run(CellGrid &orig_grid) {
    src = orig_grid; //Make a copy
    for (int x = 0; x < grid_size; x++) {
      for (int y = 0; y < grid_size; y++) {
        if (src.get(x, y) == EXPOSED_WATER) {
          exposed.clear();
          flood_fill(x, y);
          sort(exposed.begin(), exposed.end()); //Higher water is at the front.
          unique(exposed.begin(), exposed.end());
          
          cout << "Exposed water blocks: ";
          ostream_iterator<Coord> output(cout, " ");
          copy(exposed.begin(), exposed.end(), output);
          cout << endl;
          

          /*
          Now we move some water.
          Exposed water at the top of the list is moved next to the exposed water
          at the bottom of the list.
          */
          //const float viscocity = 0.8; //What proportion of the water will move to the bottom
          //int count = viscocity*exposed.size();
          //cout << "Will move " << count << " waters" << endl;
          while (/*count-- &&*/ exposed.size() > 2) {
            move_water(orig_grid, exposed.front(), exposed.back());
            exposed.pop_front();
            exposed.pop_back();
          }
        }
      }
    }
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
    next.draw(surface);
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
          case CLONER:
            next.set(x, y+1, now.get(x, y-1, CLONER));
            break;
          case DESTROYER:
            for (int dx = -1; dx != 2; dx++) {
              for (int dy = -1; dy != 2; dy++) {
                next.set(x+dx, y+dy, AIR);
              }
            }
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
    mouse_x /= block_pixel_size;
    mouse_y /= block_pixel_size;
    if (cell_type == BAD_CELL_TYPE) {
      cout << "Mouse at: " << mouse_x << "," << mouse_y << endl;
      SDL_Delay(1000);
      return;
    }
    set(mouse_x, mouse_y, cell_type);
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
          CellType new_type = CellData::lookup(event.key.keysym.unicode);
          if (new_type != BAD_CELL_TYPE) {
            place_type = new_type;
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
          else if (mouse_button == SDL_BUTTON_MIDDLE) {
            grid.mouse_set(AIR);
          }
          else if (mouse_button == SDL_BUTTON_RIGHT) {
            grid.mouse_set(BAD_CELL_TYPE);
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



