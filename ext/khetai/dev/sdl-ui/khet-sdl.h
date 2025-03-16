#ifndef KHETSDL_H
#define KHETSDL_H

#include <SDL3/SDL.h>

#define ROWS 8
#define COLS 10

#define WINDOW_WIDTH  (800)
#define WINDOW_HEIGHT (WINDOW_WIDTH * 0.8)
#define WINDOW_BUFFER (30)
#define BOARD_WIDTH   (WINDOW_WIDTH - (WINDOW_BUFFER * 2))
#define BOARD_HEIGHT  (BOARD_WIDTH * 0.8)
#define SQUARE_SIZE   (BOARD_WIDTH / 10)
#define PIECE_SIZE    (SQUARE_SIZE * 0.8)

enum MovePermission {
    S,
    R,
    B
};

static const int square_colors[8][10] = {
    {B, S, B, B, B, B, B, B, R, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, S, B, B, B, B, B, B, R, B}
};

typedef struct {
    double x, y;
} Point;

typedef struct {
    int row, col;
} Position;

typedef enum {
    SILVER_SDL,
    RED_SDL
} Color;

typedef enum {
    ANUBIS_SDL,
    PYRAMID_SDL,
    SCARAB_SDL,
    PHARAOH_SDL,
    LASER_SDL
} PieceType_SDL;

typedef enum {
    NORTH_SDL,
    EAST_SDL,
    SOUTH_SDL,
    WEST_SDL
} Orientation_SDL;

typedef struct {
    PieceType_SDL piece_type;
    Color color;
    Orientation_SDL orientation;
} Piece_SDL;

typedef struct {
    Point point;
    Position position;
    Piece_SDL *piece;
} Square_SDL;

typedef struct {
    SDL_Window *win;
    SDL_Renderer *ren;
    Square_SDL board[ROWS][COLS];
    Position clicked_pos;
    Position selected_pos;
    int valid_squares[ROWS][COLS];
    bool clicked;
    bool selected;
    bool call_ai;
} AppState;

#endif