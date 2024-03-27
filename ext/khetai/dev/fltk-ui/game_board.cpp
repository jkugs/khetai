#include "game_board.h"
#include "image_util.h"

#include <FL/fl_draw.H>
#include <FL/Fl.H>

GameBoard::GameBoard(int X, int Y, int W, int H, const char *L) : Fl_Widget(X, Y, W, H, L)
{
    rows = 8;
    cols = 10;

    cell_width = w() / cols;
    cell_height = h() / rows;

    square_selected = false;
    square_selected_num = -1;

    clicked_col = -1;
    clicked_row = -1;
}

void GameBoard::draw()
{
    // background
    fl_color(FL_WHITE);
    fl_rectf(x(), y(), w(), h());

    // grid lines
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

    // highlight selected square
    if (square_selected)
    {
        fl_color(FL_BLUE);
        fl_rectf(x() + clicked_col * cell_width, y() + clicked_row * cell_height, cell_width, cell_height);
    }

    // draw pieces
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            int piece_index = i * cols + j;
            if (piece_images[piece_index] != nullptr)
            {
                piece_images[piece_index]->draw(x() + j * cell_width, y() + i * cell_height, cell_width, cell_height);

                // debugging image drawing                
                /*fl_color(FL_YELLOW); // Highlight color
                fl_rectf(x() + j * cell_width, y() + i * cell_height, cell_width, cell_height); // Fill the cell
                piece_images[piece_index]->draw(x() + j * cell_width, y() + i * cell_height, cell_width, cell_height);
                fl_color(FL_RED); // Border color for debugging
                fl_rect(x() + j * cell_width, y() + i * cell_height, cell_width, cell_height); // Draw the cell border*/
            }
        }
    }
}

int GameBoard::handle(int event)
{
    if (event == FL_PUSH)
    {
        int clicked_x = Fl::event_x() - x();
        int clicked_y = Fl::event_y() - y();

        clicked_col = clicked_x / cell_width;
        clicked_row = clicked_y / cell_height;

        int clicked_num = (clicked_row * rows) + clicked_col;

        if (square_selected && square_selected_num == clicked_num)
        {
            square_selected = false;
            square_selected_num = -1;
        }
        else
        {
            square_selected = true;
            square_selected_num = clicked_num;
        }

        redraw();
        return 1;
    }

    return Fl_Widget::handle(event);
}

void GameBoard::init(const std::vector<std::vector<std::string>> &pieces)
{
    board_pieces = pieces;

    for (const std::vector<std::string> &row : board_pieces)
    {
        for (const std::string &piece : row)
        {
            if (piece != "--")
            {
                char piece_type = piece[0];
                std::string filename;

                auto it = GameBoard::piece_map.find(piece_type);
                filename = it->second;

                Fl_PNG_Image *orig_image = new Fl_PNG_Image(filename.c_str());
                Fl_Image *resized_image = orig_image->copy(cell_width, cell_height);
                delete orig_image;
                // piece_images.push_back(resized_image);

                // temp rotate test
                Fl_Image *rotated = rotate_image_90_clockwise((Fl_RGB_Image *)resized_image);
                piece_images.push_back(rotated);
            }
            else
            {
                piece_images.push_back(nullptr);
            }
        }
    }
}

std::unordered_map<char, std::string> GameBoard::piece_map = {
    {'L', "assets/laser_red.png"},
    {'A', "assets/anubis_red.png"},
    {'X', "assets/pharaoh_red.png"},
    {'P', "assets/pyramid_red.png"},
    {'S', "assets/scarab_red.png"},
    {'l', "assets/laser_silver.png"},
    {'a', "assets/anubis_silver.png"},
    {'x', "assets/pharaoh_silver.png"},
    {'p', "assets/pyramid_silver.png"},
    {'s', "assets/scarab_silver.png"}};
