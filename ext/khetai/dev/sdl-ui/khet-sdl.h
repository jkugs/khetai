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
#define LASER_SPEED   300 // pixels per second

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
    {R, S, B, B, B, B, B, B, R, B}};

typedef struct Position {
    int row, col;
} Position;

typedef enum PlayerColor_SDL {
    SILVER_SDL,
    RED_SDL
} PlayerColor_SDL;

typedef enum PieceType_SDL {
    ANUBIS_SDL,
    PYRAMID_SDL,
    SCARAB_SDL,
    PHARAOH_SDL,
    SPHINX_SDL
} PieceType_SDL;

typedef enum Orientation_SDL {
    NORTH_SDL,
    EAST_SDL,
    SOUTH_SDL,
    WEST_SDL
} Orientation_SDL;

typedef enum LaserInteraction {
    REFLECT,
    ABSORB,
    HIT
} LaserInteraction;

typedef struct PieceSide {
    SDL_FPoint p1, p2;
    LaserInteraction interaction;
} PieceSide;

typedef struct Piece_SDL {
    PieceType_SDL piece_type;
    PlayerColor_SDL color;
    Orientation_SDL orientation;
    PieceSide sides[4];
    SDL_FPoint cp;
    int num_sides;
} Piece_SDL;

typedef struct Square_SDL {
    SDL_FPoint point;
    Position position;
    Piece_SDL *piece;
} Square_SDL;

typedef struct LaserSegment {
    SDL_FPoint p1, p2;
} LaserSegment;

typedef enum LaserNextStep {
    IDLE,
    CONTINUE,
    OFF_BOARD,
    REMOVE_PIECE,
    STOP_AT_PIECE
} LaserNextStep;

typedef struct Laser {
    LaserSegment segments[100];
    SDL_FPoint direction_vector;
    int num_segments;
    LaserNextStep next_step;
    float hold_timer;
} Laser;

typedef struct AppState {
    SDL_Window *win;
    SDL_Renderer *ren;
    SDL_Texture *play_again_button;
    SDL_FRect play_again_rect;
    Square_SDL board[ROWS][COLS];
    Position clicked_pos;
    Position selected_pos;
    Laser laser;
    Uint64 last_tick;
    int valid_squares[ROWS][COLS];
    float touch_start_x;
    float touch_start_y;
    float delta_time;
    bool clicked;
    bool selected;
    bool call_ai;
    bool call_fire_laser_ai;
    bool thinking;
    bool drawing_laser;
    bool real_laser;
    bool game_over;
    bool swipe_gesture;
} AppState;

#endif