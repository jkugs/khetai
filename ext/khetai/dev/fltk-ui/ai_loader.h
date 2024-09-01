#ifndef AI_LOADER_H
#define AI_LOADER_H

#include <dlfcn.h>
#include <stdexcept>
#include <string>
#include "../../khetai_lib.h"

class AILoader
{
public:
    AILoader(const std::string &lib_path);
    ~AILoader();

    void (*get_init_zobrist())();
    void (*get_setup_board())(char **);
    void (*get_print_board())();
    void (*get_set_time_parameters())(int, time_t);
    Move (*get_alphabeta_root())(int, enum Player);
    void (*get_make_move())(Move);
    int(*get_get_start())(Move);
    int(*get_get_end())(Move);
    int(*get_get_rotation())(Move);
    void reload_library(const std::string &lib_path);

private:
    void *handle;
    void load_library(const std::string& lib_path);
    void *get_symbol(const std::string& symbol_name);
    void check_error();
};

#endif // AI_LOADER_H