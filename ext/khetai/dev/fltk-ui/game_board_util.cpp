#include "game_board_util.h"

#include <iostream>
#include <cstring>

time_t start_time;
int max_time;

std::vector<std::string> flatten_2d_vector_with_buffer(const std::vector<std::vector<std::string>> &vec2d)
{
    size_t new_rows = vec2d.size() + 2;
    size_t new_cols = vec2d.empty() ? 0 : vec2d[0].size() + 2;

    std::vector<std::vector<std::string>> buffered_vec2d(new_rows, std::vector<std::string>(new_cols, "--"));

    for (size_t i = 0; i < vec2d.size(); ++i)
    {
        for (size_t j = 0; j < vec2d[i].size(); ++j)
        {
            buffered_vec2d[i + 1][j + 1] = vec2d[i][j];
        }
    }

    std::vector<std::string> flattened;
    for (const auto &row : buffered_vec2d)
    {
        for (const auto &elem : row)
        {
            flattened.push_back(elem);
        }
    }

    return flattened;
}

char **vector_to_c_array(const std::vector<std::string> &vec)
{
    char **c_array = new char *[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i)
    {
        c_array[i] = new char[vec[i].size() + 1];
        std::strcpy(c_array[i], vec[i].c_str());
    }

    return c_array;
}

void free_c_array(char **c_array, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        delete[] c_array[i];
    }
    delete[] c_array;
}

Move call_ai_move(const std::vector<std::vector<std::string>> &board_pieces, Player player, int max_depth, int _max_time)
{
    char **c_board = vector_to_c_array(flatten_2d_vector_with_buffer(board_pieces));

    init_zobrist();
    srand((unsigned)time(NULL));

    setup_board(c_board);
    print_board();

    start_time = time(NULL);
    max_time = _max_time;

    int depth = 1;
    Move best_move = (Move)0;
    Move current_move = (Move)0;
    while ((time(NULL) - start_time < max_time) && (depth <= max_depth))
    {
        best_move = current_move;
        current_move = alphabeta_root(depth, player);
        depth++;
    }
    make_move(best_move);

    printf("\n========================\n\n");
    print_board();

    printf("\nDEPTH: %d\n", depth - 1);

    int start = get_start(best_move);
    int end = get_end(best_move);
    int rotation = get_rotation(best_move);
    std::cout << "START: " << start << ", END: " << end << ", ROTATION: " << rotation << std::endl;
    return best_move;
}

void get_row_col(int index, int &row, int &col)
{
    int border_width = 1;
    int width_with_border = 10 + 2 * border_width;

    int adjusted_index = index;

    int row_with_border = adjusted_index / width_with_border;
    int col_with_border = adjusted_index % width_with_border;

    row = row_with_border - border_width;
    col = col_with_border - border_width;
}