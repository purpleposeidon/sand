#include "common.h"
#include "SDL/SDL.h"

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

//These are totally correct.
Coord Coord::up() { return Coord(x, y+1); }
Coord Coord::down() { return Coord(x, y-1); }
Coord Coord::left() { return Coord(x-1, y); }
Coord Coord::right() { return Coord(x+1, y); }