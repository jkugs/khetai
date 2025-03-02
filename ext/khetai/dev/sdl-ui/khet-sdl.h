#ifndef KHETSDL_H
#define KHETSDL_H

#include <SDL3/SDL.h>

#define ROWS 8
#define COLS 10

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT (int)(WINDOW_WIDTH * 0.8)
#define WINDOW_BUFFER 30
#define BOARD_WIDTH   (WINDOW_WIDTH - (WINDOW_BUFFER * 2))
#define BOARD_HEIGHT  (int)(BOARD_WIDTH * 0.8)
#define SQUARE_SIZE   (BOARD_WIDTH / 10)
#define PIECE_SIZE    (int)(SQUARE_SIZE * 0.8)

typedef struct {
    double x, y;
} Point;

typedef struct {
    int row, col;
} Position;

typedef enum {
    SILVER,
    RED
} Color;

typedef enum {
    ANUBIS,
    PYRAMID,
    SCARAB,
    PHARAOH
} PieceType;

typedef enum {
    NORTH,
    EAST,
    SOUTH,
    WEST
} Direction;

typedef struct {
    Point point;
    Position position;
    PieceType piece_type;
    Color color;
    Direction direction;
} Piece;

typedef struct {
    SDL_Window *win;
    SDL_Renderer *ren;
    Piece board[8][10];
    Position clicked_pos;
    Position cur_clicked_pos;
    bool clicked;
} AppState;

#endif