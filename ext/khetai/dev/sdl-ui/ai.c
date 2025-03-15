#include "ai.h"
#include <stdio.h>
#include <stdlib.h>

Move call_ai_move(Square_SDL board[8][10]) {
    char *init_board[120];
    // top and bottom row padding
    for (unsigned int i = 0; i < 12; i++) {
        init_board[i] = "--";
        init_board[108 + i] = "--";
    }

    // left and right column padding
    for (unsigned int i = 0; i < 8; i++) {
        init_board[12 * (i + 1)] = "--";
        init_board[12 * (i + 2) - 1] = "--";
    }

    for (unsigned int i = 0; i < ROWS; i++) {
        for (unsigned int j = 0; j < COLS; j++) {
            Piece_SDL *piece = board[i][j].piece;
            char piece_str[3];

            if (piece != NULL) {
                switch (piece->piece_type) {
                    case PYRAMID_SDL: piece_str[0] = piece->color == RED_SDL ? 'P' : 'p'; break;
                    case SCARAB_SDL: piece_str[0] = piece->color == RED_SDL ? 'S' : 's'; break;
                    case ANUBIS_SDL: piece_str[0] = piece->color == RED_SDL ? 'A' : 'a'; break;
                    case PHARAOH_SDL: piece_str[0] = piece->color == RED_SDL ? 'X' : 'x'; break;
                    case LASER_SDL: piece_str[0] = piece->color == RED_SDL ? 'L' : 'l'; break;
                    default: break;
                }
                switch (piece->orientation) {
                    case NORTH_SDL: piece_str[1] = '0'; break;
                    case EAST_SDL: piece_str[1] = '1'; break;
                    case SOUTH_SDL: piece_str[1] = '2'; break;
                    case WEST_SDL: piece_str[1] = '3'; break;
                    default: break;
                }
                piece_str[2] = '\0';
            } else {
                strcpy(piece_str, "--");
            }

            int index = ((i + 1) * 12) + (j + 1);
            init_board[index] = malloc(3);

            strcpy(init_board[index], piece_str);
        }
    }

    reset_undo();
    init_zobrist();
    srand((unsigned)time(NULL));
    setup_board(init_board);

    printf("\nstart:\n");
    print_board();

    time_t start_time = time(NULL);
    set_time_parameters(5, start_time);

    int depth = 1;
    Move best_move = (Move)0;
    Move current_move = (Move)0;
    // while ((time(NULL) - start_time < max_time) && (depth <= 25)) {
    //     best_move = current_move;
    //     current_move = alphabeta_root(depth, RED);
    //     depth++;
    // }
    while (depth <= 25) {
        printf("\nDEPTH: %-3d->   ", depth);
        current_move = alphabeta_root(depth, RED);
        printf("MOVE -> START: %d, END: %d, ROTATION: %d\n", get_start(current_move), get_end(current_move), get_rotation(current_move));
        if ((time(NULL) - start_time < max_time))
            best_move = current_move;
        else
            break;
        depth++;
    }
    make_move(best_move);

    printf("\nend:\n");
    print_board();

    for (int i = 0; i < 120; i++) {
        if (init_board[i] != NULL && init_board[i] != "--") {
            free(init_board[i]);
        }
    }

    return best_move;
}

void get_row_col(int index, int *row, int *col) {
    *row = (index / 12) - 1;
    *col = (index % 12) - 1;
}
