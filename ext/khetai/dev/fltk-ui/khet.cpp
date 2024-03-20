#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

// Custom widget to represent the game board
class GameBoard : public Fl_Widget
{
public:
    GameBoard(int X, int Y, int W, int H, const char *L = 0) : Fl_Widget(X, Y, W, H, L) {}

    void draw() override
    {
        // Background
        fl_color(FL_WHITE);
        fl_rectf(x(), y(), w(), h());

        int rows = 8;
        int cols = 10;

        int cell_width = w() / cols;
        int cell_height = h() / rows;

        // Grid lines
        fl_color(FL_BLACK);
        
        // vertical lines
        for (int i = 0; i <= cols; ++i)
        {
            int current_x = x() + (cell_width * i);
            fl_line(current_x, y(), current_x, y() + h());
        }

        // horizontal lines
        for (int j = 0; j <= rows; ++j)
        {
            int current_y = y() + (cell_height * j);
            fl_line(x(), current_y, x() + w(), current_y);
        }
    }

    int handle(int event) override
    {
        
    }
};

int main(int argc, char **argv)
{
    Fl_Window *window = new Fl_Window(800, 600, "Khet Game");
    GameBoard *board = new GameBoard(50, 50, 700, 504);
    window->end();
    window->show(argc, argv);
    return Fl::run();
}
