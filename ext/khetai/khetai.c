#include <ruby.h>
#include <stdlib.h>
#include <time.h>
#include "khetai_lib.h"

VALUE move(VALUE self, VALUE board_array, VALUE player, VALUE max_depth, VALUE _max_time)
{
    srand((unsigned)time(NULL));

    char *init_board[120];
    unsigned int array_size = (unsigned int)RARRAY_LEN(board_array);
    for (unsigned int i = 0; i < array_size; i++)
    {
        VALUE square = rb_ary_entry(board_array, i);
        init_board[i] = StringValueCStr(square);
    }

    reset_undo();
    init_zobrist();
    setup_board(init_board);

    time_t start_time = time(NULL);
    int max_time = NUM2INT(_max_time);
    set_time_parameters(max_time, start_time);

    int depth = 1;
    Move best_move = (Move)0;
    Move current_move = (Move)0;
    while ((time(NULL) - start_time < max_time) && (depth <= NUM2INT(max_depth)))
    {
        best_move = current_move;
        current_move = alphabeta_root(depth, NUM2INT(player));
        depth++;
    }
    make_move(best_move);

    VALUE out = rb_ary_new2(3);
    rb_ary_store(out, 0, INT2NUM(get_start(best_move)));
    rb_ary_store(out, 1, INT2NUM(get_end(best_move)));
    rb_ary_store(out, 2, INT2NUM(get_rotation(best_move)));

    return out;
}

void Init_khetai()
{
    VALUE KhetAI = rb_define_module("KhetAI");
    rb_define_singleton_method(KhetAI, "move", move, 4);
}
