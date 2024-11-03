#ifndef KHET_LIB_H_INCLUDED
#define KHET_LIB_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#include <ctime>
#endif

typedef uint8_t Square;
typedef uint32_t Move;

#define NUM_VALID_MOVES 125
#define MAX_SCORE 9999999
#define MAX_DEPTH 25

enum Player
{
    Silver,
    Red
};
enum Piece
{
    Anubis = 1,
    Pyramid = 2,
    Scarab = 3,
    Pharaoh = 4,
    Sphinx = 5
};
enum Orientation
{
    North,
    East,
    South,
    West
};

extern enum Player whose_turn;
extern enum Player starter;
extern int initial_depth;

// north, east, south, west, diagonals
static const int directions[8] = {-12, 1, 12, -1, (12 + 1), (12 - 1), (-12 + 1), (-12 - 1)};
static const int rotations[2] = {1, -1};
static const int sphinx_loc[2] = {106, 13};

extern int pharaoh_loc[2];

extern Square board[120];
extern Move undo_moves[MAX_DEPTH];
extern int undo_capture_indices[MAX_DEPTH];
extern Square undo_capture_squares[MAX_DEPTH];
extern int undo_index;

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
extern "C"
{
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

Square str_to_square(char *str);
void print_piece(Square s);

void find_valid_moves(Move *valid_moves, int *vi);
void find_valid_anubis_pyramid_moves(int i, Move *valid_moves, int *vi);
void find_valid_scarab_moves(int i, Move *valid_moves, int *vi);
void find_valid_pharaoh_moves(int i, Move *valid_moves, int *vi);
void find_valid_sphinx_moves(int i, Move *valid_moves, int *vi);

int alphabeta(int depth, enum Player player, int alpha, int beta);
int calculate_score();
int distance_from_pharaoh(int i, int p);

void undo_move();
void fire_laser(uint64_t *hash);
bool is_move_legal(Move move);

static inline bool is_piece(Square s) { return s > 0; }

static inline enum Player get_owner(Square s) { return (enum Player)(s >> 1 & 0x1); }
static inline enum Piece get_piece(Square s) { return (enum Piece)(s >> 2 & 0x7); }
static inline enum Orientation get_orientation(Square s) { return (enum Orientation)(s >> 5 & 0x7); }

static inline Move new_move(int start, int end, int rotation) { return start << 1 | end << 8 | (rotation + 2) << 15; }
static inline int get_start(Move m) { return m >> 1 & 0x7F; }
static inline int get_end(Move m) { return m >> 8 & 0x7F; }
static inline int get_rotation(Move m) { return (m >> 15 & 0x3) - 2; }

#ifdef __cplusplus
extern "C"
{
#endif
    int get_start_wrapper(Move move);
    int get_end_wrapper(Move move);
    int get_rotation_wrapper(Move move);
#ifdef __cplusplus
}
#endif

static inline Square rotate(Square s, int rotation)
{
    int orientation = get_orientation(s);
    orientation = (orientation + rotation) % 4;
    if (orientation < 0)
        orientation += 4;
    return (s & 0x1F) + (orientation << 5);
}

static inline enum Player opposite_player(enum Player player)
{
    return player == Red ? Silver : Red;
}

#define Dead -1
#define Absorbed -2

// [laser direciton][piece type][piece orientation] = reflection result
// anubis, pyramid, scarab, pharaoh, sphinx
static const int reflections[4][5][4] = {
    {// North
     {Dead, Dead, Absorbed, Dead},
     {Dead, East, West, Dead},
     {West, East, West, East},
     {Dead, Dead, Dead, Dead},
     {Absorbed, Absorbed, Absorbed, Absorbed}},
    {// East
     {Dead, Dead, Dead, Absorbed},
     {Dead, Dead, South, North},
     {South, North, South, North},
     {Dead, Dead, Dead, Dead},
     {Absorbed, Absorbed, Absorbed, Absorbed}},
    {// South
     {Absorbed, Dead, Dead, Dead},
     {East, Dead, Dead, West},
     {East, West, East, West},
     {Dead, Dead, Dead, Dead},
     {Absorbed, Absorbed, Absorbed, Absorbed}},
    {// West
     {Dead, Absorbed, Dead, Dead},
     {North, South, Dead, Dead},
     {North, South, North, South},
     {Dead, Dead, Dead, Dead},
     {Absorbed, Absorbed, Absorbed, Absorbed}}};

extern uint64_t keys[0xFF][120];
extern uint64_t hashes[MAX_DEPTH];
extern uint64_t turn_key;
extern int move_num;
extern bool checkmate;

uint64_t get_board_hash();
static uint64_t seed = 1070372;
static inline uint64_t random_number()
{
    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    return seed * 0x2545F4914F6CDD1DLL;
}

#define TABLE_SIZE 0x400000

#define EXACT 0
#define ALPHA 1
#define BETA 2
typedef struct HashEntry
{
    uint64_t key;
    int depth;
    int flag;
    int score;
    Move move;
} HashEntry;

extern HashEntry table[TABLE_SIZE];
static inline HashEntry *search_table(uint64_t key) { return &table[key % TABLE_SIZE]; };
void insert_table(HashEntry *entry, uint64_t key, int depth, int flag, int score, Move move);

#endif // KHET_LIB_H_INCLUDED
