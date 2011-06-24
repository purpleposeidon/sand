#include "CellData.h"
#include "assert.h"
#include <iostream>
using namespace std;


CellData cell_data[] = {
  {AIR, L"air", {0x00, 0x00, 0x00}},
  {SAND, L"sand", {0xF7, 0xE1, 0x8F}},
  {ROCK, L"rock", {0x5C, 0x56, 0x4B}},
  {EXPOSED_WATER, L"water", {0x84, 0xA5, 0xD5}},
  {INACTIVE_WATER, L"inactive water", {0x2A, 0x4E, 0x80}},
  {CLONER, L"cloner", {0x83, 0x80, 0x26}},
  {DESTROYER, L"destroyer", {0xE5, 0xA9, 0x7D}},
};


void CellData::map_color(SDL_Surface *surface) {
  data_uint_color = SDL_MapRGB(surface->format, data_sdl_color.r, data_sdl_color.g, data_sdl_color.b);
}

void CellData::init_color(SDL_Surface *screen) {
  for (int cell_type = FIRST_CELL_TYPE; cell_type < CELL_TYPE_COUNT; cell_type++) {
    cell_data[cell_type].map_color(screen);
  }
}

Uint32 CellData::color(CellType c) {
  return cell_data[c].data_uint_color;
}

const wchar_t *CellData::name(CellType c) {
  return cell_data[c].data_name;
}

CellType CellData::lookup(wchar_t initial_letter) {
  for (int i = FIRST_CELL_TYPE; i < CELL_TYPE_COUNT; i++) {
    CellType cell_type = (CellType)i;
    if (name(cell_type)[0] == initial_letter) {
      return cell_type;
    }
  }
  return BAD_CELL_TYPE;
}

