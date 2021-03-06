#include "CellGrid.h"

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

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
  ticks = 0;
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

inline int rotate_cc(int i) {
  return (i+1) % 8;
}

inline int rotate_co(int i) {
  if (i == 0) return 7;
  return i-1;
}

inline int opposite(int i) {
  return (i+4) % 8;
}

inline float horizontal(int i) {
  if (i == 7 || i == 6 || i == 5) return 0.0;
  if (i == 0 || i == 4) return 0.5;
  if (i == 1 || i == 2 || i == 3) return 1.0;
  throw i;
}

inline float vertical(int i) {
  if (i == 7 || i == 0 || i == 1) return 0.0;
  if (i == 6 || i == 2) return 0.5;
  if (i == 5 || i == 4 || i == 3) return 1.0;
  throw i;
}



int angle(int r, int t) {
  return r - t;
}

int normalize_angle(int r) {
  if (r < 0) return r+8;
  else if (r >= 8) return r - 8;
  return r;
}

/*
7  0  1
6 -1  2
5  4  3
*/

void CellGrid::draw_active_water(Coord here) {
  //Draw the water located at 'here' to water_surface
  //...fancy!
  CellBox cell = CellBox(*this, here, AIR, ROCK, INACTIVE_WATER, INACTIVE_WATER);
  int exposed_count = cell.count(EXPOSED_WATER);
  int inactive_count = cell.count(INACTIVE_WATER);
  int air_count = cell.count(AIR);

  //We draw either a bubble, or a wave.
  bool draw_bubble = true;
  //These are directions, for drawing waves.
  //a1 and a2 indicate what the lines will be drawn through.
  int a1 = -1, a2 = -1;
  //'under' indicates what part is under water
  int under = -1;
 

  //Determine how to proceed
  if (air_count == 3 && exposed_count == 0 && inactive_count == 1) {
    //tower of water?
    int d = cell.find(INACTIVE_WATER);
    a1 = rotate_cc(d);
    a2 = rotate_co(d);
    under = d;
    draw_bubble = false;
  }
  else if (air_count == 2 && exposed_count == 1 && inactive_count == 1) {
    int e = cell.find(EXPOSED_WATER), i = cell.find(INACTIVE_WATER);
    int ei_angle = normalize_angle(angle(e, i));
    if (ei_angle == 4) {
      //opposite
      a1 = rotate_cc(i);
      a2 = rotate_co(i);
      under = i;
    }
    else if (ei_angle == 2 || ei_angle == 6) {
      //adjacent
      a1 = e;
      under = i;
      //a2 is i rotated away from a1
      if (ei_angle == 6) {
        a2 = rotate_cc(i);
      }
      else {
        a2 = rotate_co(i);
      }
    }
    else {
      throw ei_angle;
    }
    draw_bubble = false;
  }
  else if (air_count == 1 && exposed_count == 2 && inactive_count == 1) {
    int e1 = cell.find(EXPOSED_WATER, 0), e2 = cell.find(EXPOSED_WATER, 1);
    if (normalize_angle(angle(e1, e2)) == 4) {
      //We've got two exposeds opposite, with an inactive on one side
      //put the line between the two inactives
      a1 = e1;
      a2 = e2;
      draw_bubble = false;
      under = cell.find(INACTIVE_WATER);
    }
  }
  else if (air_count == 1 && exposed_count == 1 && inactive_count == 2) {
    //similiar to above, except we want two inactives adjacent
    int i1 = cell.find(INACTIVE_WATER, 0), i2 = cell.find(INACTIVE_WATER, 1);
    int n = normalize_angle(angle(i1, i2));
    if (n == 2 || n == 6) {
      draw_bubble = false;
      int e = cell.find(EXPOSED_WATER);
      int between = opposite(cell.find_air());
      a1 = e;
      a2 = opposite(e);
      under = between;
      int direction = normalize_angle(angle(between, a2));
      if (direction == 6) {
        a2 = rotate_cc(a2);
      }
      else {
        a2 = rotate_co(a2);
      }
    }
  }
  

  
  //Now do the drawing
  int seed = (here.x << here.y) + (ticks/15); //used for RNG
  const Uint32 surface_color = CellData::color(EXPOSED_WATER);
  const Uint32 under_water = CellData::color(INACTIVE_WATER);
  SDL_FillRect(water_surface, NULL, CellData::color(AIR));

  if (draw_bubble) {
    if (air_count == 4) {
      //draw drop of water instead
      seed = (here.x * 191) >> 3;
      const int offset = (block_pixel_size/2) - 1;
      const int radius = (block_pixel_size/3)-(seed % 5);
      filledCircleColor(water_surface, offset, offset, radius, under_water);
      circleColor(water_surface, offset, offset, radius, surface_color);
    }
    else {
      //A few random foamy bubbles
      if (inactive_count == 4) {
        //Put them in water
        SDL_FillRect(water_surface, NULL, CellData::color(INACTIVE_WATER));
      }
      //TODO: Maybe have some larger, darker circles in the background?
      for (int bubble_count = 100 + (seed % 4); bubble_count; bubble_count--) {
        float fx = (seed % 20)/20.0;
        seed *= bubble_count+130;
        float fy = (seed % 17)/17.0;
        seed *= 113;
        int radius = (block_pixel_size/10) + (seed % 2);
        int x = block_pixel_size*fx, y = block_pixel_size*fy;
        if (x - radius < 0 || y - radius < 0
          || x+radius+2 >= block_pixel_size || x+radius+2 >= block_pixel_size) {
          continue; //won't fit
          //XXX Some bubbles that don't fit still get drawn?
        }
        filledCircleRGBA(water_surface, x, y, radius, 0xFB, 0xFE, 0xFC, 0x80);
      }
    }
  }
  else if (a1 == -1 || a2 == -1) {
    //Failed somehow, these should have been changed
    SDL_FillRect(water_surface, NULL, surface_color);
  }
  else {
    //TODO: Fancy bezier drawing
    //bezierColor(water_surface, vx, vy, n, 5, surface_color);
    int x1 = horizontal(a1)*block_pixel_size;
    int y1 = vertical(a1)*block_pixel_size;
    int x2 = horizontal(a2)*block_pixel_size;
    int y2 = vertical(a2)*block_pixel_size;
    int xm = 0.5*block_pixel_size;
    int ym = 0.5*block_pixel_size;
    aalineColor(water_surface,
      x1, y1,
      xm, ym,
      surface_color);
    aalineColor(water_surface,
      xm, ym,
      x2, y2,
      surface_color);
    if (under != -1) {
      //dump water
      int xf = ((0.5+horizontal(under))/2.0)*block_pixel_size;
      int yf = ((0.5+vertical(under))/2.0)*block_pixel_size;
      retardo_flood_fill(water_surface, xf, yf, under_water);
      //filledCircleRGBA(water_surface, xf, yf, 3, 0xFF, 0xFF, 0xFF, 0xFF); //where we flood fill from
    }
    else {
      std::cerr << "Note: 'under' not set." << std::endl;
    }
  }

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
      if ((block_pixel_size >= 3) && cell_type == EXPOSED_WATER) {
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


CellBox::CellBox(CellGrid &src, Coord w, CellType d_up, CellType d_down, CellType d_left, CellType d_right) {
  up = src.get(w.up(), d_up);
  down = src.get(w.down(), d_down);
  left = src.get(w.left(), d_left);
  right = src.get(w.right(), d_right);
}



int CellBox::count(CellType c) {
  //rename 'sum'? We like 3-letter words...
  return (up == c) + (down == c) + (left == c) + (right == c);
}

bool CellBox::any(CellType c) {
  return up == c || down == c || left == c || right == c;
}

bool CellBox::all(CellType c) {
  return up == c && down == c && left == c && right == c;
}

int CellBox::find(CellType c, int skip) {
  for (int angle = 0; angle < 8; angle += 2) {
    if (get(angle) == c) {
      if (skip == 0) {
        return angle;
      }
      skip--;
    }
  } 

  return -1;
}

int CellBox::find_air(int skip) {
  for (int angle = 0; angle < 8; angle += 2) {
    if (get(angle) != EXPOSED_WATER && get(angle) != INACTIVE_WATER) {
      if (skip == 0) {
        return angle;
      }
      skip--;
    }
  } 

  return -1;
}

CellType CellBox::get(int d) {
  switch (d) {
    case 0: return up;
    case 2: return right;
    case 4: return down;
    case 6: return left;
    default: throw d;
  }
}
