#include <ruby.h>
#include <stdlib.h>
#include <time.h>
#include "khetai_lib.h"

VALUE move(VALUE self, VALUE board_array, VALUE player, VALUE depth) {
    Check_Type(board_array, T_ARRAY);

    srand((unsigned) time(NULL));
    
    char* init_board[120];
    unsigned int array_size = (unsigned int)RARRAY_LEN(board_array);
    for (unsigned int i = 0; i < array_size; i++) {
        VALUE square = rb_ary_entry(board_array, i);
        init_board[i] = StringValueCStr(square);
    }
    
    setup_board(init_board);
    whose_turn = NUM2INT(player);
    Move move = alphabeta_root(NUM2INT(depth));
    make_move(move);
    
    VALUE out = rb_ary_new2(3);
    rb_ary_store(out, 0, INT2NUM(get_start(move)));   
    rb_ary_store(out, 1, INT2NUM(get_end(move)));   
    rb_ary_store(out, 2, INT2NUM(get_rotation(move)));
    
    return out;
}

void Init_khetai(){
    VALUE KhetAI = rb_define_module("KhetAI");
    rb_define_singleton_method(KhetAI, "move", move, 3);
}
