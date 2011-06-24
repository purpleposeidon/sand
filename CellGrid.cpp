#include "CellGrid.h"

#include <SDL/SDL.h>

#include "common.h"


CellGrid::CellGrid() {
  for (int x = 0; x < grid_size; x++) {
    for (int y = 0; y < grid_size; y++) {
      grid[x][y] = AIR;
    }
  }
  water_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
        block_pixel_size, block_pixel_size, /*dimensions*/
        32, 0, 0, 0, 0 /*bits per pixel, RGBA masks*/);
  if (water_surface == NULL) {
    sdl_error();
  }
  SDL_FillRect(water_surface, NULL, CellData::color(EXPOSED_WATER));
}


CellGrid::~CellGrid() {
  //SDL_FreeSurface(water_surface); //Already gets freed?
}

CellType CellGrid::get(int x, int y, CellType default_type) {
  if (in_bounds(x, y)) {
    return grid[x][y];
  }
  return default_type;
}


void CellGrid::draw_active_water(Coord here) {
  //Draw the water located at 'here' to water_surface
  //...fancy!
  
  /*const Uint32 surface_color = CellData::color(EXPOSED_WATER);
  const Uint32 under_water = CellData::color(INACTIVE_WATER);

  const int n = 3;
  Sint16 vx[n], vy[n];
  bezierColor(water_surface, vx, vy, n, 5, surface_color);*/
}


void CellGrid::draw(SDL_Surface *surface) {
  for (int x = 0; x < grid_size; x++) {
    for (int y = 0; y < grid_size; y++) {
      SDL_Rect rect;
      rect.x = x*block_pixel_size+1;
      rect.y = y*block_pixel_size+1;
      rect.w = block_pixel_size;
      rect.h = block_pixel_size;
      CellType cell_type = get(x, y);
      if (0 && cell_type == EXPOSED_WATER) {
        draw_active_water(Coord(x, y));
        SDL_BlitSurface(water_surface, NULL, surface, &rect);
      }
      else {
        SDL_FillRect(surface, &rect, CellData::color(cell_type));
      }
    }
  }
}






CellBox::CellBox(CellGrid &src, Coord w) {
  up = src.get(w.up());
  down = src.get(w.down());
  left = src.get(w.left());
  right = src.get(w.right());
}