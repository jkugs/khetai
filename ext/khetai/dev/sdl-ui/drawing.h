#ifndef DRAWING_H
#define DRAWING_H

#include "khet-sdl.h"

#define BLACK_COLOR        ((SDL_Color){0, 0, 0, 255})
#define GRAY_COLOR         ((SDL_Color){170, 170, 170, 255})
#define OVERLAY_COLOR      ((SDL_Color){0, 0, 0, 40})
#define DARK_OVERLAY_COLOR ((SDL_Color){30, 30, 30, 175})
#define LIGHT_YELLOW_COLOR ((SDL_Color){255, 230, 150, 150})
#define LIGHT_GREEN_COLOR  ((SDL_Color){150, 230, 155, 150})
#define LIGHT_RED_COLOR    ((SDL_Color){255, 140, 140, 80})
#define RED_FCOLOR         ((SDL_FColor){0.5f, 0.1f, 0.1f, 1.0f})
#define SILVER_FCOLOR      ((SDL_FColor){0.4f, 0.4f, 0.4f, 1.0f})
#define MIRROR_FCOLOR      ((SDL_FColor){0.6f, 0.7f, 1.0f, 1.0f})
#define BLACK_FCOLOR       ((SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f})
#define LASER_FCOLOR       ((SDL_FColor){0.5f, 0.2f, 0.2f, 0.7f})

#ifndef M_PI
#define M_PI 3.1415927f
#endif

void draw(AppState *as);
void draw_selected_valid_highlights(AppState *as);
void draw_ai_move_highlights(AppState *as);
void draw_board_grid(AppState *as);
void draw_pieces(AppState *as);
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
void set_render_draw_color(SDL_Renderer *ren, SDL_Color color);

#endif