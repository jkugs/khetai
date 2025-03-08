#define SDL_MAIN_USE_CALLBACKS 1
#include "khet-sdl.h"
#include "drawing.h"
#include <SDL3/SDL_main.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

char *init_pieces[8][10] = {
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
            Square *s = &as->board[i][j];
            s->position.row = i;
            s->position.col = j;
            s->point.x = (j * SQUARE_SIZE) + (WINDOW_BUFFER) + 0.5;
            s->point.y = (i * SQUARE_SIZE) + (WINDOW_BUFFER) + 0.5;

            char *piece = init_pieces[i][j];
            if (strcmp(piece, "--") == 0) {
                s->piece = NULL;
                continue;
            }

            s->piece = (Piece *)malloc(sizeof(Piece));

            Piece *p = s->piece;
            p->color = (isupper(piece[0])) ? RED : SILVER;

            switch (toupper(piece[0])) {
                case 'P': p->piece_type = PYRAMID; break;
                case 'S': p->piece_type = SCARAB; break;
                case 'A': p->piece_type = ANUBIS; break;
                case 'X': p->piece_type = PHARAOH; break;
                case 'L': p->piece_type = LASER; break;
                default: break;
            }

            switch (piece[1]) {
                case '0': p->direction = NORTH; break;
                case '1': p->direction = EAST; break;
                case '2': p->direction = SOUTH; break;
                case '3': p->direction = WEST; break;
                default: break;
            }
        }
    }
}

SDL_AppResult SDL_AppIterate(void *app_state_ptr) {
    AppState *as = (AppState *)app_state_ptr;
    SDL_SetRenderDrawColor(as->ren, 169, 169, 169, 255);
    SDL_RenderClear(as->ren);
    SDL_SetRenderDrawBlendMode(as->ren, SDL_BLENDMODE_BLEND);

    // New click registered
    if (as->clicked) {
        if (as->cur_clicked_pos.row != as->clicked_pos.row || as->cur_clicked_pos.col != as->clicked_pos.col) {
            as->cur_clicked_pos.row = as->clicked_pos.row;
            as->cur_clicked_pos.col = as->clicked_pos.col;
        } else {
            as->clicked_pos.row = -1;
            as->clicked_pos.col = -1;
            as->cur_clicked_pos.row = -1;
            as->cur_clicked_pos.col = -1;
        }
        as->clicked = false;
    }

    // Highlight clicked square
    if (as->clicked_pos.row >= 0 && as->clicked_pos.col >= 0) {
        // Yellow - highlight
        SDL_SetRenderDrawColor(as->ren, 255, 255, 0, 100);
        SDL_FRect highlight_square = {
            (WINDOW_BUFFER) + as->clicked_pos.col * SQUARE_SIZE,
            (WINDOW_BUFFER) + as->clicked_pos.row * SQUARE_SIZE,
            SQUARE_SIZE, SQUARE_SIZE};
        SDL_RenderFillRect(as->ren, &highlight_square);
        as->cur_clicked_pos.row = as->clicked_pos.row;
        as->cur_clicked_pos.col = as->clicked_pos.col;
    }

    // Black
    SDL_SetRenderDrawColor(as->ren, 0, 0, 0, 255);
    SDL_FRect square = {WINDOW_BUFFER, WINDOW_BUFFER, BOARD_WIDTH, BOARD_HEIGHT};
    SDL_RenderRect(as->ren, &square);

    // Draw vertical lines
    for (int i = 0; i <= 8; i++) {
        int x = (i * SQUARE_SIZE) + (WINDOW_BUFFER + SQUARE_SIZE);
        int y = (WINDOW_BUFFER);
        SDL_RenderLine(as->ren, x, y, x, (BOARD_HEIGHT + y) - 1);
    }
    // Draw horizontal lines
    for (int i = 0; i <= 6; i++) {
        int x = (WINDOW_BUFFER);
        int y = (i * SQUARE_SIZE) + (WINDOW_BUFFER + SQUARE_SIZE);
        SDL_RenderLine(as->ren, x, y, (BOARD_WIDTH + x) - 1, y);
    }

    // Draw pieces
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (as->board[i][j].piece != NULL) {
                draw_piece(as, i, j);
            }
        }
    }

    SDL_RenderPresent(as->ren);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **app_state_ptr, int argc, char *argv[]) {
    AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!as) {
        return SDL_APP_FAILURE;
    }

    as->clicked_pos.row = -1;
    as->clicked_pos.col = -1;
    as->cur_clicked_pos.col = -1;
    as->cur_clicked_pos.col = -1;

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
