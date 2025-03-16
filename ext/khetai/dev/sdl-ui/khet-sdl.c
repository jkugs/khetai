#define SDL_MAIN_USE_CALLBACKS 1
#include "khet-sdl.h"
#include "ai.h"
#include "drawing.h"
#include <SDL3/SDL_main.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static const char *const init_pieces[8][10] = {
    {"L2", "--", "--", "--", "A2", "X2", "A2", "P1", "--", "--"},
    {"--", "--", "P2", "--", "--", "--", "--", "--", "--", "--"},
    {"--", "--", "--", "p3", "--", "--", "--", "--", "--", "--"},
    {"P0", "--", "p2", "--", "S2", "S3", "--", "P1", "--", "p3"},
    {"P1", "--", "p3", "--", "s1", "s0", "--", "P0", "--", "p2"},
    {"--", "--", "--", "--", "--", "--", "P1", "--", "--", "--"},
    {"--", "--", "--", "--", "--", "--", "--", "p0", "--", "--"},
    {"--", "--", "p3", "a0", "x0", "a0", "--", "--", "--", "l0"}};

void init_board(void *app_state_ptr) {
    AppState *as = (AppState *)app_state_ptr;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            Square_SDL *s = &as->board[i][j];
            s->position.row = i;
            s->position.col = j;
            s->point.x = (j * SQUARE_SIZE) + (WINDOW_BUFFER) + 0.5;
            s->point.y = (i * SQUARE_SIZE) + (WINDOW_BUFFER) + 0.5;

            const char *piece = init_pieces[i][j];
            if (strcmp(piece, "--") == 0) {
                s->piece = NULL;
                continue;
            }

            s->piece = (Piece_SDL *)malloc(sizeof(Piece_SDL));

            Piece_SDL *p = s->piece;
            p->color = (isupper(piece[0])) ? RED_SDL : SILVER_SDL;

            switch (toupper(piece[0])) {
            case 'P': p->piece_type = PYRAMID_SDL; break;
            case 'S': p->piece_type = SCARAB_SDL; break;
            case 'A': p->piece_type = ANUBIS_SDL; break;
            case 'X': p->piece_type = PHARAOH_SDL; break;
            case 'L': p->piece_type = LASER_SDL; break;
            default: break;
            }

            switch (piece[1]) {
            case '0': p->orientation = NORTH_SDL; break;
            case '1': p->orientation = EAST_SDL; break;
            case '2': p->orientation = SOUTH_SDL; break;
            case '3': p->orientation = WEST_SDL; break;
            default: break;
            }
        }
    }
}

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
}

void call_ai(void *app_state_ptr) {
    AppState *as = (AppState *)app_state_ptr;

    Move best_move = call_ai_move(as->board);
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
        rotate_piece(as->board, p1, clockwise);
    } else {
        move_piece(as->board, p1, p2);
    }
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

void process_click(void *app_state_ptr) {
    AppState *as = (AppState *)app_state_ptr;

    int row = as->clicked_pos.row;
    int col = as->clicked_pos.col;

    if (as->selected) {
        if (as->valid_squares[row][col] == 1) {
            move_piece(as->board, as->selected_pos, as->clicked_pos);
            reset_selection(as);
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
                if (abs(i - row) > 1 || abs(j - col) > 1 || mp->piece_type == LASER_SDL && (as->selected_pos.row != row || as->selected_pos.col != col)) {
                    continue;
                }

                Piece_SDL *p = as->board[i][j].piece;
                bool can_move = (square_colors[i][j] == B || square_colors[i][j] == perm);

                if (p == NULL) {
                    as->valid_squares[i][j] = can_move;
                } else if (mp->piece_type == SCARAB_SDL) {
                    if (p->piece_type != SCARAB_SDL && p->piece_type != PHARAOH_SDL && p->piece_type != LASER_SDL) {
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

SDL_AppResult SDL_AppIterate(void *app_state_ptr) {
    AppState *as = (AppState *)app_state_ptr;

    if (as->clicked) {
        process_click(as);
    }

    draw(as);

    if (as->call_ai) {
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

    if (!SDL_CreateWindowAndRenderer("Khet", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &as->win, &as->ren)) {
        printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    init_board(as);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *app_state_ptr, SDL_Event *event) {
    AppState *as = (AppState *)app_state_ptr;
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    } else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        int mouse_x = event->button.x;
        int mouse_y = event->button.y;

        if (mouse_x >= WINDOW_BUFFER && mouse_x < WINDOW_BUFFER + BOARD_WIDTH &&
            mouse_y >= WINDOW_BUFFER && mouse_y < WINDOW_BUFFER + BOARD_HEIGHT) {
            // Calculate which square was clicked
            as->clicked_pos.col = (mouse_x - WINDOW_BUFFER) / SQUARE_SIZE;
            as->clicked_pos.row = (mouse_y - WINDOW_BUFFER) / SQUARE_SIZE;
            as->clicked = true;
        }
    } else if (event->type == SDL_EVENT_KEY_UP) {
        // TODO: a move will be triggered by a valid move eventually...
        if (event->key.key == SDLK_RETURN) {
            call_ai(as);
        } else if (event->key.key == SDLK_LEFT || event->key.key == SDLK_RIGHT) {
            if (as->selected) {
                Piece_SDL *p = as->board[as->selected_pos.row][as->selected_pos.col].piece;
                bool rotate_right = (event->key.key == SDLK_RIGHT);
                if (p->piece_type != LASER_SDL || (rotate_right && p->orientation == WEST_SDL) || (!rotate_right && p->orientation == NORTH_SDL)) {
                    rotate_piece(as->board, as->selected_pos, rotate_right);
                    reset_selection(as);
                    as->call_ai = true;
                }
            }
        }
    }
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
