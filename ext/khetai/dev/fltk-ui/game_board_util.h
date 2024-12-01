#ifndef GAME_BOARD_UTIL_H
#define GAME_BOARD_UTIL_H

#include "ai_loader.h"

#include <string>
#include <vector>

std::vector<std::string> flatten_2d_vector_with_buffer(const std::vector<std::vector<std::string>> &vec2d);
char **vector_to_c_array(const std::vector<std::string> &vec);
void free_c_array(char **c_array, size_t size);
Move call_ai_move(AILoader &ai_loader, const std::vector<std::vector<std::string>> &board_pieces, Player player, int max_depth, int _max_time);
void get_row_col(int index, int &row, int &col);

#endif