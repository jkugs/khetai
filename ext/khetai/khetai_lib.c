#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "khetai_lib.h"

Square board[120] = {0};
enum Player whose_turn;

Move undo_moves[MAX_DEPTH] = {0};
int undo_capture_indices[MAX_DEPTH] = {0};
Square undo_capture_squares[MAX_DEPTH] = {0};

HashEntry table[TABLE_SIZE] = {0};
uint64_t hashes[MAX_DEPTH] = {0};
uint64_t keys[0xFF][120] = {0};

int undo_index = 0;
int move_num = 0;
bool checkmate = false;

Move alphabeta_root(int depth, enum Player player)
{
    whose_turn = player;
    int best_score = -MAX_SCORE;
    Move best_move = (Move)0;
    int alpha = -MAX_SCORE;
    int beta = MAX_SCORE;
    Move valid_moves[NUM_VALID_MOVES] = {0};
    int vi = 0;
    find_valid_moves(valid_moves, &vi);
    for (int i = 0; i < NUM_VALID_MOVES; i++)
    {
        if (valid_moves[i] == 0)
            break;
        make_move(valid_moves[i]);
        int score = -alphabeta(depth - 1, opposite_player(player), -beta, -alpha);
        undo_move();
        whose_turn = player;
        if (score > best_score)
        {
            best_score = score;
            best_move = valid_moves[i];
        }
        if (best_score > alpha)
            alpha = best_score;
        if (alpha >= beta)
            break;
    }
    return best_move;
}

int alphabeta(int depth, enum Player player, int alpha, int beta)
{
    if (depth == 0 || checkmate)
    {
        return player == Red ? calculate_score() : -calculate_score();
    }

    int alpha_orig = alpha;
    Move valid_moves[NUM_VALID_MOVES] = {0};
    int vi = 0;
    HashEntry *entry = search_table(hashes[move_num]);
    if (entry->key == hashes[move_num] && entry->depth >= depth)
    {
        if (entry->flag == EXACT)
            return entry->score;
        else if (entry->flag == ALPHA)
        {
            if (entry->score > alpha)
                alpha = entry->score;
        }
        else
        {
            if (entry->score < beta)
                beta = entry->score;
        }

        if (alpha >= beta)
            return entry->score;

        if (is_move_legal(entry->move))
            valid_moves[vi++] = entry->move;
    }

    whose_turn = player;
    find_valid_moves(valid_moves, &vi);
    int best_score = -MAX_SCORE;
    Move best_move = (Move)0;
    for (int i = 0; (i < NUM_VALID_MOVES && (time(NULL) - start_time < max_time)); i++)
    {
        if (valid_moves[i] == 0)
            break;
        make_move(valid_moves[i]);
        int score = -alphabeta(depth - 1, opposite_player(player), -beta, -alpha);
        undo_move();
        whose_turn = player;
        if (score > best_score)
        {
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
        flag = BETA;
    else if (best_score >= beta)
        flag = ALPHA;

    insert_table(hashes[move_num], depth, flag, best_score, best_move);
    return best_score;
}

void insert_table(uint64_t key, int depth, int flag, int score, Move move)
{
    HashEntry *entry = search_table(key);
    if (entry->key != 0)
    {
        if (depth > entry->depth)
        {
            entry->key = key;
            entry->depth = depth;
            entry->flag = flag;
            entry->score = score;
            entry->move = move;
        }
    }
    else
    {
        entry->key = key;
        entry->depth = depth;
        entry->flag = flag;
        entry->score = score;
        entry->move = move;
    }
}

int calculate_score()
{
    int score = 0;
    int anubis_score = 500;
    int scarab_score = 750;
    int pyramid_score = 1000;
    int pharaoh_score = 100000;
    for (int i = 0; i < 120; i++)
    {
        if (is_piece(board[i]))
        {
            int value = 0;
            switch (get_piece(board[i]))
            {
            case Anubis:
                value += anubis_score;
                break;
            case Pyramid:
                value += pyramid_score;
                break;
            case Scarab:
                value += scarab_score;
                break;
            case Pharaoh:
                value += pharaoh_score;
                break;
            default:
                break;
            }
            score += get_owner(board[i]) == Red ? value : -value;
        }
    }
    return score += rand() % 100;
}

void make_move(Move move)
{
    uint64_t hash = hashes[move_num++];

    int start = get_start(move);
    // remove starting piece
    hash ^= keys[board[start]][start];
    int end = get_end(move);
    int rotation = get_rotation(move);

    if (rotation != 0)
    {
        board[start] = rotate(board[start], rotation);
        // add starting piece back with rotation
        hash ^= keys[board[start]][start];
    }
    else
    {
        // remove ending piece if swapping
        if (is_piece(board[end]))
            hash ^= keys[board[end]][end];

        Square moving_piece = board[start];
        board[start] = board[end];
        board[end] = moving_piece;
        // add starting piece to end location
        hash ^= keys[board[end]][end];

        // add ending piece to start position if swapping
        if (is_piece(board[start]))
            hash ^= keys[board[start]][start];
    }

    undo_moves[undo_index] = new_move(end, start, -rotation);
    fire_laser(&hash);
    hashes[move_num] = hash;
    undo_index++;
}

void fire_laser(uint64_t *hash)
{
    int i = sphinx_loc[whose_turn];
    int laser_dir = get_orientation(board[i]);
    bool traversing = true;
    while (traversing)
    {
        i = i + directions[laser_dir];
        if (i >= 0 && i < 120 && on_board[i] == 1)
        {
            if (is_piece(board[i]))
            {
                int piece = get_piece(board[i]) - 1;
                int orientation = get_orientation(board[i]);
                int result = reflections[laser_dir][piece][orientation];
                if (result == Dead)
                {
                    if (get_piece(board[i]) == Pharaoh)
                        checkmate = true;
                    *hash ^= keys[board[i]][i];
                    undo_capture_indices[undo_index] = i;
                    undo_capture_squares[undo_index] = board[i];
                    board[i] = (Square)0;
                    traversing = false;
                }
                else if (result == Absorbed)
                {
                    traversing = false;
                }
                else
                {
                    laser_dir = result;
                }
            }
        }
        else
        {
            traversing = false;
        }
    }
}

void undo_move()
{
    move_num--;
    undo_index--;

    Square captured = (Square)undo_capture_squares[undo_index];
    undo_capture_squares[undo_index] = 0;
    if (captured > 0)
    {
        board[undo_capture_indices[undo_index]] = captured;
        undo_capture_indices[undo_index] = 0;
    }

    Move move = undo_moves[undo_index];
    undo_moves[undo_index] = 0;
    int start = get_start(move);
    int end = get_end(move);
    int rotation = get_rotation(move);

    if (rotation != 0)
    {
        board[start] = rotate(board[start], rotation);
    }
    else
    {
        Square moving_piece = board[start];
        board[start] = board[end];
        board[end] = moving_piece;
    }
    checkmate = false;
}

void find_valid_moves(Move *valid_moves, int *vi)
{
    for (int i = 0; i < 120; i++)
    {
        Square s = board[i];
        enum Player piece_color = get_owner(s);
        if (is_piece(s) && piece_color == whose_turn)
        {
            enum Piece piece = get_piece(s);
            switch (piece)
            {
            case Anubis:
                find_valid_anubis_pyramid_moves(i, valid_moves, vi);
                break;
            case Pyramid:
                find_valid_anubis_pyramid_moves(i, valid_moves, vi);
                break;
            case Scarab:
                find_valid_scarab_moves(i, valid_moves, vi);
                break;
            case Pharaoh:
                find_valid_pharaoh_moves(i, valid_moves, vi);
                break;
            case Sphinx:
                find_valid_sphinx_moves(i, valid_moves, vi);
                break;
            default:
                break;
            }
        }
    }
}

void find_valid_anubis_pyramid_moves(int i, Move *valid_moves, int *vi)
{
    for (int j = 0; j < 8; j++)
    {
        int dest = i + directions[j];
        if (!is_piece(board[dest]) && can_move[whose_turn][dest])
        {
            valid_moves[(*vi)++] = new_move(i, dest, 0);
        }
    }
    for (int j = 0; j < 2; j++)
    {
        valid_moves[(*vi)++] = new_move(i, i, rotations[j]);
    }
}

void find_valid_scarab_moves(int i, Move *valid_moves, int *vi)
{
    for (int j = 0; j < 8; j++)
    {
        int dest = i + directions[j];
        if (can_move[whose_turn][dest])
        {
            if (is_piece(board[dest]))
            {
                if (get_piece(board[dest]) == Anubis || get_piece(board[dest]) == Pyramid)
                {
                    valid_moves[(*vi)++] = new_move(i, dest, 0);
                }
            }
            else
            {
                valid_moves[(*vi)++] = new_move(i, dest, 0);
            }
        }
    }
    for (int j = 0; j < 2; j++)
    {
        valid_moves[(*vi)++] = new_move(i, i, rotations[j]);
    }
}

void find_valid_pharaoh_moves(int i, Move *valid_moves, int *vi)
{
    for (int j = 0; j < 8; j++)
    {
        int dest = i + directions[j];
        if (!is_piece(board[dest]) && can_move[whose_turn][dest])
        {
            valid_moves[(*vi)++] = new_move(i, dest, 0);
        }
    }
}

void find_valid_sphinx_moves(int i, Move *valid_moves, int *vi)
{
    enum Player player = get_owner(board[i]);
    enum Orientation orientation = get_orientation(board[i]);
    int rotation = player == Silver ? (orientation == North ? -1 : 1) : (orientation == South ? -1 : 1);
    valid_moves[(*vi)++] = new_move(i, i, rotation);
}

void setup_board(char *init_board[120])
{
    uint64_t hash = 0;
    for (int i = 0; i < 120; i++)
    {
        board[i] = str_to_square(init_board[i]);
        if (is_piece(board[i]))
            hash ^= keys[board[i]][i];
    }
    hashes[0] = hash;

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        table[i].key = 0;
        table[i].depth = 0;
        table[i].flag = 0;
        table[i].score = 0;
        table[i].move = 0;
    }
}

Square str_to_square(char *str)
{
    enum Player player;
    enum Piece piece;
    enum Orientation orientation;

    if (str[0] != '-')
    {
        if (islower(str[0]))
            player = Silver;
        else
            player = Red;

        char p = tolower(str[0]);
        if (p == 'a')
            piece = Anubis;
        else if (p == 'p')
            piece = Pyramid;
        else if (p == 's')
            piece = Scarab;
        else if (p == 'x')
            piece = Pharaoh;
        else
            piece = Sphinx;

        char o = str[1];
        if (o == '0')
            orientation = North;
        else if (o == '1')
            orientation = East;
        else if (o == '2')
            orientation = South;
        else
            orientation = West;

        return (Square)((int)player << 1 | (int)piece << 2 | (int)orientation << 5);
    }

    return (Square)0;
}

void print_board()
{
    for (int i = 0; i < 120; i++)
    {
        if (is_piece(board[i]))
        {
            enum Player player = get_owner(board[i]);
            enum Piece piece = get_piece(board[i]);
            print_piece(player, piece);

            enum Orientation orientation = get_orientation(board[i]);
            switch (orientation)
            {
            case North:
                printf("0");
                break;
            case East:
                printf("1");
                break;
            case South:
                printf("2");
                break;
            case West:
                printf("3");
                break;
            default:
                break;
            }
        }
        else
        {
            printf("--");
        }

        if ((i + 1) % 12 == 0)
            printf("\n");
    }
}

void print_piece(enum Player player, enum Piece piece)
{
    switch (piece)
    {
    case Anubis:
        if (player == Silver)
            printf("a");
        else
            printf("A");
        break;

    case Pyramid:
        if (player == Silver)
            printf("p");
        else
            printf("P");
        break;

    case Scarab:
        if (player == Silver)
            printf("s");
        else
            printf("S");
        break;

    case Pharaoh:
        if (player == Silver)
            printf("x");
        else
            printf("X");
        break;

    case Sphinx:
        if (player == Silver)
            printf("l");
        else
            printf("L");
        break;

    default:
        break;
    }
}

void reset_undo()
{
    undo_index = 0;
    for (int i = 0; i < MAX_DEPTH; i++)
    {
        undo_moves[i] = 0;
        undo_capture_indices[i] = 0;
        undo_capture_squares[i] = 0;
    }
}

void init_zobrist()
{
    for (int i = 0; i < 0xFF; i++)
    {
        for (int j = 0; j < 120; j++)
        {
            keys[i][j] = random_number();
        }
    }
}

bool is_move_legal(Move move)
{
    int start = get_start(move);
    int end = get_end(move);
    if (is_piece(board[start]) && get_owner(board[start]) == whose_turn)
    {
        if (!is_piece(board[end]) || get_rotation(move) != 0)
            return true;
        else if (is_piece(board[end]) && get_piece(board[start]) == Scarab && get_piece(board[end]) < 3)
            return true;
    }
    return false;
}
