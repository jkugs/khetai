#define SDL_MAIN_USE_CALLBACKS 1
#include "khet-sdl.h"
#include "ai.h"
#include "drawing.h"
#include <SDL3/SDL_main.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
static void start_ai_calculation(void *app_state_ptr);
#endif

static void init_board(AppState *as);
static void call_ai(AppState *as);
static void reset_selection(AppState *as);
static void process_click(AppState *as);
static void fire_laser(AppState *as, PlayerColor_SDL color);
static void calc_next_laser_step(AppState *as);
static void reset_delta_time(AppState *as);
static void reset_laser(AppState *as);

static void calc_piece_sides(Square_SDL *square);
static void rotate_piece_side(PieceSide *ps, SDL_FPoint cp, float angle);
static bool check_laser_piece_intersection(Piece_SDL *p, LaserSegment *ls, SDL_FPoint r, SDL_FPoint *intersection, PieceSide *crossed_side);

static void move_piece(Square_SDL board[ROWS][COLS], Position p1, Position p2);
static void rotate_piece(Square_SDL board[ROWS][COLS], Position pos, bool clockwise);
static void apply_move(Square_SDL board[ROWS][COLS], Move best_move);
static void remove_piece(Square_SDL board[ROWS][COLS], Position pos);
static Square_SDL *get_square_from_point(Square_SDL board[ROWS][COLS], SDL_FPoint p);

static float cross(SDL_FPoint a, SDL_FPoint b);
static SDL_FPoint normalize(SDL_FPoint v);

static const char *const init_pieces[8][10] = {
    {"L2", "--", "--", "--", "A2", "X2", "A2", "P1", "--", "--"},
    {"--", "--", "P2", "--", "--", "--", "--", "--", "--", "--"},
    {"--", "--", "--", "p3", "--", "--", "--", "--", "--", "--"},
    {"P0", "--", "p2", "--", "S2", "S3", "--", "P1", "--", "p3"},
    {"P1", "--", "p3", "--", "s1", "s0", "--", "P0", "--", "p2"},
    {"--", "--", "--", "--", "--", "--", "P1", "--", "--", "--"},
    {"--", "--", "--", "--", "--", "--", "--", "p0", "--", "--"},
    {"--", "--", "p3", "a0", "x0", "a0", "--", "--", "--", "l0"}};

void init_board(AppState *as) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            Square_SDL *s = &as->board[i][j];
            s->position.row = i;
            s->position.col = j;
            s->point.x = (j * SQUARE_SIZE) + (WINDOW_BUFFER) + 0.5f;
            s->point.y = (i * SQUARE_SIZE) + (WINDOW_BUFFER) + 0.5f;

            const char *piece = init_pieces[i][j];
            if (strcmp(piece, "--") == 0) {
                s->piece = NULL;
                continue;
            }

            s->piece = (Piece_SDL *)malloc(sizeof(Piece_SDL));

            Piece_SDL *p = s->piece;
            p->color = (isupper(piece[0])) ? RED_SDL : SILVER_SDL;

            // clang-format off
            switch (toupper(piece[0])) {
            case 'P': p->piece_type = PYRAMID_SDL; p->num_sides = 3; break;
            case 'S': p->piece_type = SCARAB_SDL; p->num_sides = 1; break;
            case 'A': p->piece_type = ANUBIS_SDL; p->num_sides = 4; break;
            case 'X': p->piece_type = PHARAOH_SDL; p->num_sides = 4; break;
            case 'L': p->piece_type = SPHINX_SDL; p->num_sides = 3; break;
            } // clang-format on

            switch (piece[1]) {
            case '0': p->orientation = NORTH_SDL; break;
            case '1': p->orientation = EAST_SDL; break;
            case '2': p->orientation = SOUTH_SDL; break;
            case '3': p->orientation = WEST_SDL; break;
            }

            calc_piece_sides(s);
        }
    }
}

void calc_piece_sides(Square_SDL *square) {
    if (square->piece == NULL) {
        return;
    }

    Piece_SDL *p = square->piece;
    p->cp = (SDL_FPoint){(square->point.x + SQUARE_SIZE * 0.5f), (square->point.y + SQUARE_SIZE * 0.5f)};
    SDL_FPoint cp = p->cp;

    // Assume all pieces are facing NORTH for creation.
    // Always start at the top left and go clockwise.
    float half = PIECE_SIZE * 0.5f;
    switch (p->piece_type) {
    case PYRAMID_SDL: {
        SDL_FPoint p1 = {cp.x - half, cp.y - half};
        SDL_FPoint p2 = {cp.x + half, cp.y + half};
        SDL_FPoint p3 = {cp.x - half, cp.y + half};
        p->sides[0] = (PieceSide){p1, p2, REFLECT};
        p->sides[1] = (PieceSide){p2, p3, HIT};
        p->sides[2] = (PieceSide){p3, p1, HIT};
        break;
    }
    case SCARAB_SDL: {
        SDL_FPoint p1 = {cp.x - half, cp.y - half};
        SDL_FPoint p2 = {cp.x + half, cp.y + half};
        p->sides[0] = (PieceSide){p1, p2, REFLECT};
        break;
    }
    case ANUBIS_SDL: {
        SDL_FPoint p1 = {cp.x - half, cp.y - half};
        SDL_FPoint p2 = {cp.x + half, cp.y - half};
        SDL_FPoint p3 = {cp.x + half, cp.y + half};
        SDL_FPoint p4 = {cp.x - half, cp.y + half};
        p->sides[0] = (PieceSide){p1, p2, ABSORB};
        p->sides[1] = (PieceSide){p2, p3, HIT};
        p->sides[2] = (PieceSide){p3, p4, HIT};
        p->sides[3] = (PieceSide){p4, p1, HIT};
        break;
    }
    case PHARAOH_SDL: {
        SDL_FPoint p1 = {cp.x - half, cp.y - half};
        SDL_FPoint p2 = {cp.x + half, cp.y - half};
        SDL_FPoint p3 = {cp.x + half, cp.y + half};
        SDL_FPoint p4 = {cp.x - half, cp.y + half};
        p->sides[0] = (PieceSide){p1, p2, HIT};
        p->sides[1] = (PieceSide){p2, p3, HIT};
        p->sides[2] = (PieceSide){p3, p4, HIT};
        p->sides[3] = (PieceSide){p4, p1, HIT};
        break;
    }
    case SPHINX_SDL: {
        SDL_FPoint p1 = {cp.x, cp.y - half};
        SDL_FPoint p2 = {cp.x + half, cp.y + half};
        SDL_FPoint p3 = {cp.x - half, cp.y + half};
        p->sides[0] = (PieceSide){p1, p2, ABSORB};
        p->sides[1] = (PieceSide){p2, p3, ABSORB};
        p->sides[2] = (PieceSide){p3, p1, ABSORB};
        break;
    }
    }

    for (int i = 0; i < p->num_sides; i++) {
        float angle = 0;
        switch (p->orientation) {
        case NORTH_SDL:
            break;
        case EAST_SDL:
            angle = M_PI / 2.0f;
            rotate_piece_side(&p->sides[i], cp, angle);
            break;
        case SOUTH_SDL:
            angle = M_PI;
            rotate_piece_side(&p->sides[i], cp, angle);
            break;
        case WEST_SDL:
            angle = 3.0f * M_PI / 2.0f;
            rotate_piece_side(&p->sides[i], cp, angle);
            break;
        }
    }
}

void rotate_piece_side(PieceSide *ps, SDL_FPoint cp, float angle) {
    SDL_FPoint *p1 = &ps->p1;
    SDL_FPoint *p2 = &ps->p2;

    float cos_a = cos(angle);
    float sin_a = sin(angle);

    // Rotate each segment around the (0, 0) origin
    // and then move them back to their true location.

    float x1 = p1->x - cp.x;
    float y1 = p1->y - cp.y;
    p1->x = cp.x + (x1 * cos_a - y1 * sin_a);
    p1->y = cp.y + (x1 * sin_a + y1 * cos_a);

    float x2 = p2->x - cp.x;
    float y2 = p2->y - cp.y;
    p2->x = cp.x + (x2 * cos_a - y2 * sin_a);
    p2->y = cp.y + (x2 * sin_a + y2 * cos_a);
}

// TODO: Move piece over time function

void move_piece(Square_SDL board[ROWS][COLS], Position p1, Position p2) {
    if (board[p2.row][p2.col].piece == NULL) {
        board[p2.row][p2.col].piece = board[p1.row][p1.col].piece;
        board[p1.row][p1.col].piece = NULL;
    } else {
        Piece_SDL *temp;
        temp = board[p2.row][p2.col].piece;
        board[p2.row][p2.col].piece = board[p1.row][p1.col].piece;
        board[p1.row][p1.col].piece = temp;
    }

    // TODO: use move piece sides function. will use cp.
    // will calc cp location over time.
    calc_piece_sides(&board[p1.row][p1.col]);
    calc_piece_sides(&board[p2.row][p2.col]);
}

void remove_piece(Square_SDL board[ROWS][COLS], Position pos) {
    board[pos.row][pos.col].piece = NULL;
}

Square_SDL *get_square_from_point(Square_SDL board[ROWS][COLS], SDL_FPoint p) {
    int col = (p.x - WINDOW_BUFFER) / SQUARE_SIZE;
    int row = (p.y - WINDOW_BUFFER) / SQUARE_SIZE;
    return &board[row][col];
}

void rotate_piece(Square_SDL board[ROWS][COLS], Position pos, bool clockwise) {
    Piece_SDL *p = board[pos.row][pos.col].piece;
    switch (p->orientation) {
    case NORTH_SDL: p->orientation = clockwise ? EAST_SDL : WEST_SDL; break;
    case EAST_SDL: p->orientation = clockwise ? SOUTH_SDL : NORTH_SDL; break;
    case SOUTH_SDL: p->orientation = clockwise ? WEST_SDL : EAST_SDL; break;
    case WEST_SDL: p->orientation = clockwise ? NORTH_SDL : SOUTH_SDL; break;
    default: break;
    }

    // TODO: calc angle over time.
    float angle = clockwise ? (M_PI / 2.0f) : -(M_PI / 2.0f);
    for (int i = 0; i < p->num_sides; i++) {
        rotate_piece_side(&p->sides[i], p->cp, angle);
    }
}

void apply_move(Square_SDL board[ROWS][COLS], Move best_move) {
    int start = get_start(best_move);
    int end = get_end(best_move);
    int rotation = get_rotation(best_move);

    int start_row, start_col, end_row, end_col;
    get_row_col(start, &start_row, &start_col);
    get_row_col(end, &end_row, &end_col);

    Position p1 = {start_row, start_col};
    Position p2 = {end_row, end_col};

    if (rotation != 0) {
        bool clockwise = rotation == 1;
        rotate_piece(board, p1, clockwise);
    } else {
        move_piece(board, p1, p2);
    }

    // drain event queue
    // TODO: address in wasm...
    SDL_Event event;
    while (SDL_PollEvent(&event)) {}
}

// Initializes the Laser along with its first segment
void fire_laser(AppState *as, PlayerColor_SDL player) {
    reset_laser(as);
    as->drawing_laser = true;
    Laser *laser = &as->laser;
    laser->hold_timer = 0.0f;
    float distance = LASER_SPEED * as->delta_time;

    int row, col;
    if (player == RED_SDL) {
        row = 0;
        col = 0;
    } else {
        row = 7;
        col = 9;
    }

    Square_SDL square = as->board[row][col];
    float x1, y1, x2, y2;
    SDL_FPoint p1, p2;
    x1 = square.point.x + (SQUARE_SIZE * 0.5);
    y1 = square.point.y + (SQUARE_SIZE * 0.5);

    SDL_FPoint dir;
    switch (square.piece->orientation) {
    case NORTH_SDL:
        y1 -= (PIECE_SIZE * 0.5);
        dir = (SDL_FPoint){0, -1};
        break;
    case EAST_SDL:
        x1 += (PIECE_SIZE * 0.5);
        dir = (SDL_FPoint){1, 0};
        break;
    case SOUTH_SDL:
        y1 += (PIECE_SIZE * 0.5);
        dir = (SDL_FPoint){0, 1};
        break;
    case WEST_SDL:
        x1 -= (PIECE_SIZE * 0.5);
        dir = (SDL_FPoint){-1, 0};
        break;
    }
    p1 = (SDL_FPoint){x1, y1};
    p2 = (SDL_FPoint){(p1.x + dir.x * distance), (p1.y + dir.y * distance)};

    laser->direction_vector = dir;
    laser->segments[0].p1 = p1;
    laser->segments[0].p2 = p2;
    laser->num_segments++;
    laser->next_step = CONTINUE;
}

void reset_laser(AppState *as) {
    reset_delta_time(as);
    as->laser.num_segments = 0;
    as->laser.next_step = IDLE;
    as->laser.hold_timer = 0.0f;
    as->drawing_laser = false;
}

void calc_next_laser_step(AppState *as) {
    Laser *laser = &as->laser;

    if (laser->next_step == OFF_BOARD || laser->next_step == STOP_AT_PIECE) {
        laser->hold_timer += as->delta_time;
        if (laser->hold_timer > 0.7f) {
            as->real_laser = false;
            reset_laser(as);
        }
        return;
    } else if (laser->next_step == REMOVE_PIECE) {
        laser->hold_timer += as->delta_time;
        if (laser->hold_timer > 0.7f) {
            Square_SDL *square = get_square_from_point(as->board, laser->segments[laser->num_segments - 1].p2);
            remove_piece(as->board, square->position);
            as->real_laser = false;
            reset_laser(as);
        }
        return;
    }

    float distance = LASER_SPEED * as->delta_time;
    LaserSegment *segment = &laser->segments[laser->num_segments - 1];

    segment->p2.x += laser->direction_vector.x * distance;
    segment->p2.y += laser->direction_vector.y * distance;

    SDL_FPoint p1 = segment->p1;
    SDL_FPoint p2 = segment->p2;

    // check if we are off the board
    if (p2.x > WINDOW_WIDTH | p2.x < 0 || p2.y > WINDOW_HEIGHT || p2.y < 0) {
        laser->next_step = OFF_BOARD;
        laser->hold_timer = 0.0f;
        return;
    }

    // check if we've crossed a piece side
    // calculate r -> laser vector from origin
    SDL_FPoint r = (SDL_FPoint){(p2.x - p1.x), (p2.y - p1.y)};

    SDL_FPoint intersection;
    PieceSide crossed_side;

    bool crossed = false;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (as->board[i][j].piece == NULL)
                continue;

            if (check_laser_piece_intersection(as->board[i][j].piece, segment, r, &intersection, &crossed_side)) {
                crossed = true;
                break;
            }
        }
    }

    // a piece side was crossed. clamp laser to intersection point.
    // decide what to do next...
    if (crossed) {
        segment->p2 = intersection;
        switch (crossed_side.interaction) {
        case REFLECT: {
            // get side vector and normal
            SDL_FPoint s1 = crossed_side.p1;
            SDL_FPoint s2 = crossed_side.p2;
            SDL_FPoint s = (SDL_FPoint){s2.x - s1.x, s2.y - s1.y};
            SDL_FPoint n = normalize((SDL_FPoint){-s.y, s.x}); // 90 degrees ccw normalized

            // normalize incoming laser direction
            SDL_FPoint r_norm = normalize(r);

            // reflect the laser
            float dot = r_norm.x * n.x + r_norm.y * n.y;
            SDL_FPoint reflected = (SDL_FPoint){(r_norm.x - 2 * dot * n.x), (r_norm.y - 2 * dot * n.y)};
            SDL_FPoint reflection_vector = normalize(reflected);

            // start new laser segment in the reflected direction
            LaserSegment *new_segment = &laser->segments[laser->num_segments];
            new_segment->p1 = intersection;
            new_segment->p2 = (SDL_FPoint){(intersection.x + reflection_vector.x * 0.01f), (intersection.y + reflection_vector.y * 0.01f)};
            laser->num_segments++;
            laser->direction_vector = reflection_vector;
            laser->next_step = CONTINUE;
            break;
        }
        case HIT:
            if (as->real_laser) {
                laser->next_step = REMOVE_PIECE;
            } else {
                laser->next_step = STOP_AT_PIECE;
            }
            break;
        case ABSORB:
            laser->next_step = STOP_AT_PIECE;
            break;
        }
    }
}

SDL_FPoint normalize(SDL_FPoint v) {
    float length = sqrtf(v.x * v.x + v.y * v.y);
    if (length == 0.0f)
        return (SDL_FPoint){0.0f, 0.0f}; // avoid divide by zero
    return (SDL_FPoint){v.x / length, v.y / length};
}

float cross(SDL_FPoint a, SDL_FPoint b) {
    return a.x * b.y - a.y * b.x;
}

bool check_laser_piece_intersection(Piece_SDL *piece, LaserSegment *ls, SDL_FPoint r, SDL_FPoint *intersection, PieceSide *crossed_side) {
    for (int i = 0; i < piece->num_sides; i++) {
        SDL_FPoint q1 = piece->sides[i].p1;
        SDL_FPoint q2 = piece->sides[i].p2;
        SDL_FPoint s = (SDL_FPoint){(q2.x - q1.x), (q2.y - q1.y)};

        float r_cross_s = cross(r, s);
        if (r_cross_s == 0.0f)
            continue;

        // r = laser vector
        // s = side vector
        SDL_FPoint p = ls->p1; // laser start point
        SDL_FPoint q = q1;     // side start point

        SDL_FPoint qp = {(q.x - p.x), (q.y - p.y)};
        float t = cross(qp, s) / r_cross_s;
        float u = cross(qp, r) / r_cross_s;

        // check for valid intersection
        if (t >= 1e-4f && t <= 1 && u >= 0 && u <= 1) {
            intersection->x = p.x + (t * r.x);
            intersection->y = p.y + (t * r.y);
            *crossed_side = piece->sides[i];
            return true;
        }
    }

    return false;
}

#ifdef __EMSCRIPTEN__
void start_ai_calculation(void *app_state_ptr) {
    AppState *as = (AppState *)app_state_ptr;
    Move best_move = call_ai_move(as->board);
    apply_move(as->board, best_move);
    as->call_fire_laser_ai = true;

    // re-calculate delta_time and last_time because AI takes a while
    reset_delta_time(as);
}
#endif

void call_ai(AppState *as) {
#ifdef __EMSCRIPTEN__
    // Due to how wasm handles drawing, we need to delay
    // the call to the ai to allow the board to update on screen
    emscripten_async_call(start_ai_calculation, as, 32);
#else
    Move best_move = call_ai_move(as->board);
    apply_move(as->board, best_move);
    as->call_fire_laser_ai = true;

    // re-calculate delta_time and last_time because AI takes a while
    reset_delta_time(as);
#endif
}

void reset_selection(AppState *as) {
    as->clicked_pos.row = -1;
    as->clicked_pos.col = -1;
    as->selected_pos.row = -1;
    as->selected_pos.col = -1;
    as->selected = false;

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            as->valid_squares[i][j] = 0;
        }
    }
}

void process_click(AppState *as) {
    int row = as->clicked_pos.row;
    int col = as->clicked_pos.col;

    if (as->selected) {
        if (as->valid_squares[row][col] == 1) {
            move_piece(as->board, as->selected_pos, as->clicked_pos);
            reset_selection(as);
            as->real_laser = true;
            fire_laser(as, SILVER_SDL);
            as->call_ai = true;
        } else if (as->selected_pos.row == row && as->selected_pos.col == col) {
            reset_selection(as);
        }
    } else if (as->board[row][col].piece != NULL && as->board[row][col].piece->color == SILVER_SDL) {
        as->selected_pos.row = row;
        as->selected_pos.col = col;

        // Determine valid squares the selected piece can move to
        Piece_SDL *mp = as->board[row][col].piece;
        enum MovePermission perm = mp->color == RED_SDL ? R : S;
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (abs(i - row) > 1 || abs(j - col) > 1 || mp->piece_type == SPHINX_SDL && (as->selected_pos.row != row || as->selected_pos.col != col)) {
                    continue;
                }

                Piece_SDL *p = as->board[i][j].piece;
                bool can_move = (square_colors[i][j] == B || square_colors[i][j] == perm);

                if (p == NULL && mp->piece_type != SPHINX_SDL) {
                    as->valid_squares[i][j] = can_move;
                } else if (mp->piece_type == SCARAB_SDL) {
                    if (p->piece_type != SCARAB_SDL && p->piece_type != PHARAOH_SDL && p->piece_type != SPHINX_SDL) {
                        as->valid_squares[i][j] = can_move;
                    }
                }
            }
        }
        as->selected = true;
    }

    // click has been processed... reset it to false
    as->clicked = false;
}

void reset_delta_time(AppState *as) {
    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();
    as->delta_time = (float)(now - as->last_tick) / (float)freq;
    as->last_tick = now;
}

SDL_AppResult SDL_AppEvent(void *app_state_ptr, SDL_Event *event) {
    AppState *as = (AppState *)app_state_ptr;

    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (as->real_laser)
            break;

        int x = event->button.x;
        int y = event->button.y;
        bool in_bounds = x >= WINDOW_BUFFER && x < WINDOW_BUFFER + BOARD_WIDTH &&
                         y >= WINDOW_BUFFER && y < WINDOW_BUFFER + BOARD_HEIGHT;

        if (in_bounds) {
            as->clicked_pos.col = (x - WINDOW_BUFFER) / SQUARE_SIZE;
            as->clicked_pos.row = (y - WINDOW_BUFFER) / SQUARE_SIZE;
            as->clicked = true;
        }
        break;
    case SDL_EVENT_KEY_UP:
        if (as->real_laser)
            break;

        SDL_Keycode key = event->key.key;

        if (key == SDLK_RETURN) {
            call_ai(as);
        } else if (key == SDLK_LEFT || key == SDLK_RIGHT) {
            if (as->selected) {
                Piece_SDL *p = as->board[as->selected_pos.row][as->selected_pos.col].piece;
                bool rotate_right = (key == SDLK_RIGHT);
                bool can_rotate_sphinx = p->piece_type == SPHINX_SDL &&
                                         ((rotate_right && p->orientation == WEST_SDL) ||
                                          (!rotate_right && p->orientation == NORTH_SDL));

                if (p->piece_type != SPHINX_SDL || can_rotate_sphinx) {
                    rotate_piece(as->board, as->selected_pos, rotate_right);
                    reset_selection(as);
                    as->real_laser = true;
                    fire_laser(as, SILVER_SDL);
                    as->call_ai = true;
                }
            }
        } else if (key == SDLK_SPACE) {
            fire_laser(as, SILVER_SDL);
        }
        break;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *app_state_ptr) {
    AppState *as = (AppState *)app_state_ptr;
    reset_delta_time(as);

    if (as->clicked) {
        process_click(as);
    }

    if (as->drawing_laser) {
        calc_next_laser_step(as);
    }

    if (as->call_fire_laser_ai) {
        fire_laser(as, RED_SDL);
        as->call_fire_laser_ai = false;
        as->real_laser = true;
    }

    draw(as);

    if (as->call_ai && !as->drawing_laser) {
        call_ai(as);
        as->call_ai = false;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **app_state_ptr, int argc, char *argv[]) {
    AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!as) {
        return SDL_APP_FAILURE;
    }

    as->clicked_pos.row = -1;
    as->clicked_pos.col = -1;
    as->selected_pos.col = -1;
    as->selected_pos.col = -1;

    *app_state_ptr = as;
    reset_laser(as);

    if (!SDL_CreateWindowAndRenderer("Khet", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &as->win, &as->ren)) {
        SDL_Log("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

#ifndef __EMSCRIPTEN__
    if (SDL_SetRenderVSync(as->ren, SDL_RENDERER_VSYNC_ADAPTIVE) < 0) {
        SDL_Log("Warning: Adaptive VSync not supported: %s\n", SDL_GetError());
    }
#else
    SDL_Log("Info: Skipping VSync setup under Emscripten.\n");
#endif

    init_board(as);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *app_state_ptr, SDL_AppResult result) {
    if (app_state_ptr != NULL) {
        AppState *as = (AppState *)app_state_ptr;
        SDL_DestroyRenderer(as->ren);
        SDL_DestroyWindow(as->win);
        SDL_free(as);
    }
}
