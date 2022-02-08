#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "khetai_lib.h"

Square board[120] = {0};
enum Player whose_turn;
Move undo_moves[25] = {0};
int undo_capture_indices[25] = {0};
Square undo_capture_squares[25] = {0};
int undo_index = 0;

Move alphabeta_root(int depth)
{
    Move best_move = (Move)0;
    Move valid_moves[NUM_VALID_MOVES] = {0};
    find_valid_moves(valid_moves);
    int alpha = -9999999;
    int beta = 9999999;
    if (whose_turn == Red)
    {
        int best_score = -9999999;
        for (int i = 0; i < NUM_VALID_MOVES; i++)
        {
            if (valid_moves[i] == 0)
                break;
            make_move(valid_moves[i]);
            int score = alphabeta(depth - 1, false, alpha, beta);
            undo_move();
            whose_turn = Red;
            if (score > best_score)
            {
                best_score = score;
                best_move = valid_moves[i];
            }
            if (score > alpha)
                alpha = score;
            if (beta <= alpha)
                break;
        }
    }
    else
    {
        int best_score = 9999999;
        for (int i = 0; i < NUM_VALID_MOVES; i++)
        {
            if (valid_moves[i] == 0)
                break;
            make_move(valid_moves[i]);
            int score = alphabeta(depth - 1, true, alpha, beta);
            undo_move();
            whose_turn = Silver;
            if (score < best_score)
            {
                best_score = score;
                best_move = valid_moves[i];
            }
            if (score < beta)
                beta = score;
            if (beta <= alpha)
                break;
        }
    }
    return best_move;
}

int alphabeta(int depth, bool maximize, int alpha, int beta)
{
    if (depth == 0)
    {
        return calculate_score();
    }
    if (maximize)
    {
        int max = -9999999;
        whose_turn = Red;
        Move valid_moves[NUM_VALID_MOVES] = {0};
        find_valid_moves(valid_moves);
        for (int i = 0; i < NUM_VALID_MOVES; i++)
        {
            if (valid_moves[i] == 0)
                break;
            make_move(valid_moves[i]);
            int score = alphabeta(depth - 1, false, alpha, beta);
            undo_move();
            whose_turn = Red;
            if (score > max)
                max = score;
            if (score > alpha)
                alpha = score;
            if (beta <= alpha)
                break;
        }
        return max;
    }
    else
    {
        int min = 9999999;
        whose_turn = Silver;
        Move valid_moves[NUM_VALID_MOVES] = {0};
        find_valid_moves(valid_moves);
        for (int i = 0; i < NUM_VALID_MOVES; i++)
        {
            if (valid_moves[i] == 0)
                break;
            make_move(valid_moves[i]);
            int score = alphabeta(depth - 1, true, alpha, beta);
            undo_move();
            whose_turn = Silver;
            if (score < min)
                min = score;
            if (score < beta)
                beta = score;
            if (beta <= alpha)
                break;
        }
        return min;
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
        if (board[i] > 0)
        {
            if (get_owner(board[i]) == Red)
            {
                switch (get_piece(board[i]))
                {
                case Anubis:
                    score += anubis_score;
                    break;
                case Pyramid:
                    score += pyramid_score;
                    break;
                case Scarab:
                    score += scarab_score;
                    break;
                case Pharaoh:
                    score += pharaoh_score;
                    break;
                default:
                    break;
                }
            }
            else
            {
                switch (get_piece(board[i]))
                {
                case Anubis:
                    score -= anubis_score;
                    break;
                case Pyramid:
                    score -= pyramid_score;
                    break;
                case Scarab:
                    score -= scarab_score;
                    break;
                case Pharaoh:
                    score -= pharaoh_score;
                    break;
                default:
                    break;
                }
            }
        }
    }
    return score + (rand() % 100);
}

void make_move(Move move)
{
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

    undo_moves[undo_index] = new_move(end, start, -rotation);
    fire_laser();
    undo_index++;
}

void fire_laser()
{
    int i = sphinx_loc[whose_turn];
    int laser_dir = get_orientation(board[i]);
    bool traversing = true;
    while (traversing)
    {
        i = i + directions[laser_dir];
        if (i >= 0 && i < 120 && on_board[i] == 1)
        {
            if (board[i] > 0)
            {
                int piece = get_piece(board[i]) - 1;
                int orientation = get_orientation(board[i]);
                int result = reflections[laser_dir][piece][orientation];
                if (result == Dead)
                {
                    undo_capture_indices[undo_index] = i;
                    undo_capture_squares[undo_index] = (Square)board[i];
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
}

void find_valid_moves(Move *valid_moves)
{
    int vi = 0;
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
                find_valid_anubis_pyramid_moves(i, valid_moves, &vi);
                break;
            case Pyramid:
                find_valid_anubis_pyramid_moves(i, valid_moves, &vi);
                break;
            case Scarab:
                find_valid_scarab_moves(i, valid_moves, &vi);
                break;
            case Pharaoh:
                find_valid_pharaoh_moves(i, valid_moves, &vi);
                break;
            case Sphinx:
                find_valid_sphinx_moves(i, valid_moves, &vi);
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
            valid_moves[*vi] = new_move(i, dest, 0);
            *vi += 1;
        }
    }
    for (int j = 0; j < 2; j++)
    {
        valid_moves[*vi] = new_move(i, i, rotations[j]);
        *vi += 1;
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
                    valid_moves[*vi] = new_move(i, dest, 0);
                    *vi += 1;
                }
            }
            else
            {
                valid_moves[*vi] = new_move(i, dest, 0);
                *vi += 1;
            }
        }
    }
    for (int j = 0; j < 2; j++)
    {
        valid_moves[*vi] = new_move(i, i, rotations[j]);
        *vi += 1;
    }
}

void find_valid_pharaoh_moves(int i, Move *valid_moves, int *vi)
{
    for (int j = 0; j < 8; j++)
    {
        int dest = i + directions[j];
        if (!is_piece(board[dest]) && can_move[whose_turn][dest])
        {
            valid_moves[*vi] = new_move(i, dest, 0);
            *vi += 1;
        }
    }
}

void find_valid_sphinx_moves(int i, Move *valid_moves, int *vi)
{
    enum Player player = get_owner(board[i]);
    enum Orientation orientation = get_orientation(board[i]);
    int rotation = player == Silver ? (orientation == North ? -1 : 1) : (orientation == South ? -1 : 1);
    valid_moves[*vi] = new_move(i, i, rotation);
    *vi += 1;
}

void setup_board(char *init_board[120])
{
    for (int i = 0; i < 120; i++)
    {
        board[i] = str_to_square(init_board[i]);
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
        if (board[i] > 0)
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
    for (int i = 0; i < 25; i++)
    {
        undo_moves[i] = 0;
        undo_capture_indices[i] = 0;
        undo_capture_squares[i] = 0;
    }
}
