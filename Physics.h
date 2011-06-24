
#ifndef PHYSICS_H
#define PHYSICS_H


#include <deque>
#include <stack>
#include <algorithm>

#include <SDL/SDL_gfxPrimitives.h>

#include "common.h"
#include "CellGrid.h"


class FluidSimulator {
private:
  CellGrid src;
  std::deque<Coord> exposed;
  std::stack<Coord> branch;

  void add(int x, int y);
  void push(int x, int y);
  void pop(int &x, int &y);

  void flood_fill(int x, int y);
  static bool height_sorter(Coord a, Coord b);
  void move_water(CellGrid &grid, Coord move, Coord target);
public:
  void run(CellGrid &orig_grid);
};



class SandGrid {
private:
  CellGrid a, b;
  CellGrid &now, &next;
  bool parity;
  FluidSimulator fluid_sim;

  void toggle_parity();
  bool touches_air(int x, int y);
  void simple_physics_pass();
  void replicator_physics_pass();
public:
  SandGrid();
  void draw(SDL_Surface *surface);
  void update(bool do_physics);

  CellType get(int x, int y);
  CellType get(int x, int y, CellType default_type);
  void set(int x, int y, CellType cell_type);
  void mouse_set(CellType cell_type);
};





#endif /* PHYSICS_H */

