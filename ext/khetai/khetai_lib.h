#ifndef KHET_LIB_H_INCLUDED
#define KHET_LIB_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef uint8_t Square;
typedef uint32_t Move;

#define NUM_VALID_MOVES 125
#define MAX_SCORE 9999999
#define MAX_DEPTH 25

enum Player {
    SILVER,
    RED
};
enum Piece {
    ANUBIS = 1,
    PYRAMID = 2,
    SCARAB = 3,
    PHARAOH = 4,
    SPHINX = 5
};
enum Orientation {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

// north, east, south, west, diagonals
static const int directions[8] = {-12, 1, 12, -1, (12 + 1), (12 - 1), (-12 + 1), (-12 - 1)};
static const int rotations[2] = {1, -1};
static const int sphinx_loc[2] = {106, 13};

static const int can_move[2][120] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0,
     0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
     0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
     0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
     0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
     0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
     0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
     0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
     0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
     0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
     0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
     0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
     0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
     0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
     0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

static const int on_board[120] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

extern time_t start_time;
extern int max_time;

#ifdef __cplusplus
extern "C" {
#endif

void set_time_parameters(int _max_time, time_t _start_time);
void reset_undo();
void init_zobrist();
void setup_board(char *board[]);
Move alphabeta_root(int depth, enum Player player);
void make_move(Move move);
void print_board();

#ifdef __cplusplus
}
#endif

static inline bool is_piece(Square s) { return s > 0; }

static inline enum Player get_owner(Square s) { return (enum Player)(s >> 1 & 0x1); }
static inline enum Piece get_piece(Square s) { return (enum Piece)(s >> 2 & 0x7); }
static inline enum Orientation get_orientation(Square s) { return (enum Orientation)(s >> 5 & 0x7); }

static inline Move new_move(int start, int end, int rotation) { return start << 1 | end << 8 | (rotation + 2) << 15; }
static inline int get_start(Move m) { return m >> 1 & 0x7F; }
static inline int get_end(Move m) { return m >> 8 & 0x7F; }
static inline int get_rotation(Move m) { return (m >> 15 & 0x3) - 2; }

#ifdef __cplusplus
extern "C" {
#endif
int get_start_wrapper(Move move);
int get_end_wrapper(Move move);
int get_rotation_wrapper(Move move);
#ifdef __cplusplus
}
#endif

static inline Square rotate(Square s, int rotation) {
    int orientation = get_orientation(s);
    orientation = (orientation + rotation) % 4;
    if (orientation < 0)
        orientation += 4;
    return (s & 0x1F) + (orientation << 5);
}

static inline enum Player opposite_player(enum Player player) {
    return player == RED ? SILVER : RED;
}

#define DEAD -1
#define ABSORBED -2

// [laser direciton][piece type][piece orientation] = reflection result
// anubis, pyramid, scarab, pharaoh, sphinx
static const int reflections[4][5][4] = {
    {// North
     {DEAD, DEAD, ABSORBED, DEAD},
     {DEAD, EAST, WEST, DEAD},
     {WEST, EAST, WEST, EAST},
     {DEAD, DEAD, DEAD, DEAD},
     {ABSORBED, ABSORBED, ABSORBED, ABSORBED}},
    {// East
     {DEAD, DEAD, DEAD, ABSORBED},
     {DEAD, DEAD, SOUTH, NORTH},
     {SOUTH, NORTH, SOUTH, NORTH},
     {DEAD, DEAD, DEAD, DEAD},
     {ABSORBED, ABSORBED, ABSORBED, ABSORBED}},
    {// South
     {ABSORBED, DEAD, DEAD, DEAD},
     {EAST, DEAD, DEAD, WEST},
     {EAST, WEST, EAST, WEST},
     {DEAD, DEAD, DEAD, DEAD},
     {ABSORBED, ABSORBED, ABSORBED, ABSORBED}},
    {// West
     {DEAD, ABSORBED, DEAD, DEAD},
     {NORTH, SOUTH, DEAD, DEAD},
     {NORTH, SOUTH, NORTH, SOUTH},
     {DEAD, DEAD, DEAD, DEAD},
     {ABSORBED, ABSORBED, ABSORBED, ABSORBED}}};

static uint64_t seed = 1070372;
static inline uint64_t random_number() {
    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    return seed * 0x2545F4914F6CDD1DLL;
}

#define TABLE_SIZE 0x400000
#define TABLE_MASK 0x3FFFFF

#define EXACT 0
#define LOWERBOUND 1
#define UPPERBOUND 2
typedef struct HashEntry {
    uint64_t key;
    int depth;
    int flag;
    int score;
    Move move;
} HashEntry;

extern HashEntry table[TABLE_SIZE];
static inline HashEntry *search_table(uint64_t key) { return &table[key & TABLE_MASK]; };

#define NO_SWAP 0
#define SWAP_SAME 1
#define SWAP_OTHER 2
#define EMPTY 0xFF
typedef struct PieceTracker {
    uint8_t positions[13];
    uint8_t board_idx_position[120];
} PieceTracker;
extern PieceTracker piece_trackers[2];

static inline uint8_t get_board_index(enum Player player, uint8_t pos_idx) { return piece_trackers[player].positions[pos_idx]; }
static inline uint8_t get_position_index(enum Player player, uint8_t board_idx) { return piece_trackers[player].board_idx_position[board_idx]; }
static inline void update_piece_tracker(enum Player player, uint8_t old_board_idx, uint8_t new_board_idx, bool swap) {
    uint8_t pos_idx = get_position_index(player, old_board_idx);
    if (!swap) {
        piece_trackers[player].positions[pos_idx] = new_board_idx;
        piece_trackers[player].board_idx_position[old_board_idx] = EMPTY;
        piece_trackers[player].board_idx_position[new_board_idx] = pos_idx;
    } else {
        uint8_t other_pos_idx = get_position_index(player, new_board_idx);
        piece_trackers[player].positions[pos_idx] = new_board_idx;
        piece_trackers[player].positions[other_pos_idx] = old_board_idx;
        piece_trackers[player].board_idx_position[new_board_idx] = pos_idx;
        piece_trackers[player].board_idx_position[old_board_idx] = other_pos_idx;
    }
}
static inline void remove_from_piece_tracker(enum Player player, uint8_t board_idx) {
    uint8_t pos_idx = get_position_index(player, board_idx);
    piece_trackers[player].positions[pos_idx] = EMPTY;
    piece_trackers[player].board_idx_position[board_idx] = EMPTY;
}
static inline void add_to_piece_tracker(enum Player player, uint8_t board_idx) {
    uint8_t pos_idx = 0;
    while (piece_trackers[player].positions[pos_idx] != EMPTY)
        pos_idx++;
    piece_trackers[player].positions[pos_idx] = board_idx;
    piece_trackers[player].board_idx_position[board_idx] = pos_idx;
}

#endif // KHET_LIB_H_INCLUDED
