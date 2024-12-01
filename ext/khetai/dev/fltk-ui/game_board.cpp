#include "game_board.h"
#include "../../khetai_lib.h"
#include "game_board_util.h"

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <cstring>
#include <iostream>

GameBoard::GameBoard(int X, int Y, int W, int H, const char *L)
    : Fl_Widget(X, Y, W, H, L),
      ai_loader("./libkhetai.so") {
    cell_width = w() / cols;
    cell_height = h() / rows;
}

void GameBoard::draw() {
    // background
    fl_color(FL_WHITE);
    fl_rectf(x(), y(), w(), h());

    // grid lines
    fl_color(FL_BLACK);

    // vertical lines
    for (int i = 0; i <= cols; ++i) {
        int current_x = x() + (cell_width * i);
        fl_line(current_x, y(), current_x, y() + h());
    }

    // horizontal lines
    for (int j = 0; j <= rows; ++j) {
        int current_y = y() + (cell_height * j);
        fl_line(x(), current_y, x() + w(), current_y);
    }

    // label spaces that only certain colors can move onto
    drawInnerSquares();

    // highlight selected square
    if (square_selected) {
        fl_color(FL_YELLOW);
        fl_rectf((x() + clicked_col * cell_width) + 1, (y() + clicked_row * cell_height) + 1, cell_width - 1, cell_height - 1);
    }

    // draw laser
    if (laser_active) {
        fl_color(FL_RED);
        for (auto &segment : laser_path) {
            fl_line(std::get<0>(segment), std::get<1>(segment), std::get<2>(segment), std::get<3>(segment));
        }
    }

    // draw pieces
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int piece_index = i * cols + j;
            if (piece_images[piece_index] != nullptr) {
                piece_images[piece_index]->draw(x() + j * cell_width, y() + i * cell_height, cell_width, cell_height);
            }
        }
    }
}

void GameBoard::drawInnerSquares() {
    static const Fl_Color SILVER = fl_rgb_color(160, 160, 160);
    static const Fl_Color DARK_RED = fl_rgb_color(178, 34, 34);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int padding_x = cell_width / 6;
            int padding_y = cell_height / 6;
            int square_width = cell_width - (2 * padding_x);
            int square_height = cell_height - (2 * padding_y);

            switch (move_permissions[row][col]) {
            case S:
                fl_color(SILVER);
                fl_line_style(FL_SOLID, 1);
                fl_rect(x() + col * cell_width + padding_x,
                        y() + row * cell_height + padding_y,
                        square_width,
                        square_height);
                break;
            case R:
                fl_color(DARK_RED);
                fl_line_style(FL_SOLID, 1);
                fl_rect(x() + col * cell_width + padding_x,
                        y() + row * cell_height + padding_y,
                        square_width,
                        square_height);
                break;
            case B:
                break;
            }
        }
    }
    fl_line_style(0);
}

int GameBoard::handle(int event) {
    // std::cout << "EVENT: " event << std::endl;
    switch (event) {
    case FL_PUSH: {
        int clicked_x = Fl::event_x() - x();
        int clicked_y = Fl::event_y() - y();

        clicked_col = clicked_x / cell_width;
        clicked_row = clicked_y / cell_height;

        int clicked_num = (clicked_row * cols) + clicked_col;

        if (square_selected && square_selected_num == clicked_num) {
            square_selected = false;
            square_selected_num = -1;
        } else if (square_selected && square_selected_num != clicked_num &&
                   squareContainsPiece(square_selected_num) && clicked_num != 0 && clicked_num != 79) {
            if (squareContainsPiece(clicked_num)) {
                swapPieces(clicked_num);
            } else {
                moveSelectedPiece(clicked_num);
            }
        } else if (squareContainsPiece(clicked_num)) {
            square_selected = true;
            square_selected_num = clicked_num;
        }

        redraw();
        return 1;
    }
    case FL_KEYUP: {
        int key = Fl::event_key();
        int state = Fl::event_state();

        if (key == 'r') {
            resetPieces();
        } else if (key == FL_Enter) {
            const char *max_time_value = max_time_input->value();
            if (!max_time_value || max_time_value[0] == '\0') {
                max_time_input->value("5");
                max_time_value = max_time_input->value();
            }
            int max_time = std::atoi(max_time_value);

            const char *max_depth_value = max_depth_input->value();
            if (!max_depth_value || max_depth_value[0] == '\0') {
                max_depth_input->value("25");
                max_depth_value = max_depth_input->value();
            }
            int max_depth = std::atoi(max_depth_value);
            if (max_depth < 2) {
                max_depth_input->value("2");
                max_depth = 2;
            }

            Move move = call_ai_move(ai_loader, board_pieces, Player::RED, max_depth, max_time);
            int start = get_start(move);
            int end = get_end(move);
            int rotation = get_rotation(move);

            int start_row, start_col, end_row, end_col;
            get_row_col(start, start_row, start_col);
            get_row_col(end, end_row, end_col);

            int start_num = (start_row * cols) + start_col;
            int end_num = (end_row * cols) + end_col;

            square_selected_num = start_num;

            if (board_pieces[end_row][end_col] == "--") {
                moveSelectedPiece(end_num);
            } else if (start_num != end_num && board_pieces[end_row][end_col] != "--") {
                swapPieces(end_num);
            } else if (start_num == end_num && rotation != 0) {
                bool clockwise = rotation == 1;
                rotateSelectedPiece(clockwise);
            }

            fireLaser(RED);
        } else if (key == 'k' && (state & FL_SHIFT)) {
            rebuildReloadKhetAILib();
            return 1;
        } else if (square_selected) {
            switch (key) {
            case FL_Left:
                rotateSelectedPiece(false);
                break;
            case FL_Right:
                rotateSelectedPiece(true);
                break;
            case FL_Delete:
                deletePiece();
                break;
            case ' ':
                if (square_selected_num == 0) {
                    fireLaser(RED);
                } else if (square_selected_num == 79) {
                    fireLaser(SILVER);
                }
                square_selected = false;
                square_selected_num = -1;
                break;
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

void GameBoard::init(const std::vector<std::vector<std::string>> &pieces) {
    board_pieces = pieces;

    for (const std::vector<std::string> &row : board_pieces) {
        for (const std::string &piece : row) {
            if (piece != "--") {
                char piece_type = piece[0];
                int direction = piece[1] - '0';
                std::string filename;

                filename = getPieceFilename(piece_type, direction);

                Fl_PNG_Image *orig_image = new Fl_PNG_Image(filename.c_str());
                Fl_Image *resized_image = orig_image->copy(cell_width, cell_height);
                delete orig_image;
                piece_images.push_back(resized_image);
            } else {
                piece_images.push_back(nullptr);
            }
        }
    }
}

void GameBoard::resetPieces() {
    for (Fl_Image *image : piece_images) {
        delete image;
    }
    piece_images.clear();

    square_selected = false;
    square_selected_num = -1;

    std::vector<std::vector<std::string>> init_board = {
        {"L2", "--", "--", "--", "A2", "X2", "A2", "P1", "--", "--"},
        {"--", "--", "P2", "--", "--", "--", "--", "--", "--", "--"},
        {"--", "--", "--", "p3", "--", "--", "--", "--", "--", "--"},
        {"P0", "--", "p2", "--", "S2", "S3", "--", "P1", "--", "p3"},
        {"P1", "--", "p3", "--", "s1", "s0", "--", "P0", "--", "p2"},
        {"--", "--", "--", "--", "--", "--", "P1", "--", "--", "--"},
        {"--", "--", "--", "--", "--", "--", "--", "p0", "--", "--"},
        {"--", "--", "p3", "a0", "x0", "a0", "--", "--", "--", "l0"}};

    init(init_board);
}

void GameBoard::deletePiece() {
    if (square_selected_num == -1)
        return;

    if (square_selected_num == 0 || square_selected_num == 79) {
        square_selected = false;
        square_selected_num = -1;
        return;
    }

    delete piece_images[square_selected_num];
    piece_images[square_selected_num] = nullptr;

    int row = square_selected_num / cols;
    int col = square_selected_num % cols;

    board_pieces[row][col] = "--";

    square_selected = false;
    square_selected_num = -1;
}

std::string GameBoard::getPieceFilename(char piece, int direction) {
    static const std::string directions[] = {"_n.png", "_e.png", "_s.png", "_w.png"};

    if (piece == 'X' || piece == 'x') {
        return piece_map[piece] + ".png";
    }

    return piece_map[piece] + directions[direction];
}

void GameBoard::rotateSelectedPiece(bool clockwise) {
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

    if (clockwise) {
        new_direction = rotate_right_map[current_direction];
    } else {
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

void GameBoard::swapPieces(int swap_square) {
    if (square_selected_num == -1)
        return;

    if (square_selected_num == 0 || square_selected_num == 79 ||
        swap_square == 0 || swap_square == 79) {
        square_selected = true;
        square_selected_num = swap_square;
        return;
    }

    delete piece_images[square_selected_num];
    piece_images[square_selected_num] = nullptr;

    delete piece_images[swap_square];
    piece_images[swap_square] = nullptr;

    int first_row = square_selected_num / cols;
    int first_col = square_selected_num % cols;

    int second_row = swap_square / cols;
    int second_col = swap_square % cols;

    auto first = board_pieces[first_row][first_col];
    auto second = board_pieces[second_row][second_col];

    board_pieces[first_row][first_col] = second;
    board_pieces[second_row][second_col] = first;

    std::string piece_one = board_pieces[first_row][first_col];
    char piece_one_type = piece_one[0];
    int piece_one_direction = std::stoi(piece_one.substr(1));

    std::string filename_one;
    filename_one = getPieceFilename(piece_one_type, piece_one_direction);
    Fl_PNG_Image *orig_image_one = new Fl_PNG_Image(filename_one.c_str());
    Fl_Image *resized_image_one = orig_image_one->copy(cell_width, cell_height);
    delete orig_image_one;
    piece_images[square_selected_num] = resized_image_one;

    std::string piece_two = board_pieces[second_row][second_col];
    char piece_two_type = piece_two[0];
    int piece_two_direction = std::stoi(piece_two.substr(1));

    std::string filename_two;
    filename_two = getPieceFilename(piece_two_type, piece_two_direction);
    Fl_PNG_Image *orig_image_two = new Fl_PNG_Image(filename_two.c_str());
    Fl_Image *resized_image_two = orig_image_two->copy(cell_width, cell_height);
    delete orig_image_two;
    piece_images[swap_square] = resized_image_two;

    square_selected = false;
    square_selected_num = -1;
}

void GameBoard::moveSelectedPiece(int end_square) {
    if (square_selected_num == -1)
        return;

    if (square_selected_num == 0 || square_selected_num == 79) {
        square_selected = false;
        square_selected_num = -1;
        return;
    }

    delete piece_images[square_selected_num];
    piece_images[square_selected_num] = nullptr;

    int start_row = square_selected_num / cols;
    int start_col = square_selected_num % cols;

    int end_row = end_square / cols;
    int end_col = end_square % cols;

    board_pieces[end_row][end_col] = board_pieces[start_row][start_col];
    board_pieces[start_row][start_col] = "--";

    std::string current_piece = board_pieces[end_row][end_col];
    char piece_type = current_piece[0];
    int piece_direction = std::stoi(current_piece.substr(1));

    std::string filename;
    filename = getPieceFilename(piece_type, piece_direction);
    Fl_PNG_Image *orig_image = new Fl_PNG_Image(filename.c_str());
    Fl_Image *resized_image = orig_image->copy(cell_width, cell_height);
    delete orig_image;
    piece_images[end_square] = resized_image;

    square_selected = false;
    square_selected_num = -1;
}

void GameBoard::fireLaser(Color color) {
    laser_active = true;
    laser_path.clear();

    int start_x;
    int start_y;
    int end_x;
    int end_y;

    if (color == RED) {
        std::string piece_str = board_pieces[0][0];
        auto [piece_type, piece_orientation] = getPieceTypeAndOrientation(piece_str);

        start_x = x() + (cell_width / 2);
        start_y = y() + (cell_height / 2);

        if (piece_orientation == ORIENT_EAST) {

            end_x = start_x + laser_step;
            end_y = start_y;
            laser_direction = EAST;
            laser_square_row = 0;
            laser_square_col = 0;
        } else if (piece_orientation == ORIENT_SOUTH) {

            end_x = start_x;
            end_y = start_y + laser_step;
            laser_direction = SOUTH;
            laser_square_row = 0;
            laser_square_col = 0;
        } else {
            laser_active = false;
            return;
        }
    } else if (color == SILVER) {
        int current_row = rows - 1;
        int current_col = cols - 1;
        std::string piece_str = board_pieces[current_row][current_col];
        auto [piece_type, piece_orientation] = getPieceTypeAndOrientation(piece_str);

        if (piece_orientation == ORIENT_WEST) {
            start_x = x() + (current_col * cell_width) + (cell_width / 2);
            start_y = y() + (current_row * cell_height) + (cell_height / 2);
            end_x = start_x - laser_step;
            end_y = start_y;
            laser_direction = WEST;
            laser_square_row = current_row;
            laser_square_col = current_col;
        } else if (piece_orientation == ORIENT_NORTH) {
            start_x = x() + (current_col * cell_width) + (cell_width / 2);
            start_y = y() + (current_row * cell_height) + (cell_height / 2);
            end_x = start_x;
            end_y = start_y - laser_step;
            laser_direction = NORTH;
            laser_square_row = current_row;
            laser_square_col = current_col;
        } else {
            laser_active = false;
            return;
        }
    }

    calculateLaserPathSquares();
    // print path:
    std::cout << "laser_path_squares = [";
    for (size_t i = 0; i < laser_path_squares.size(); ++i) {
        std::apply([](auto &&...args) {
                       std::cout << '(';
                       ((std::cout << args << ", "), ...);
                       std::cout << '\b' << '\b' << ')'; },
                   laser_path_squares[i]);
        if (i != laser_path_squares.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    l_idx = 0;

    // add the segment to the path
    laser_path.push_back(std::make_tuple(start_x, start_y, end_x, end_y));
    laser_y = end_y;
    laser_x = end_x;

    redraw();
    Fl::add_timeout(0.01, laser_timer_cb, this);
}

void GameBoard::updateLaserPosition() {
    if (laser_y >= y() + (rows * cell_height) || laser_y <= y() || laser_x >= x() + (cols * cell_width) || laser_x <= x()) {
        laser_active = false;
        laser_path.clear();
        redraw();
        return;
    }

    std::tuple<int, int, int, int> current_segment = laser_path_squares[l_idx];
    int goal_row = std::get<2>(current_segment);
    int goal_col = std::get<3>(current_segment);

    int goal_x = x() + (goal_col * cell_width) + (cell_width / 2);
    int goal_y = y() + (goal_row * cell_height) + (cell_height / 2);

    // if the laser steps beyond the middle of the square,
    // set to exactly the middle
    if (((laser_direction == NORTH && laser_y < goal_y) ||
         (laser_direction == SOUTH && laser_y > goal_y) ||
         (laser_direction == EAST && laser_x > goal_x) ||
         (laser_direction == WEST && laser_x < goal_x)) &&
        (l_idx < laser_path_squares.size())) {
        laser_x = goal_x;
        laser_y = goal_y;
    }

    // determine next direction if we are at the middle of a square
    if (laser_x == goal_x && laser_y == goal_y) {
        if (l_idx >= laser_path_squares.size() - 1) {
            laser_active = false;
            laser_path.clear();
            redraw();
            return;
        }

        l_idx++;
        current_segment = laser_path_squares[l_idx];
        auto [cur_row, cur_col, end_row, end_col] = current_segment;

        if (cur_row == end_row) {
            laser_direction = (cur_col < end_col) ? EAST : WEST;
        } else {
            laser_direction = (cur_row < end_row) ? SOUTH : NORTH;
        }
    }

    switch (laser_direction) {
    case NORTH:
        laser_y -= laser_step;
        break;
    case SOUTH:
        laser_y += laser_step;
        break;
    case EAST:
        laser_x += laser_step;
        break;
    case WEST:
        laser_x -= laser_step;
        break;
    }

    auto last_segment = laser_path.back();
    int start_x = std::get<2>(last_segment);
    int start_y = std::get<3>(last_segment);
    int end_x = laser_x;
    int end_y = laser_y;

    laser_path.push_back(std::make_tuple(start_x, start_y, end_x, end_y));
    redraw();
}

void GameBoard::laser_timer_cb(void *data) {
    GameBoard *gb = static_cast<GameBoard *>(data);
    if (gb->laser_active) {
        gb->updateLaserPosition();
        Fl::repeat_timeout(0.01, laser_timer_cb, data);
    }
}

void GameBoard::calculateLaserPathSquares() {
    laser_path_squares.clear();

    int current_row = laser_square_row;
    int current_col = laser_square_col;
    LaserDirection direction = laser_direction;

    bool calculating = true;
    while (calculating) {
        int start_row = current_row;
        int start_col = current_col;
        int end_row = current_row;
        int end_col = current_col;

        switch (direction) {
        case NORTH:
            end_row -= 1;
            break;
        case EAST:
            end_col += 1;
            break;
        case SOUTH:
            end_row += 1;
            break;
        case WEST:
            end_col -= 1;
            break;
        }

        // out of bounds?
        if (end_row < 0 || end_row >= rows || end_col < 0 || end_col >= cols) {
            break;
        }

        laser_path_squares.push_back(std::make_tuple(start_row, start_col, end_row, end_col));

        // determine next direction...
        current_row = end_row;
        current_col = end_col;

        std::string piece_str = board_pieces[current_row][current_col];

        if (piece_str == "--") {
            continue;
        }

        // next square contains a piece... determine what happens next...
        auto [piece_type, piece_orientation] = getPieceTypeAndOrientation(piece_str);
        auto reflection_result = reflections_map[direction][piece_type][piece_orientation];

        switch (reflection_result) {
        case RESULT_ABSORBED:
            calculating = false;
            break;
        case RESULT_DEAD:
            remove_piece = true;
            remove_row = current_row;
            remove_col = current_col;
            calculating = false;
            break;
        case RESULT_EAST:
            direction = EAST;
            break;
        case RESULT_WEST:
            direction = WEST;
            break;
        case RESULT_SOUTH:
            direction = SOUTH;
            break;
        case RESULT_NORTH:
            direction = NORTH;
            break;
        }
    }
}

std::pair<GameBoard::PieceType, GameBoard::PieceOrientation> GameBoard::getPieceTypeAndOrientation(const std::string &piece_str) {

    char piece_char = std::toupper(piece_str[0]);
    char orient_char = piece_str[1];

    PieceType piece_type;
    PieceOrientation piece_orientation;

    switch (piece_char) {
    case 'P':
        piece_type = PYRAMID;
        break;
    case 'A':
        piece_type = ANUBIS;
        break;
    case 'S':
        piece_type = SCARAB;
        break;
    case 'X':
        piece_type = PHARAOH;
        break;
    case 'L':
        piece_type = SPHINX;
        break;
    default:
        throw std::invalid_argument("Invalid piece type");
    }

    switch (orient_char) {
    case '0':
        piece_orientation = ORIENT_NORTH;
        break;
    case '1':
        piece_orientation = ORIENT_EAST;
        break;
    case '2':
        piece_orientation = ORIENT_SOUTH;
        break;
    case '3':
        piece_orientation = ORIENT_WEST;
        break;
    default:
        throw std::invalid_argument("Invalid piece orientation");
    }

    return std::make_pair(piece_type, piece_orientation);
}

bool GameBoard::squareContainsPiece(int square_num) {
    int row = square_num / cols;
    int col = square_num % cols;
    return board_pieces[row][col] != "--";
}

void GameBoard::rebuildReloadKhetAILib() {
    std::cout << "\nRebuilding and reloading KhetAI lib..." << std::endl;

    int build_result = system("./build_khetai.sh");
    if (build_result != 0) {
        std::cerr << "Failed to rebuild KhetAI lib" << std::endl;
        return;
    }

    try {
        ai_loader.reload_library("./libkhetai.so");
        std::cout << "KhetAI lib reloaded successfully" << std::endl;
    } catch (const std::runtime_error &e) {
        std::cerr << "Failed to reload KhetAI lib: " << e.what() << std::endl;
    }
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

const GameBoard::MovePermission GameBoard::move_permissions[8][10] = {
    {B, S, B, B, B, B, B, B, R, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, B, B, B, B, B, B, B, B, S},
    {R, S, B, B, B, B, B, B, R, B}};

// clang-format off
std::unordered_map<GameBoard::LaserDirection, std::unordered_map<GameBoard::PieceType, std::unordered_map<GameBoard::PieceOrientation, GameBoard::ReflectionResult>>> GameBoard::reflections_map = {
    {NORTH, {
        {ANUBIS, {{ORIENT_NORTH, RESULT_DEAD}, {ORIENT_EAST, RESULT_DEAD}, {ORIENT_SOUTH, RESULT_ABSORBED}, {ORIENT_WEST, RESULT_DEAD}}},
        {PYRAMID, {{ORIENT_NORTH, RESULT_DEAD}, {ORIENT_EAST, RESULT_EAST}, {ORIENT_SOUTH, RESULT_WEST}, {ORIENT_WEST, RESULT_DEAD}}},
        {SCARAB, {{ORIENT_NORTH, RESULT_WEST}, {ORIENT_EAST, RESULT_EAST}, {ORIENT_SOUTH, RESULT_WEST}, {ORIENT_WEST, RESULT_EAST}}},
        {PHARAOH, {{ORIENT_NORTH, RESULT_DEAD}, {ORIENT_EAST, RESULT_DEAD}, {ORIENT_SOUTH, RESULT_DEAD}, {ORIENT_WEST, RESULT_DEAD}}},
        {SPHINX, {{ORIENT_NORTH, RESULT_ABSORBED}, {ORIENT_EAST, RESULT_ABSORBED}, {ORIENT_SOUTH, RESULT_ABSORBED}, {ORIENT_WEST, RESULT_ABSORBED}}}
    }},
    {EAST, {
        {ANUBIS, {{ORIENT_NORTH, RESULT_DEAD}, {ORIENT_EAST, RESULT_DEAD}, {ORIENT_SOUTH, RESULT_DEAD}, {ORIENT_WEST, RESULT_ABSORBED}}},
        {PYRAMID, {{ORIENT_NORTH, RESULT_DEAD}, {ORIENT_EAST, RESULT_DEAD}, {ORIENT_SOUTH, RESULT_SOUTH}, {ORIENT_WEST, RESULT_NORTH}}},
        {SCARAB, {{ORIENT_NORTH, RESULT_SOUTH}, {ORIENT_EAST, RESULT_NORTH}, {ORIENT_SOUTH, RESULT_SOUTH}, {ORIENT_WEST, RESULT_NORTH}}},
        {PHARAOH, {{ORIENT_NORTH, RESULT_DEAD}, {ORIENT_EAST, RESULT_DEAD}, {ORIENT_SOUTH, RESULT_DEAD}, {ORIENT_WEST, RESULT_DEAD}}},
        {SPHINX, {{ORIENT_NORTH, RESULT_ABSORBED}, {ORIENT_EAST, RESULT_ABSORBED}, {ORIENT_SOUTH, RESULT_ABSORBED}, {ORIENT_WEST, RESULT_ABSORBED}}}
    }},
    {SOUTH, {
        {ANUBIS, {{ORIENT_NORTH, RESULT_ABSORBED}, {ORIENT_EAST, RESULT_DEAD}, {ORIENT_SOUTH, RESULT_DEAD}, {ORIENT_WEST, RESULT_DEAD}}},
        {PYRAMID, {{ORIENT_NORTH, RESULT_EAST}, {ORIENT_EAST, RESULT_DEAD}, {ORIENT_SOUTH, RESULT_DEAD}, {ORIENT_WEST, RESULT_WEST}}},
        {SCARAB, {{ORIENT_NORTH, RESULT_EAST}, {ORIENT_EAST, RESULT_WEST}, {ORIENT_SOUTH, RESULT_EAST}, {ORIENT_WEST, RESULT_WEST}}},
        {PHARAOH, {{ORIENT_NORTH, RESULT_DEAD}, {ORIENT_EAST, RESULT_DEAD}, {ORIENT_SOUTH, RESULT_DEAD}, {ORIENT_WEST, RESULT_DEAD}}},
        {SPHINX, {{ORIENT_NORTH, RESULT_ABSORBED}, {ORIENT_EAST, RESULT_ABSORBED}, {ORIENT_SOUTH, RESULT_ABSORBED}, {ORIENT_WEST, RESULT_ABSORBED}}}
    }},
    {WEST, {
        {ANUBIS, {{ORIENT_NORTH, RESULT_DEAD}, {ORIENT_EAST, RESULT_ABSORBED}, {ORIENT_SOUTH, RESULT_DEAD}, {ORIENT_WEST, RESULT_DEAD}}},
        {PYRAMID, {{ORIENT_NORTH, RESULT_NORTH}, {ORIENT_EAST, RESULT_SOUTH}, {ORIENT_SOUTH, RESULT_DEAD}, {ORIENT_WEST, RESULT_DEAD}}},
        {SCARAB, {{ORIENT_NORTH, RESULT_NORTH}, {ORIENT_EAST, RESULT_SOUTH}, {ORIENT_SOUTH, RESULT_NORTH}, {ORIENT_WEST, RESULT_SOUTH}}},
        {PHARAOH, {{ORIENT_NORTH, RESULT_DEAD}, {ORIENT_EAST, RESULT_DEAD}, {ORIENT_SOUTH, RESULT_DEAD}, {ORIENT_WEST, RESULT_DEAD}}},
        {SPHINX, {{ORIENT_NORTH, RESULT_ABSORBED}, {ORIENT_EAST, RESULT_ABSORBED}, {ORIENT_SOUTH, RESULT_ABSORBED}, {ORIENT_WEST, RESULT_ABSORBED}}}
    }}
};
