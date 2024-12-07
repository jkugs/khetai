#include "khetai_lib.h"
#include <ruby.h>
#include <stdlib.h>
#include <time.h>

static int convert_index(int i) {
    int row = i / 12;
    int col = i % 12;

    int new_row = row - 1;
    int new_col = col - 1;

    return (new_row * 10) + new_col;
}

VALUE move(VALUE self, VALUE board_array, VALUE _player, VALUE _max_depth, VALUE _max_time) {
    // verify parameters
    int player = NUM2INT(_player);
    if (player < 0 || player > 1) {
        rb_raise(rb_eArgError, "player must be 0 (silver) or 1 (red)");
    }

    int max_time = NUM2INT(_max_time);
    if (max_time < 1) {
        rb_raise(rb_eArgError, "max_time (seconds) must be 1 or greater");
    }

    int max_depth = NUM2INT(_max_depth);
    if (max_depth < 2 || max_depth > 25) {
        rb_raise(rb_eArgError, "max_depth (ply) must be between 2 and 25");
    }

    srand((unsigned)time(NULL));

    unsigned int array_size = (unsigned int)RARRAY_LEN(board_array);

    // verify board_array is valid
    if (array_size != 80) {
        rb_raise(rb_eArgError, "board_array must have exactly 80 elements");
    }
    const char valid_pieces[] = {'L', 'A', 'X', 'P', 'S', 'p', 'a', 'x', 's', 'l'};
    size_t valid_pieces_count = 10;
    for (unsigned int i = 0; i < array_size; i++) {
        VALUE element = rb_ary_entry(board_array, i);
        if (!RB_TYPE_P(element, T_STRING) || RSTRING_LEN(element) != 2) {
            rb_raise(rb_eArgError, "each element in board_array must be 2 characters");
        }

        // check if element in board_array is a valid string
        const char *element_str = RSTRING_PTR(element);
        int is_valid = 0;
        for (unsigned int j = 0; j < valid_pieces_count; j++) {
            if ((element_str[0] == valid_pieces[j] && element_str[1] >= '0' && element_str[1] <= '3') || strcmp(element_str, "--") == 0) {
                is_valid = 1;
                break;
            }
        }
        if (!is_valid) {
            rb_raise(rb_eArgError, "each element in board_array must be a valid piece (example: 'p1') or empty ('--')");
        }
    }

    // khetai_lib assumes an extra level of padding around the board
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

    // fill in the rest of the board passed from ruby
    for (unsigned int i = 0; i < array_size; i++) {
        VALUE square = rb_ary_entry(board_array, i);
        init_board[13 + ((i % 10) + ((i / 10) * 12))] = StringValueCStr(square);
    }

    reset_undo();
    init_zobrist();
    setup_board(init_board);

    time_t start_time = time(NULL);
    set_time_parameters(max_time, start_time);

    // iterative deepening
    int depth = 1;
    Move best_move = (Move)0;
    Move current_move = (Move)0;
    while ((time(NULL) - start_time < max_time) && (depth <= max_depth)) {
        best_move = current_move;
        current_move = alphabeta_root(depth, player);
        depth++;
    }
    make_move(best_move);

    VALUE out = rb_ary_new2(3);
    rb_ary_store(out, 0, INT2NUM(convert_index(get_start(best_move))));
    rb_ary_store(out, 1, INT2NUM(convert_index(get_end(best_move))));
    rb_ary_store(out, 2, INT2NUM(get_rotation(best_move)));

    return out;
}

void Init_khetai() {
    VALUE KhetAI = rb_define_module("KhetAI");
    rb_define_singleton_method(KhetAI, "move", move, 4);
}
