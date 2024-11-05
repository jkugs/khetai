#include "ai_loader.h"

AILoader::AILoader(const std::string &lib_path) : handle(nullptr)
{
    load_library(lib_path);
}

AILoader::~AILoader()
{
    if (handle)
    {
        dlclose(handle);
    }
}

void AILoader::load_library(const std::string &lib_path)
{
    handle = dlopen(lib_path.c_str(), RTLD_LAZY);
    if (!handle)
    {
        throw std::runtime_error("Failed to load KhetAI: " + std::string(dlerror()));
    }
}

void AILoader::reload_library(const std::string &lib_path)
{
    if (handle)
    {
        dlclose(handle);
    }
    load_library(lib_path);
}

void *AILoader::get_symbol(const std::string &symbol_name)
{
    void *symbol = dlsym(handle, symbol_name.c_str());
    check_error();
    return symbol;
}

void AILoader::check_error()
{
    const char *error = dlerror();
    if (error)
    {
        throw std::runtime_error("Error in dynamic loading: " + std::string(error));
    }
}

void (*AILoader::get_init_zobrist())()
{
    return reinterpret_cast<void (*)()>(get_symbol("init_zobrist"));
}

void (*AILoader::get_reset_undo())()
{
    return reinterpret_cast<void (*)()>(get_symbol("reset_undo"));
}

void (*AILoader::get_setup_board())(char **)
{
    return reinterpret_cast<void (*)(char **)>(get_symbol("setup_board"));
}

void (*AILoader::get_print_board())()
{
    return reinterpret_cast<void (*)()>(get_symbol("print_board"));
}

void (*AILoader::get_set_time_parameters())(int, time_t)
{
    return reinterpret_cast<void (*)(int, time_t)>(get_symbol("set_time_parameters"));
}

Move (*AILoader::get_alphabeta_root())(int, enum Player)
{
    return reinterpret_cast<Move (*)(int, enum Player)>(get_symbol("alphabeta_root"));
}

void (*AILoader::get_make_move())(Move)
{
    return reinterpret_cast<void (*)(Move)>(get_symbol("make_move"));
}

int (*AILoader::get_get_start())(Move)
{
    return reinterpret_cast<int (*)(Move)>(get_symbol("get_start_wrapper"));
}

int (*AILoader::get_get_end())(Move)
{
    return reinterpret_cast<int (*)(Move)>(get_symbol("get_end_wrapper"));
}

int (*AILoader::get_get_rotation())(Move)
{
    return reinterpret_cast<int (*)(Move)>(get_symbol("get_rotation_wrapper"));
}