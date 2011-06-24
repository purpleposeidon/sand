
#ifndef COMMON_H
#define COMMON_H

#include <iostream>


const int grid_size = 80;
const int block_pixel_size = 10;
const int screen_size = grid_size*block_pixel_size;
const int update_speed = 20;

inline int sign(int x) {
  return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

void sdl_error();

struct Coord {
  int x, y;
  Coord(int X, int Y);
  bool operator<(const Coord &b) const;
  bool operator==(const Coord&b) const;
  friend std::ostream& operator<<(std::ostream &fd, const Coord &r);
  
  Coord up();
  Coord down();
  Coord left();
  Coord right();
};

#endif /* COMMON_H */

