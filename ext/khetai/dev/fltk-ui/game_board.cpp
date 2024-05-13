#include "game_board.h"

#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <iostream>

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
        fl_color(FL_YELLOW);
        fl_rectf((x() + clicked_col * cell_width) + 1, (y() + clicked_row * cell_height) + 1, cell_width - 1, cell_height - 1);
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
            }
        }
    }
}

int GameBoard::handle(int event)
{
    // std::cout << "EVENT: " event << std::endl;
    switch (event)
    {
    case FL_PUSH:
    {
        int clicked_x = Fl::event_x() - x();
        int clicked_y = Fl::event_y() - y();

        clicked_col = clicked_x / cell_width;
        clicked_row = clicked_y / cell_height;

        int clicked_num = (clicked_row * cols) + clicked_col;

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
    case FL_KEYUP:
    {
        int key = Fl::event_key();
        if (square_selected)
        {
            switch (key)
            {
            case FL_Left:
            {
                rotateSelectedPiece(false);
                break;
            }
            case FL_Right:
            {
                rotateSelectedPiece(true);
                break;
            }
            }
        }

        redraw();
        return 1;
    }
    default:
        break;
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
                int direction = piece[1] - '0';
                std::string filename;

                filename = getPieceFilename(piece_type, direction);

                Fl_PNG_Image *orig_image = new Fl_PNG_Image(filename.c_str());
                Fl_Image *resized_image = orig_image->copy(cell_width, cell_height);
                delete orig_image;
                piece_images.push_back(resized_image);
            }
            else
            {
                piece_images.push_back(nullptr);
            }
        }
    }
}

std::string GameBoard::getPieceFilename(char piece, int direction)
{
    static const std::string directions[] = {"_n.png", "_e.png", "_s.png", "_w.png"};

    if (piece == 'X' || piece == 'x')
    {
        return piece_map[piece] + ".png";
    }

    return piece_map[piece] + directions[direction];
}

void GameBoard::rotateSelectedPiece(bool clockwise)
{
    if (square_selected_num == -1)
        return;

    delete piece_images[square_selected_num];
    piece_images[square_selected_num] = nullptr;

    int row = square_selected_num / cols;
    int col = square_selected_num % cols;

    std::string current_piece = board_pieces[row][col];
    char piece_type = current_piece[0];
    int current_direction = std::stoi(current_piece.substr(1));

    int new_direction;
    std::string new_piece;

    if (clockwise)
    {
        new_direction = rotate_right_map[current_direction];
    }
    else
    {
        new_direction = rotate_left_map[current_direction];
    }

    new_piece = piece_type + std::to_string(new_direction);
    board_pieces[row][col] = new_piece;

    std::string filename;
    filename = getPieceFilename(piece_type, new_direction);
    Fl_PNG_Image *orig_image = new Fl_PNG_Image(filename.c_str());
    Fl_Image *resized_image = orig_image->copy(cell_width, cell_height);
    delete orig_image;
    piece_images[square_selected_num] = resized_image;
}

std::unordered_map<char, std::string> GameBoard::piece_map = {
    {'L', "assets/laser_red"},
    {'A', "assets/anubis_red"},
    {'X', "assets/pharaoh_red"},
    {'P', "assets/pyramid_red"},
    {'S', "assets/scarab_red"},
    {'l', "assets/laser_silver"},
    {'a', "assets/anubis_silver"},
    {'x', "assets/pharaoh_silver"},
    {'p', "assets/pyramid_silver"},
    {'s', "assets/scarab_silver"}};

std::unordered_map<int, int> GameBoard::rotate_left_map = {
    {0, 3}, {3, 2}, {2, 1}, {1, 0}};

std::unordered_map<int, int> GameBoard::rotate_right_map = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}};