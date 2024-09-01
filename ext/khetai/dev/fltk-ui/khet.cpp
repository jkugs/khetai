#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Box.H>
#include "game_board.h"

class PositiveIntInput : public Fl_Int_Input
{
public:
    PositiveIntInput(int X, int Y, int W, int H, const char *L = 0)
        : Fl_Int_Input(X, Y, W, H, L) {}

    int handle(int event) override
    {
        int result = Fl_Int_Input::handle(event);
        if (event == FL_KEYDOWN || event == FL_KEYUP || event == FL_PASTE)
        {
            const char *value = this->value();
            if (value[0] == '-')
            {
                this->value("");
            }
        }
        return result;
    }
};

int main(int argc, char **argv)
{
    Fl_Double_Window *window = new Fl_Double_Window(800, 600, "Khet AI");
    GameBoard *board = new GameBoard(50, 50, 700, 504);

    std::vector<std::vector<std::string>> init_board = {
        {"L2", "--", "--", "--", "A2", "X2", "A2", "P1", "--", "--"},
        {"--", "--", "P2", "--", "--", "--", "--", "--", "--", "--"},
        {"--", "--", "--", "p3", "--", "--", "--", "--", "--", "--"},
        {"P0", "--", "p2", "--", "S2", "S3", "--", "P1", "--", "p3"},
        {"P1", "--", "p3", "--", "s1", "s0", "--", "P0", "--", "p2"},
        {"--", "--", "--", "--", "--", "--", "P1", "--", "--", "--"},
        {"--", "--", "--", "--", "--", "--", "--", "p0", "--", "--"},
        {"--", "--", "p3", "a0", "x0", "a0", "--", "--", "--", "l0"}};

    board->init(init_board);

    Fl_Box *max_time_label = new Fl_Box(50, 10, 80, 30, "Max Time:");
    PositiveIntInput *max_time_input = new PositiveIntInput(125, 15, 30, 20);
    max_time_input->value("5");
    board->max_time_input = max_time_input;

    Fl_Box *max_depth_label = new Fl_Box(250, 10, 80, 30, "Max Depth:");
    PositiveIntInput *max_depth_input = new PositiveIntInput(325, 15, 30, 20);
    max_depth_input->value("10");
    board->max_depth_input = max_depth_input;

    window->end();
    window->show(argc, argv);
    return Fl::run();
}
