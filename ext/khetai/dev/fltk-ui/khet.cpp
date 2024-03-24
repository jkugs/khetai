#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include "game_board.h"

int main(int argc, char **argv)
{
    Fl_Double_Window *window = new Fl_Double_Window(800, 600, "Khet Game");
    GameBoard *board = new GameBoard(50, 50, 700, 504);

    std::vector<std::vector<std::string>> init_board = {
        {"L2", "--", "--", "--", "A2", "X2", "A2", "P1", "--", "--"},
        {"--", "--", "P2", "--", "--", "--", "--", "--", "--", "--"},
        {"--", "--", "--", "p3", "--", "--", "--", "--", "--", "--"},
        {"P0", "--", "p2", "--", "S2", "S3", "--", "P1", "--", "p3"},
        {"P1", "--", "p3", "--", "s1", "s0", "--", "P0", "--", "p2"},
        {"--", "--", "--", "--", "--", "--", "P1", "--", "--", "--"},
        {"--", "--", "--", "--", "--", "--", "--", "p0", "--", "--"},
        {"--", "--", "p3", "a0", "x0", "a0", "--", "--", "--", "l0"}
    };

    board->init(init_board);

    window->end();
    window->show(argc, argv);
    return Fl::run();
}
