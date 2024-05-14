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
    void init(const std::vector<std::vector<std::string>> &pieces);

    static std::string getPieceFilename(char piece, int direction);
    static void laser_timer_cb(void *data);

private:
    int rows = 8, cols = 10, clicked_row = -1, clicked_col = -1, square_selected_num = -1;
    int cell_width, cell_height;
    bool square_selected = false;
    bool laser_active = false;
    int laser_step = 2, laser_direction;
    float laser_y, laser_x;
    std::vector<std::vector<std::string>> board_pieces;
    std::vector<Fl_Image *> piece_images;
    std::vector<std::tuple<int, int, int, int>> laser_path;

    static std::unordered_map<char, std::string> piece_map;
    static std::unordered_map<int, int> rotate_left_map;
    static std::unordered_map<int, int> rotate_right_map;

    void rotateSelectedPiece(bool clockwise);
    void fireLaser();
    void updateLaserPosition();
};

#endif