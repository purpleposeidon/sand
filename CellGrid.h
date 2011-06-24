
#ifndef CELLGRID_H
#define CELLGRID_H

#include "CellData.h"
#include "common.h"

class CellGrid {
private:
  CellType grid[grid_size][grid_size];
  SDL_Surface *water_surface;
  
  void draw_active_water(Coord here);
  inline bool in_bounds(int x, int y) {
    if (x < 0 || y < 0 || x >= grid_size || y >= grid_size) {
      return false;
    }
    return true;
  }

public:
  CellGrid();
  ~CellGrid();

  CellType get(int x, int y, CellType default_type = ROCK);
  

  inline void set(int x, int y, CellType c) {
    if (in_bounds(x, y)) {
      grid[x][y] = c;
    }
  }

  inline CellType get(Coord p, CellType default_type = ROCK) { return get(p.x, p.y, default_type); }
  inline void set(Coord p, CellType c) { set(p.x, p.y, c); }

  
  void draw(SDL_Surface *surface);

  int ticks;
};

struct CellBox {
public:
  CellType up, down, left, right;
  CellBox(CellGrid &src, Coord w);
  bool any(CellType c);
  bool all(CellType c);
  int count(CellType c);
  int find(CellType c, int skip = 0);
  int find_air(int skip = 0);
  CellType get(int d);
};

#endif /* CELLGRID_H */

