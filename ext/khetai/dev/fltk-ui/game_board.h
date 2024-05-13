#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <FL/Fl_Widget.H>
#include <FL/Fl_PNG_Image.H>
#include <string>
#include <vector>
#include <unordered_map>

class GameBoard : public Fl_Widget
{
public:
    GameBoard(int X, int Y, int W, int H, const char *L = 0);
    void draw() override;
    int handle(int event) override;
    void init(const std::vector<std::vector<std::string>>& pieces);

    static std::string getPieceFilename(char piece, int direction);

private:
    int rows, cols, cell_width, cell_height, clicked_row, clicked_col;
    bool square_selected;
    int square_selected_num;
    std::vector<std::vector<std::string>> board_pieces;
    std::vector<Fl_Image*> piece_images;

    static std::unordered_map<char, std::string> piece_map;
    static std::unordered_map<int, int> rotate_left_map;
    static std::unordered_map<int, int> rotate_right_map;

    void rotateSelectedPiece(bool clockwise);
};

#endif