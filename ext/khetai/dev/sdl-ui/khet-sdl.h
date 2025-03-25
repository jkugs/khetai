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
#define LASER_SPEED   250

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
    int row, col;
} Position;

typedef enum {
    SILVER_SDL,
    RED_SDL
} PlayerColor_SDL;

typedef enum {
    ANUBIS_SDL,
    PYRAMID_SDL,
    SCARAB_SDL,
    PHARAOH_SDL,
    SPHINX_SDL
} PieceType_SDL;

typedef enum {
    NORTH_SDL,
    EAST_SDL,
    SOUTH_SDL,
    WEST_SDL
} Orientation_SDL;

typedef enum {
    REFLECT,
    ABSORB,
    HIT
} LaserInteraction;

typedef struct {
    SDL_FPoint p1, p2;
    LaserInteraction interaction;
} PieceSide;

typedef struct {
    PieceType_SDL piece_type;
    PlayerColor_SDL color;
    Orientation_SDL orientation;
    PieceSide sides[4];
} Piece_SDL;

typedef struct {
    SDL_FPoint point;
    Position position;
    Piece_SDL *piece;
} Square_SDL;

typedef struct {
    SDL_FPoint p1, p2;
} LaserSegment;

typedef enum {
    CONTINUE,
    NEW_SEGMENT,
    STOP
} LaserNextStep;

typedef struct {
    LaserSegment segments[100];
    Orientation_SDL direction;
    int num_segments;
    LaserNextStep next_step;
} Laser;

typedef struct {
    SDL_Window *win;
    SDL_Renderer *ren;
    Square_SDL board[ROWS][COLS];
    Position clicked_pos;
    Position selected_pos;
    Laser laser;
    Uint64 last_tick;
    int valid_squares[ROWS][COLS];
    float delta_time;
    bool clicked;
    bool selected;
    bool call_ai;
    bool drawing_laser;
} AppState;

#endif