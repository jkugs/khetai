#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../khetai_lib.h"

char* init_board[120] =
    {"--", "--", "--", "--", "--", "--", "--", "--", "--", "--", "--", "--",
     "--", "L2", "--", "--", "--", "A2", "X2", "A2", "P1", "--", "--", "--",
     "--", "--", "--", "P2", "--", "--", "--", "--", "--", "--", "--", "--",
     "--", "--", "--", "--", "p3", "--", "--", "--", "--", "--", "--", "--",
     "--", "P0", "--", "p2", "--", "S2", "S3", "--", "P1", "--", "p3", "--",
     "--", "P1", "--", "p3", "--", "s1", "s0", "--", "P0", "--", "p2", "--",
     "--", "--", "--", "--", "--", "--", "--", "P1", "--", "--", "--", "--",
     "--", "--", "--", "--", "--", "--", "--", "--", "p0", "--", "--", "--",
     "--", "--", "--", "p3", "a0", "x0", "a0", "--", "--", "--", "l0", "--",
     "--", "--", "--", "--", "--", "--", "--", "--", "--", "--", "--", "--"};

time_t start_time;
int max_time;
     
int main() {
    init_zobrist();
    srand((unsigned) time(NULL));
    
    setup_board(init_board);
    print_board();
    
    start_time = time(NULL);
    max_time = 5;
    int max_depth = 25;
    
    int depth = 1;
    Move best_move = (Move)0;
    Move current_move = (Move)0;
    while((time(NULL) - start_time < max_time) && (depth <= max_depth))
    {
        best_move = current_move;
        current_move = alphabeta_root(depth, Red);
        depth++;
    }
    make_move(best_move);
    
    printf("\n========================\n\n");
    print_board();
    
    printf("\nDEPTH: %d\n", depth-1);
    
    return 0;
}
