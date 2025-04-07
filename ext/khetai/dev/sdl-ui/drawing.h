#ifndef DRAWING_H
#define DRAWING_H

#include "khet-sdl.h"

#define RED_COLOR    ((SDL_FColor){0.5f, 0.1f, 0.1f, 1.0f})
#define SILVER_COLOR ((SDL_FColor){0.4f, 0.4f, 0.4f, 1.0f})
#define MIRROR_COLOR ((SDL_FColor){0.6f, 0.7f, 1.0f, 1.0f})
#define BLACK_COLOR  ((SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f})
#define LASER_COLOR  ((SDL_FColor){0.5f, 0.2f, 0.2f, 0.7f})

#ifndef M_PI
#define M_PI 3.1415927f
#endif

void draw(AppState *as);
void draw_inner_square(AppState *as, int row, int col, SDL_FColor color);
void draw_piece(AppState *as, int row, int col);
void draw_pyramid(AppState *as, int row, int col);
void draw_scarab(AppState *as, int row, int col);
void draw_anubis(AppState *as, int row, int col);
void draw_pharaoh(AppState *as, int row, int col);
void draw_sphinx(AppState *as, int row, int col);
void draw_laser_animation(AppState *as);
void draw_thinking_overlay(AppState *as);
void draw_end_overlay(AppState *as);

#endif