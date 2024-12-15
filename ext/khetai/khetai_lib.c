#include "khetai_lib.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int max_time;
time_t start_time;

static Square board[120] = {0};
static int pharaoh_loc[2] = {0};
static enum Player whose_turn;
static enum Player starter;
static int initial_depth = 0;

static Move undo_moves[MAX_DEPTH] = {0};
static int undo_capture_indices[MAX_DEPTH] = {0};
static Square undo_capture_squares[MAX_DEPTH] = {0};

PieceTracker piece_trackers[2] = {0};

HashEntry table[TABLE_SIZE] = {0};
static uint64_t hashes[MAX_DEPTH] = {0};
static uint64_t keys[0xFF][120] = {0};
static uint64_t turn_key = 0;

static int undo_index = 0;
static int hashes_index = 0;
static bool checkmate = false;

static int alphabeta(int depth, enum Player player, int alpha, int beta);
static void insert_table(HashEntry *entry, uint64_t key, int depth, int flag, int score, Move move);
static int calculate_score(void);
static int distance_from_pharaoh(int i, int p);
static void fire_laser(uint64_t *hash);
static void undo_move();
static void find_valid_moves(Move *valid_moves, int *vi);
static void find_valid_anubis_pyramid_moves(int i, Move *valid_moves, int *vi);
static void find_valid_scarab_moves(int i, Move *valid_moves, int *vi);
static void find_valid_pharaoh_moves(int i, Move *valid_moves, int *vi);
static void find_valid_sphinx_moves(int i, Move *valid_moves, int *vi);
static uint64_t get_board_hash();
static void init_piece_trackers();
static bool is_move_legal(Move move);
static Square str_to_square(char *str);
static void print_piece(Square s);

Move alphabeta_root(int depth, enum Player player) {
    whose_turn = player;
    starter = player;
    initial_depth = depth;
    int best_score = -MAX_SCORE;
    Move best_move = (Move)0;
    int alpha = -MAX_SCORE;
    int beta = MAX_SCORE;
    Move valid_moves[NUM_VALID_MOVES] = {0};
    int vi = 0;
    find_valid_moves(valid_moves, &vi);
    for (int i = 0; i < NUM_VALID_MOVES; i++) {
        if (valid_moves[i] == 0)
            break;
        make_move(valid_moves[i]);
        int score = -alphabeta(depth - 1, opposite_player(player), -beta, -alpha);
        undo_move();
        whose_turn = player;
        if (score > best_score) {
            best_score = score;
            best_move = valid_moves[i];
        }
        if (best_score > alpha)
            alpha = best_score;
        if (alpha >= beta)
            break;
    }

    // print final score for testing purposes:
    // printf("SCORE: %-10d\n", best_score);
    return best_move;
}

int alphabeta(int depth, enum Player player, int alpha, int beta) {
    whose_turn = player;
    if (depth == 0 || checkmate) {
        return player == RED ? calculate_score() : -calculate_score();
    }

    int alpha_orig = alpha;
    Move valid_moves[NUM_VALID_MOVES] = {0};
    int vi = 0;

    int table_depth = initial_depth - depth;
    HashEntry *entry = search_table(hashes[hashes_index]);
    if (entry->key == hashes[hashes_index] && entry->depth > table_depth && is_move_legal(entry->move)) {
        valid_moves[vi++] = entry->move;

        if (entry->flag == EXACT)
            return entry->score;
        else if (entry->flag == LOWERBOUND && entry->score > alpha)
            alpha = entry->score;
        else if (entry->flag == UPPERBOUND && entry->score < beta)
            beta = entry->score;

        if (alpha >= beta)
            return entry->score;
    }

    find_valid_moves(valid_moves, &vi);
    int best_score = -MAX_SCORE;
    Move best_move = (Move)0;

    for (int i = 0; (i < NUM_VALID_MOVES && (time(NULL) - start_time < max_time)); i++) {
        if (valid_moves[i] == 0)
            break;
        make_move(valid_moves[i]);
        int score = -alphabeta(depth - 1, opposite_player(player), -beta, -alpha);
        undo_move();
        whose_turn = player;
        if (score > best_score) {
            best_score = score;
            best_move = valid_moves[i];
        }
        if (best_score > alpha)
            alpha = best_score;
        if (alpha >= beta)
            break;
    }

    int flag = EXACT;
    if (best_score <= alpha_orig)
        flag = UPPERBOUND;
    else if (best_score >= beta)
        flag = LOWERBOUND;

    insert_table(entry, hashes[hashes_index], table_depth, flag, best_score, best_move);
    return best_score;
}

void insert_table(HashEntry *entry, uint64_t key, int table_depth, int flag, int score, Move move) {
    if (entry->key == 0 || table_depth > entry->depth) {
        entry->key = key;
        entry->depth = table_depth;
        entry->flag = flag;
        entry->score = score;
        entry->move = move;
    }
}

int calculate_score() {
    int score = 0;
    int anubis_score = 800;
    int pyramid_score = 1000;
    int pharaoh_score = 100000;

    for (int j = 0; j < 2; j++) {
        enum Player player = j;
        for (int k = 0; k < 13; k++) {
            int i = get_board_index(player, k);
            if (i != EMPTY) {
                Square s = board[i];
                int value = 0;
                switch (get_piece(s)) {
                case ANUBIS:
                    value += anubis_score;
                    value -= distance_from_pharaoh(i, pharaoh_loc[player]) * 10;
                    break;
                case PYRAMID:
                    value += pyramid_score;
                    value += (rand() % 51) - 25;
                    break;
                case SCARAB:
                    int max_distance = 16;
                    int base_score = 1000;
                    value += (max_distance - distance_from_pharaoh(i, pharaoh_loc[opposite_player(player)])) * base_score / max_distance;
                    break;
                case PHARAOH:
                    value += pharaoh_score;
                    break;
                default:
                    break;
                }
                score += get_owner(s) == RED ? value : -value;
            }
        }
    }
    return score;
}

int distance_from_pharaoh(int i, int p) {
    int px = p / 12;
    int py = p % 12;
    int ix = i / 12;
    int iy = i % 12;
    int m_distance = abs(px - ix) + abs(py - iy);
    return m_distance;
}

void make_move(Move move) {
    uint64_t hash = hashes[hashes_index++];

    int start = get_start(move);
    // remove starting piece
    hash ^= keys[board[start]][start];
    int end = get_end(move);
    int rotation = get_rotation(move);

    if (rotation != 0) {
        board[start] = rotate(board[start], rotation);
        // add starting piece back with rotation
        hash ^= keys[board[start]][start];
    } else {
        // remove ending piece
        if (is_piece(board[end]))
            hash ^= keys[board[end]][end];

        Square starting_square = board[start];
        Square ending_square = board[end];
        board[start] = ending_square;
        board[end] = starting_square;

        // add starting piece to end location
        hash ^= keys[starting_square][end];

        enum Player moving_player = get_owner(starting_square);
        if (!is_piece(ending_square)) {
            update_piece_tracker(moving_player, start, end, false);
        } else {
            // add ending piece to start location if swapping
            hash ^= keys[ending_square][start];

            enum Player other_player = get_owner(ending_square);
            if (get_owner(starting_square) == other_player)
                update_piece_tracker(other_player, end, start, true);
            else {
                update_piece_tracker(moving_player, start, end, false);
                update_piece_tracker(other_player, end, start, false);
            }
        }

        if (get_piece(starting_square) == PHARAOH)
            pharaoh_loc[get_owner(starting_square)] = end;
    }

    undo_moves[undo_index] = new_move(end, start, -rotation);

    fire_laser(&hash);
    hash ^= turn_key;

    // testing that hashing works properly
    // printf("\nHASH:\t%lu\nBOARD:\t%lu\n", hash, get_board_hash());

    hashes[hashes_index] = hash;
    undo_index++;
}

void fire_laser(uint64_t *hash) {
    int i = sphinx_loc[whose_turn];
    int laser_dir = get_orientation(board[i]);
    bool traversing = true;
    while (traversing) {
        i = i + directions[laser_dir];
        if (i >= 0 && i < 120 && on_board[i] == 1) {
            Square s = board[i];
            if (is_piece(s)) {
                int piece = get_piece(s) - 1;
                int orientation = get_orientation(s);
                int result = reflections[laser_dir][piece][orientation];
                if (result == DEAD) {
                    if (get_piece(s) == PHARAOH)
                        checkmate = true;
                    // remove piece
                    *hash ^= keys[s][i];

                    enum Player remove_player = get_owner(s);
                    remove_from_piece_tracker(remove_player, i);

                    undo_capture_indices[undo_index] = i;
                    undo_capture_squares[undo_index] = s;
                    board[i] = (Square)0;
                    traversing = false;
                } else if (result == ABSORBED) {
                    traversing = false;
                } else {
                    laser_dir = result;
                }
            }
        } else {
            traversing = false;
        }
    }
}

void undo_move() {
    hashes_index--;
    undo_index--;

    Square captured = (Square)undo_capture_squares[undo_index];
    undo_capture_squares[undo_index] = 0;
    if (captured > 0) {
        uint8_t board_pos = undo_capture_indices[undo_index];
        board[board_pos] = captured;
        undo_capture_indices[undo_index] = 0;

        enum Player captured_player = get_owner(captured);
        add_to_piece_tracker(captured_player, board_pos);
    }

    Move move = undo_moves[undo_index];
    undo_moves[undo_index] = 0;
    int start = get_start(move);
    int end = get_end(move);
    int rotation = get_rotation(move);

    if (rotation != 0) {
        board[start] = rotate(board[start], rotation);
    } else {
        Square starting_square = board[start];
        Square ending_square = board[end];
        board[start] = ending_square;
        board[end] = starting_square;

        enum Player moving_player = get_owner(starting_square);
        if (ending_square == 0) {
            update_piece_tracker(moving_player, start, end, false);
        } else {
            enum Player other_player = get_owner(ending_square);

            if (get_owner(starting_square) == other_player)
                update_piece_tracker(other_player, end, start, true);
            else {
                update_piece_tracker(moving_player, start, end, false);
                update_piece_tracker(other_player, end, start, false);
            }
        }

        if (get_piece(starting_square) == PHARAOH)
            pharaoh_loc[get_owner(starting_square)] = end;
    }
    checkmate = false;
}

void find_valid_moves(Move *valid_moves, int *vi) {
    for (int i = 0; i < 13; i++) {
        uint8_t board_pos = piece_trackers[whose_turn].positions[i];
        if (board_pos != EMPTY) {
            enum Piece piece = get_piece(board[board_pos]);
            switch (piece) {
            case ANUBIS:
                find_valid_anubis_pyramid_moves(board_pos, valid_moves, vi);
                break;
            case PYRAMID:
                find_valid_anubis_pyramid_moves(board_pos, valid_moves, vi);
                break;
            case SCARAB:
                find_valid_scarab_moves(board_pos, valid_moves, vi);
                break;
            case PHARAOH:
                find_valid_pharaoh_moves(board_pos, valid_moves, vi);
                break;
            case SPHINX:
                find_valid_sphinx_moves(board_pos, valid_moves, vi);
                break;
            default:
                break;
            }
        }
    }
}

void find_valid_anubis_pyramid_moves(int i, Move *valid_moves, int *vi) {
    for (int j = 0; j < 8; j++) {
        int dest = i + directions[j];
        if (!is_piece(board[dest]) && can_move[whose_turn][dest]) {
            valid_moves[(*vi)++] = new_move(i, dest, 0);
        }
    }
    for (int j = 0; j < 2; j++) {
        valid_moves[(*vi)++] = new_move(i, i, rotations[j]);
    }
}

void find_valid_scarab_moves(int i, Move *valid_moves, int *vi) {
    for (int j = 0; j < 8; j++) {
        int dest = i + directions[j];
        if (can_move[whose_turn][dest]) {
            if (!is_piece(board[dest]) || (get_piece(board[dest]) != SCARAB && get_piece(board[dest]) != PHARAOH)) {
                valid_moves[(*vi)++] = new_move(i, dest, 0);
            }
        }
    }
    for (int j = 0; j < 2; j++) {
        valid_moves[(*vi)++] = new_move(i, i, rotations[j]);
    }
}

void find_valid_pharaoh_moves(int i, Move *valid_moves, int *vi) {
    for (int j = 0; j < 8; j++) {
        int dest = i + directions[j];
        if (!is_piece(board[dest]) && can_move[whose_turn][dest]) {
            valid_moves[(*vi)++] = new_move(i, dest, 0);
        }
    }
}

void find_valid_sphinx_moves(int i, Move *valid_moves, int *vi) {
    enum Player player = get_owner(board[i]);
    enum Orientation orientation = get_orientation(board[i]);
    int rotation = player == SILVER ? (orientation == NORTH ? -1 : 1) : (orientation == SOUTH ? -1 : 1);
    valid_moves[(*vi)++] = new_move(i, i, rotation);
}

void init_zobrist() {
    for (int i = 0; i < 0xFF; i++) {
        for (int j = 0; j < 120; j++) {
            keys[i][j] = random_number();
        }
    }
    turn_key = random_number();
}

uint64_t get_board_hash() {
    uint64_t hash = 0;
    for (int i = 0; i < 120; i++) {
        if (is_piece(board[i]))
            hash ^= keys[board[i]][i];
    }
    if (whose_turn == starter)
        hash ^= turn_key;
    return hash;
}

void init_piece_trackers() {
    for (int i = 0; i < 13; i++) {
        piece_trackers[SILVER].positions[i] = EMPTY;
        piece_trackers[RED].positions[i] = EMPTY;
    }

    int si = 0;
    int ri = 0;

    for (int i = 0; i < 120; i++) {
        Square s = board[i];
        if (is_piece(s)) {
            if (get_owner(s) == SILVER) {
                piece_trackers[SILVER].positions[si] = i;
                piece_trackers[SILVER].board_idx_position[i] = si;
                si++;
            } else if (get_owner(s) == RED) {
                piece_trackers[RED].positions[ri] = i;
                piece_trackers[RED].board_idx_position[i] = ri;
                ri++;
            }
        } else {
            piece_trackers[SILVER].board_idx_position[i] = EMPTY;
            piece_trackers[RED].board_idx_position[i] = EMPTY;
        }
    }
}

bool is_move_legal(Move move) {
    int start = get_start(move);
    int end = get_end(move);
    if (is_piece(board[start]) && get_owner(board[start]) == whose_turn) {
        if (!is_piece(board[end]) || (get_rotation(move) != 0 && start == end))
            return true;
        else if (is_piece(board[end]) && (get_piece(board[start]) == SCARAB && get_piece(board[end]) < 3))
            return true;
    }
    return false;
}

void reset_undo() {
    undo_index = 0;
    for (int i = 0; i < MAX_DEPTH; i++) {
        undo_moves[i] = 0;
        undo_capture_indices[i] = 0;
        undo_capture_squares[i] = 0;
    }
}

void setup_board(char *init_board[120]) {
    hashes_index = 0;
    uint64_t hash = 0;
    for (int i = 0; i < 120; i++) {
        board[i] = str_to_square(init_board[i]);
        Square s = board[i];
        if (is_piece(s)) {
            hash ^= keys[s][i];
            if (get_piece(s) == PHARAOH) {
                if (get_owner(s) == SILVER)
                    pharaoh_loc[SILVER] = i;
                else if (get_owner(s) == RED)
                    pharaoh_loc[RED] = i;
            }
        }
    }
    hashes[hashes_index] = hash;

    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i].key = 0;
        table[i].depth = 0;
        table[i].flag = 0;
        table[i].score = 0;
        table[i].move = 0;
    }

    init_piece_trackers();
}

Square str_to_square(char *str) {
    enum Player player;
    enum Piece piece;
    enum Orientation orientation;

    if (str[0] != '-') {
        if (islower(str[0]))
            player = SILVER;
        else
            player = RED;

        char p = tolower(str[0]);
        if (p == 'a')
            piece = ANUBIS;
        else if (p == 'p')
            piece = PYRAMID;
        else if (p == 's')
            piece = SCARAB;
        else if (p == 'x')
            piece = PHARAOH;
        else
            piece = SPHINX;

        char o = str[1];
        if (o == '0')
            orientation = NORTH;
        else if (o == '1')
            orientation = EAST;
        else if (o == '2')
            orientation = SOUTH;
        else
            orientation = WEST;

        return (Square)((int)player << 1 | (int)piece << 2 | (int)orientation << 5);
    }

    return (Square)0;
}

void print_board() {
    for (int i = 0; i < 120; i++) {
        print_piece(board[i]);
        if ((i + 1) % 12 == 0)
            printf("\n");
    }
}

void print_piece(Square s) {
    enum Player player = get_owner(s);
    if (is_piece(s)) {
        enum Piece piece = get_piece(s);
        enum Orientation orientation = get_orientation(s);
        switch (piece) {
        case ANUBIS:
            if (player == SILVER)
                printf("a");
            else
                printf("A");
            break;

        case PYRAMID:
            if (player == SILVER)
                printf("p");
            else
                printf("P");
            break;

        case SCARAB:
            if (player == SILVER)
                printf("s");
            else
                printf("S");
            break;

        case PHARAOH:
            if (player == SILVER)
                printf("x");
            else
                printf("X");
            break;

        case SPHINX:
            if (player == SILVER)
                printf("l");
            else
                printf("L");
            break;
        default:
            printf("-");
            break;
        }
        switch (orientation) {
        case NORTH:
            printf("0");
            break;
        case EAST:
            printf("1");
            break;
        case SOUTH:
            printf("2");
            break;
        case WEST:
            printf("3");
            break;
        default:
            printf("-");
            break;
        }
    } else
        printf("--");
}

int get_start_wrapper(Move move) { return get_start(move); }
int get_end_wrapper(Move move) { return get_end(move); }
int get_rotation_wrapper(Move move) { return get_rotation(move); }

void set_time_parameters(int _max_time, time_t _start_time) {
    max_time = _max_time;
    start_time = _start_time;
}
