#define SDL_MAIN_USE_CALLBACKS 1
#include "khet-sdl.h"
#include "drawing.h"
#include <SDL3/SDL_main.h>
#include <stdio.h>

void init_board(void *app_state_ptr) {
    AppState *as = (AppState *)app_state_ptr;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            Piece *p = &as->board[i][j];
            p->color = RED;
            p->direction = NORTH;
            p->piece_type = PYRAMID;
            p->position.row = i;
            p->position.col = j;
            p->point.x = (j * SQUARE_SIZE) + (WINDOW_BUFFER);
            p->point.y = (i * SQUARE_SIZE) + (WINDOW_BUFFER);
        }
    }
}

SDL_AppResult SDL_AppIterate(void *app_state_ptr) {
    AppState *as = (AppState *)app_state_ptr;
    SDL_SetRenderDrawColor(as->ren, 169, 169, 169, 255);
    SDL_RenderClear(as->ren);
    SDL_SetRenderDrawBlendMode(as->ren, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(as->ren, 0, 0, 0, 255);
    SDL_FRect square = {WINDOW_BUFFER, WINDOW_BUFFER, BOARD_WIDTH, BOARD_HEIGHT};
    SDL_RenderRect(as->ren, &square);

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
    if ((as->clicked_pos.row >= 0 && as->clicked_pos.col >= 0)) {
        SDL_SetRenderDrawColor(as->ren, 255, 255, 0, 100);
        SDL_FRect highlight_square = {
            (WINDOW_BUFFER + 1) + as->clicked_pos.col * SQUARE_SIZE,
            (WINDOW_BUFFER + 1) + as->clicked_pos.row * SQUARE_SIZE,
            SQUARE_SIZE - 2, SQUARE_SIZE - 2};
        SDL_RenderFillRect(as->ren, &highlight_square);
        as->cur_clicked_pos.row = as->clicked_pos.row;
        as->cur_clicked_pos.col = as->clicked_pos.col;
    }

    // Reset draw color to black for lines
    SDL_SetRenderDrawColor(as->ren, 0, 0, 0, 255);

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
            draw_pyramid(as, i, j);
            // draw_pharaoh(as, i, j);
            // draw_anubis(as, i, j);
            // draw_scarab(as, i, j);
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
