
#ifndef COMMON_H
#define COMMON_H


const int grid_size = 80;
const int block_pixel_size = 10;
const int screen_size = grid_size*block_pixel_size;
const int update_speed = 20;

inline int sign(int x) {
  return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

#endif /* COMMON_H */

