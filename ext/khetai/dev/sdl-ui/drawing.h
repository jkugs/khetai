#ifndef DRAWING_H
#define DRAWING_H

#include "khet-sdl.h"

#define RED_COLOR    ((SDL_FColor){0.5f, 0.1f, 0.1f, 1.0f})
#define SILVER_COLOR ((SDL_FColor){0.4f, 0.4f, 0.4f, 1.0f})
#define MIRROR_COLOR ((SDL_FColor){0.6f, 0.7f, 1.0f, 1.0f})
#define BLACK_COLOR  ((SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f})
#define LIGHT_GREEN  ((SDL_FColor){0.6f, 0.9f, 0.6f, 0.7f})

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void draw(void *app_state_ptr);
void draw_inner_square(void *app_state_ptr, int row, int col, SDL_FColor color);
void draw_piece(void *app_state_ptr, int row, int col);
void draw_pyramid(void *app_state_ptr, int row, int col);
void draw_scarab(void *app_state_ptr, int row, int col);
void draw_anubis(void *app_state_ptr, int row, int col);
void draw_pharaoh(void *app_state_ptr, int row, int col);
void draw_laser(void *app_state_ptr, int row, int col);

#endif