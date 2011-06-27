#include "common.h"
#include "SDL/SDL.h"

#include <stack>
using namespace std;

void sdl_error() {
  cerr << SDL_GetError() << endl;
  exit(-1);
}



Coord::Coord(int X, int Y) : x(X), y(Y) {}

bool Coord::operator<(const Coord &b) const {
  if (y == b.y) {
    return x < b.x;
  }
  return y < b.y;
}

bool Coord::operator==(const Coord&b) const {
  return x == b.x && y == b.y;
}

std::ostream& operator<<(std::ostream &fd, const Coord &r) {
  fd << r.x << "," << r.y;
  return fd;
}

Coord Coord::up() { return Coord(x, y-1); }
Coord Coord::down() { return Coord(x, y+1); }
Coord Coord::left() { return Coord(x-1, y); }
Coord Coord::right() { return Coord(x+1, y); }



/*
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 */
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

void retardo_flood_fill(SDL_Surface *surface, int x, int y, Uint32 color) {
  //Hey, guess what? These libraries have no flood fill function for some inscrutably retarded reason
  std::stack<Coord> s;
  s.push(Coord(x, y));
  Uint32 orig = getpixel(surface, x, y);
  int width = surface->w, height = surface->h;
  while (s.size()) {
    x = s.top().x;
    y = s.top().y;
    s.pop();
    while (x >= 0 && getpixel(surface, x, y) == orig) x--;
    x++;
    bool span_up = false, span_down = false;
    while (x < width && getpixel(surface, x, y) == orig) {
      putpixel(surface, x, y, color);
      if (y > 0) {
        bool eq_orig = getpixel(surface, x, y-1) == orig;
        if (!span_up && eq_orig) {
          s.push(Coord(x, y-1));
          span_up = true;
        }
        else if (span_up && !eq_orig) {
          span_up = false;
        }
      }

      if (y < height - 1) {
        bool eq_orig = getpixel(surface, x, y+1) == orig;
        if (!span_down && eq_orig) {
          s.push(Coord(x, y+1));
          span_down = true;
        }
        else if (span_down && !eq_orig) {
          span_down = false;
        }
      }

      x++;
    }
  }
}
