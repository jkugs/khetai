#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include "ai_loader.h"

#include <FL/Fl_Widget.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Input.H>
#include <string>
#include <vector>
#include <unordered_map>

class GameBoard : public Fl_Widget
{
public:
    GameBoard(int X, int Y, int W, int H, const char *L = 0);
    Fl_Input *max_time_input, *max_depth_input;
    void draw() override;
    int handle(int event) override;
    void init(const std::vector<std::vector<std::string>> &pieces);

    enum Color
    {
        SILVER = 1,
        RED = 2
    };
    enum LaserDirection
    {
        NORTH = 1,
        EAST = 2,
        SOUTH = 3,
        WEST = 4
    };
    enum PieceType
    {
        ANUBIS,
        PYRAMID,
        SCARAB,
        PHARAOH,
        SPHINX
    };
    enum PieceOrientation
    {
        ORIENT_NORTH,
        ORIENT_EAST,
        ORIENT_SOUTH,
        ORIENT_WEST
    };
    enum ReflectionResult
    {
        RESULT_DEAD,
        RESULT_ABSORBED,
        RESULT_EAST,
        RESULT_WEST,
        RESULT_SOUTH,
        RESULT_NORTH
    };
    enum MovePermission
    {
        S = 1,
        R = 2,
        B = 3
    };

    static std::string getPieceFilename(char piece, int direction);
    static std::pair<PieceType, PieceOrientation> getPieceTypeAndOrientation(const std::string &piece_str);
    static void laser_timer_cb(void *data);

private:
    AILoader ai_loader;
    int rows = 8, cols = 10, clicked_row = -1, clicked_col = -1, square_selected_num = -1;
    int cell_width, cell_height;
    bool square_selected = false;
    bool laser_active = false;
    int laser_step = 3;
    LaserDirection laser_direction;
    float laser_y, laser_x;
    bool remove_piece = false;
    int laser_square_row, laser_square_col, remove_row, remove_col;
    int l_idx;
    std::vector<std::vector<std::string>> board_pieces;
    std::vector<Fl_Image *> piece_images;
    std::vector<std::tuple<int, int, int, int>> laser_path;
    std::vector<std::tuple<int, int, int, int>> laser_path_squares;

    static std::unordered_map<LaserDirection, std::unordered_map<PieceType, std::unordered_map<PieceOrientation, ReflectionResult>>> reflections_map;
    static std::unordered_map<char, std::string> piece_map;
    static std::unordered_map<int, int> rotate_left_map;
    static std::unordered_map<int, int> rotate_right_map;
    static const MovePermission move_permissions[8][10];

    void drawInnerSquares();
    void rotateSelectedPiece(bool clockwise);
    void moveSelectedPiece(int end_square);
    void swapPieces(int swap_square);
    void deletePiece();
    void resetPieces();
    void fireLaser(Color color);
    void updateLaserPosition();
    void calculateLaserPathSquares();
    bool squareContainsPiece(int square_num);
};

#endif