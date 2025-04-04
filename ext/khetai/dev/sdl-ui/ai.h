#ifndef AI_H
#define AI_H

#include "khet-sdl.h"
#include "khetai_lib.h"

Move call_ai_move(Square_SDL board[8][10]);
void get_row_col(int index, int *row, int *col);

#endif