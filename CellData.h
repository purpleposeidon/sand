
#ifndef CELLDATA_H
#define CELLDATA_H

#include <SDL/SDL.h>
#include <wchar.h>

enum CellType {
  BAD_CELL_TYPE = -1,
  FIRST_CELL_TYPE = 0,
  AIR = FIRST_CELL_TYPE,
  SAND,
  ROCK,
  EXPOSED_WATER,
  INACTIVE_WATER,
  CELL_TYPE_COUNT //Leave last
};

struct CellData {
// private:
  CellType data_id;
  const wchar_t *data_name;
  SDL_Color data_sdl_color;
  Uint32 data_uint_color;

  void map_color(SDL_Surface *surface);

// public:
  static void init_color(SDL_Surface *screen);
  static Uint32 color(CellType c);
  static const wchar_t *name(CellType c);
  static CellType lookup(wchar_t initial_letter);
};




#endif /* CELLDATA_H */

