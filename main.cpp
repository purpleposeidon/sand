

#include <iostream>
#include <iterator>
#include <assert.h>

#include <SDL/SDL.h>

#include "CellData.h"
#include "common.h"
#include "Physics.h"

using namespace std;






Uint32 draw_timer_callback(Uint32 interval, void *param) {
  SDL_Event event;
  event.type = SDL_USEREVENT;
  SDL_PushEvent(&event);
  return interval;
}


void app_loop(SDL_Surface *screen) {
  SandGrid grid;
  SDL_Event event;
  CellType place_type = SAND;
  bool do_update = true;

  SDL_AddTimer(update_speed, draw_timer_callback, NULL); //triggers a draw event every 50 ms

  while (SDL_WaitEvent(&event)) {
    switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.unicode == L'q') {
          return;
        }
        else {
          CellType new_type = CellData::lookup(event.key.keysym.unicode);
          if (new_type != BAD_CELL_TYPE) {
            place_type = new_type;
            grid.mouse_set(place_type);
          }
        }
        break;

      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_SPACE) {
          do_update = !do_update;
        }
        if (event.key.keysym.sym == SDLK_PERIOD) {
          if (!do_update) {
            grid.update(true);
            grid.draw(screen);
          }
        }

      case SDL_MOUSEMOTION:
      case SDL_MOUSEBUTTONDOWN:
        {
          Uint8 mouse_button = 0;

          if (event.type == SDL_MOUSEBUTTONDOWN) {
            mouse_button = event.button.button;
          }
          else {
            mouse_button = event.motion.state;
          }
          if (mouse_button == SDL_BUTTON_LEFT) {
            //Use the previous type
            grid.mouse_set(place_type);
          }
          else if (mouse_button == SDL_BUTTON_MIDDLE) {
            grid.mouse_set(AIR);
          }
          else if (mouse_button == SDL_BUTTON_RIGHT) {
            grid.mouse_set(BAD_CELL_TYPE);
          }
        }
        break;

      case SDL_USEREVENT:
        grid.update(do_update);
        grid.draw(screen);
        break;

      case SDL_QUIT:
        return;
    }
  }
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
    sdl_error();
  }

  SDL_Surface *screen = SDL_SetVideoMode(screen_size+2, screen_size+2, 0, 0);
  
  if (screen == NULL) {
    sdl_error();
  }

  CellData::init_color(screen);
  SDL_EnableUNICODE(1);
  SDL_WM_SetCaption("sand", "sand");
  atexit(SDL_Quit);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_INTERVAL, SDL_DEFAULT_REPEAT_INTERVAL);

  app_loop(screen);

  return 0;
}



