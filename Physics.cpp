#include "Physics.h"
#include <iostream>

void FluidSimulator::add(int x, int y) {
  if (src.get(x, y, AIR) == EXPOSED_WATER) {
    exposed.push_back(Coord(x, y));
    src.set(x, y, AIR);
  }
}

void FluidSimulator::push(int x, int y) {
  branch.push(Coord(x, y));
}

void FluidSimulator::pop(int &x, int &y) {
  Coord here = branch.top();
  x = here.x, y = here.y;
  branch.pop();
}

void FluidSimulator::flood_fill(int x, int y) {
  //Use scanline flood fill aglorithm to locate EXPOSED_WATER edges
  //This algorithm is from Somewhere on Siggraph I Think.
  //Sorry, I lost the original URL...
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


bool FluidSimulator::height_sorter(Coord a, Coord b) {
  static char parity = 0;
  if (a.y == b.y) {
    parity++;
    return (parity % 2) ? a.x < b.y : a.x > b.y;
  }
  return a.y < b.y;
}


void FluidSimulator::move_water(CellGrid &grid, Coord move, Coord target) {
  //Try to move water to target
  //Check that it isn't a lame movement
  if (move.y + 1 >= target.y) return; 
  //We need to be considerate of the order we check.
  static char parity = 0;
  parity++; //XXX should check if this actually does anything
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
    return; //Didn't work
  }
  grid.set(move, AIR); //Did work
}


void FluidSimulator::run(CellGrid &orig_grid) {
  src = orig_grid; //Make a copy
  for (int x = 0; x < grid_size; x++) {
    for (int y = 0; y < grid_size; y++) {
      if (src.get(x, y) == EXPOSED_WATER) {
        exposed.clear();
        flood_fill(x, y);
        sort(exposed.begin(), exposed.end()); //Higher water is at the front.
        unique(exposed.begin(), exposed.end());
        
        /*
        Now we move some water.
        Exposed water at the top of the list is moved next to the exposed water
        at the bottom of the list.
        */
        while (exposed.size() > 2) {
          move_water(orig_grid, exposed.front(), exposed.back());
          exposed.pop_front();
          exposed.pop_back();
        }
      }
    }
  }
}













void SandGrid::toggle_parity() {
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

bool SandGrid::touches_air(int x, int y) {
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

SandGrid::SandGrid() : now(a), next(b), parity(false) {}

void SandGrid::draw(SDL_Surface *surface) {
  next.draw(surface);
  rectangleRGBA(surface, /*dimensions*/ 0, 0, screen_size+1, screen_size+1, /*color*/ 0x80, 0x80, 0x80, 0xFF);
  SDL_UpdateRect(surface, 0, 0, 0, 0); //updates entire screen. Economical!
}

void SandGrid::simple_physics_pass() {
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
        default: break;
      }
      next.set(x, y, next_cell);
    }
  }
}

void SandGrid::replicator_physics_pass() {
  for (int x = 0; x < grid_size; x++) {
    for (int y = 0; y < grid_size; y++) {
      switch (next.get(x, y, AIR)) {
        case CLONER:
          if (now.get(x, y+1, ROCK) == AIR || now.get(x, y+1, ROCK) == CLONER) {
            next.set(x, y+1, now.get(x, y-1, CLONER));
          }
          break;
        case DESTROYER:
          for (int dx = -1; dx != 2; dx++) {
            for (int dy = -1; dy != 2; dy++) {
              if (dx == 0 && dy == 0) continue;
              next.set(x+dx, y+dy, AIR);
            }
          }
          break;
        default: break;
      }
    }
  }
}

void SandGrid::update(bool do_physics) {
  next = now;
  if (do_physics) {
    simple_physics_pass();
    replicator_physics_pass();
    fluid_sim.run(now);
  }
  toggle_parity();
}

CellType SandGrid::get(int x, int y) {
  return now.get(x, y);
}

CellType SandGrid::get(int x, int y, CellType default_type) {
  return now.get(x, y, default_type);
}

void SandGrid::set(int x, int y, CellType cell_type) {
  now.set(x, y, cell_type);
}

void SandGrid::mouse_set(CellType cell_type) {
  int mouse_x, mouse_y;
  SDL_GetMouseState(&mouse_x, &mouse_y);
  mouse_x /= block_pixel_size;
  mouse_y /= block_pixel_size;
  if (cell_type == BAD_CELL_TYPE) {
    std::cout << "Mouse at: " << mouse_x << "," << mouse_y << std::endl;
    SDL_Delay(1000);
    return;
  }
  set(mouse_x, mouse_y, cell_type);
}