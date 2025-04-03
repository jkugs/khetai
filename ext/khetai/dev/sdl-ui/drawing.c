#include "drawing.h"
#include <math.h>

void draw(AppState *as) {
    SDL_SetRenderDrawColor(as->ren, 170, 170, 170, 255);
    SDL_RenderClear(as->ren);
    SDL_SetRenderDrawBlendMode(as->ren, SDL_BLENDMODE_BLEND);

    // Highlight selected square
    if (as->selected) {
        // Yellow - highlight
        SDL_SetRenderDrawColor(as->ren, 255, 230, 150, 150);
        SDL_FRect highlight_square = {
            (WINDOW_BUFFER) + as->selected_pos.col * SQUARE_SIZE,
            (WINDOW_BUFFER) + as->selected_pos.row * SQUARE_SIZE,
            SQUARE_SIZE, SQUARE_SIZE};
        SDL_RenderFillRect(as->ren, &highlight_square);

        // Highlight valid squares
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (as->valid_squares[i][j] == 1 && (i != as->selected_pos.row || j != as->selected_pos.col)) {
                    SDL_SetRenderDrawColor(as->ren, 150, 230, 155, 150);
                    SDL_FRect valid_square = {
                        (WINDOW_BUFFER) + j * SQUARE_SIZE,
                        (WINDOW_BUFFER) + i * SQUARE_SIZE,
                        SQUARE_SIZE, SQUARE_SIZE};
                    SDL_RenderFillRect(as->ren, &valid_square);
                }
            }
        }
    }

    // Black
    SDL_SetRenderDrawColor(as->ren, 0, 0, 0, 255);
    SDL_FRect square = {WINDOW_BUFFER, WINDOW_BUFFER, BOARD_WIDTH, BOARD_HEIGHT};
    SDL_RenderRect(as->ren, &square);

    // Draw vertical lines
    for (int i = 0; i <= 8; i++) {
        int x = (i * SQUARE_SIZE) + (WINDOW_BUFFER + SQUARE_SIZE);
        int y = (WINDOW_BUFFER);
        SDL_RenderLine(as->ren, x, y, x, (BOARD_HEIGHT + y) - 1);
    }
    // Draw horizontal lines
    for (int i = 0; i <= 6; i++) {
        int x = (WINDOW_BUFFER);
        int y = (i * SQUARE_SIZE) + (WINDOW_BUFFER + SQUARE_SIZE);
        SDL_RenderLine(as->ren, x, y, (BOARD_WIDTH + x) - 1, y);
    }

    // Draw inner squares
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (square_colors[i][j] == R) {
                draw_inner_square(as, i, j, RED_COLOR);
            } else if (square_colors[i][j] == S) {
                draw_inner_square(as, i, j, SILVER_COLOR);
            }
        }
    }

    // Draw laser
    if (as->drawing_laser) {
        draw_laser_animation(as);
    }

    // Draw pieces
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (as->board[i][j].piece != NULL) {
                draw_piece(as, i, j);
            }
        }
    }

    // Tinted overlays
    // SDL_SetRenderDrawColor(as->ren, 200, 200, 255, 30);
    SDL_SetRenderDrawColor(as->ren, 0, 0, 0, 40);
    SDL_FRect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(as->ren, &overlay);

    if (as->game_over) {
        draw_end_overlay(as);
    }

    SDL_RenderPresent(as->ren);
}

void draw_laser_animation(AppState *as) {
    SDL_Renderer *ren = as->ren;
    Laser *laser = &as->laser;

    for (int i = 0; i < laser->num_segments; i++) {
        LaserSegment *segment = &laser->segments[i];
        SDL_FPoint p1 = segment->p1;
        SDL_FPoint p2 = segment->p2;

        float edge_dx = p2.x - p1.x;
        float edge_dy = p2.y - p1.y;
        float edge_length = sqrt(edge_dx * edge_dx + edge_dy * edge_dy);

        // Normalize
        float unit_x = edge_dx / edge_length;
        float unit_y = edge_dy / edge_length;

        // Compute perpendicular offset
        float perp_x = -unit_y * 1.0;
        float perp_y = unit_x * 1.0;

        // Define laser segment quad
        SDL_Vertex laser_vertices[4] = {
            {{p1.x - perp_x, p1.y - perp_y}, RED_COLOR, {0, 0}},
            {{p2.x - perp_x, p2.y - perp_y}, RED_COLOR, {0, 0}},
            {{p1.x + perp_x, p1.y + perp_y}, RED_COLOR, {0, 0}},
            {{p2.x + perp_x, p2.y + perp_y}, RED_COLOR, {0, 0}}};

        int laser_indices[6] = {0, 1, 2, 1, 3, 2};
        SDL_RenderGeometry(ren, NULL, laser_vertices, 4, laser_indices, 6);
    }
}

void draw_inner_square(AppState *as, int row, int col, SDL_FColor color) {
    SDL_Renderer *ren = as->ren;
    SDL_FPoint op = as->board[row][col].point;
    SDL_FPoint cp = {(op.x + SQUARE_SIZE * 0.5f), (op.y + SQUARE_SIZE * 0.5f)};

    float inner_square_size = PIECE_SIZE * 0.8f;
    float half_size = inner_square_size * 0.5f;
    float x = cp.x - half_size;
    float y = cp.y - half_size;

    SDL_FRect square = {x, y, inner_square_size, inner_square_size};
    Uint8 r = (Uint8)(color.r * 255);
    Uint8 g = (Uint8)(color.g * 255);
    Uint8 b = (Uint8)(color.b * 255);
    Uint8 a = (Uint8)(color.a * 255);
    SDL_SetRenderDrawColor(ren, r, g, b, a);
    SDL_RenderRect(ren, &square);
}

void draw_piece(AppState *as, int row, int col) {
    switch (as->board[row][col].piece->piece_type) {
    case PYRAMID_SDL: draw_pyramid(as, row, col); break;
    case SCARAB_SDL: draw_scarab(as, row, col); break;
    case ANUBIS_SDL: draw_anubis(as, row, col); break;
    case PHARAOH_SDL: draw_pharaoh(as, row, col); break;
    case SPHINX_SDL: draw_sphinx(as, row, col); break;
    default: break;
    }
}

void draw_mirror(SDL_Renderer *ren, SDL_FPoint p1, SDL_FPoint p2, float thickness) {
    float edge_dx = p2.x - p1.x;
    float edge_dy = p2.y - p1.y;
    float edge_length = sqrt(edge_dx * edge_dx + edge_dy * edge_dy);

    // Normalize
    float unit_x = edge_dx / edge_length;
    float unit_y = edge_dy / edge_length;

    // Compute perpendicular offset
    float perp_x = -unit_y * thickness;
    float perp_y = unit_x * thickness;

    // Define mirror quad
    SDL_Vertex mirror_vertices[4] = {
        {{p1.x - perp_x, p1.y - perp_y}, MIRROR_COLOR, {0, 0}},
        {{p2.x - perp_x, p2.y - perp_y}, MIRROR_COLOR, {0, 0}},
        {{p1.x + perp_x, p1.y + perp_y}, MIRROR_COLOR, {0, 0}},
        {{p2.x + perp_x, p2.y + perp_y}, MIRROR_COLOR, {0, 0}}};

    int mirror_indices[6] = {0, 1, 2, 1, 3, 2};
    SDL_RenderGeometry(ren, NULL, mirror_vertices, 4, mirror_indices, 6);
}

void draw_sphinx(AppState *as, int row, int col) {
    SDL_Renderer *ren = as->ren;
    SDL_FPoint cp = as->board[row][col].piece->cp;
    SDL_FColor piece_color = as->board[row][col].piece->color == SILVER_SDL ? SILVER_COLOR : RED_COLOR;

    float half_size = PIECE_SIZE * 0.5f;
    SDL_FPoint original_vertices[3] = {
        {cp.x, cp.y - half_size},
        {cp.x - half_size, cp.y + half_size},
        {cp.x + half_size, cp.y + half_size}};

    float angle = 0.0f;
    switch (as->board[row][col].piece->orientation) {
    case NORTH_SDL: angle = 0.0f; break;
    case EAST_SDL: angle = M_PI / 2.0f; break;
    case SOUTH_SDL: angle = M_PI; break;
    case WEST_SDL: angle = 3.0f * M_PI / 2.0f; break;
    }

    float cos_a = cos(angle);
    float sin_a = sin(angle);

    SDL_Vertex vertices[3];
    for (int i = 0; i < 3; i++) {
        float x = original_vertices[i].x - cp.x;
        float y = original_vertices[i].y - cp.y;
        vertices[i].position.x = cp.x + (x * cos_a - y * sin_a);
        vertices[i].position.y = cp.y + (x * sin_a + y * cos_a);
        vertices[i].color = piece_color;
    }

    SDL_RenderGeometry(ren, NULL, vertices, 3, NULL, 0);
}

void draw_pyramid(AppState *as, int row, int col) {
    SDL_Renderer *ren = as->ren;
    SDL_FPoint cp = as->board[row][col].piece->cp;
    SDL_FColor piece_color = as->board[row][col].piece->color == SILVER_SDL ? SILVER_COLOR : RED_COLOR;

    float half_size = PIECE_SIZE * 0.5f;
    SDL_FPoint original_vertices[3] = {
        {cp.x - half_size, cp.y - half_size},
        {cp.x + half_size, cp.y + half_size},
        {cp.x - half_size, cp.y + half_size}};

    float angle = 0.0f;
    switch (as->board[row][col].piece->orientation) {
    case NORTH_SDL: angle = 0.0f; break;
    case EAST_SDL: angle = M_PI / 2.0f; break;
    case SOUTH_SDL: angle = M_PI; break;
    case WEST_SDL: angle = 3.0f * M_PI / 2.0f; break;
    }

    float cos_a = cos(angle);
    float sin_a = sin(angle);

    SDL_Vertex vertices[3];
    for (int i = 0; i < 3; i++) {
        float x = original_vertices[i].x - cp.x;
        float y = original_vertices[i].y - cp.y;
        vertices[i].position.x = cp.x + (x * cos_a - y * sin_a);
        vertices[i].position.y = cp.y + (x * sin_a + y * cos_a);
        vertices[i].color = piece_color;
    }

    SDL_RenderGeometry(ren, NULL, vertices, 3, NULL, 0);

    SDL_FPoint v1 = vertices[0].position;
    SDL_FPoint v2 = vertices[1].position;
    float thickness = 2.0f;
    draw_mirror(ren, v1, v2, thickness);
}

void draw_scarab(AppState *as, int row, int col) {
    SDL_Renderer *ren = as->ren;
    SDL_FPoint cp = as->board[row][col].piece->cp;
    SDL_FColor piece_color = as->board[row][col].piece->color == SILVER_SDL ? SILVER_COLOR : RED_COLOR;

    float width = PIECE_SIZE * 0.12f;
    float height = PIECE_SIZE + (PIECE_SIZE * 0.1f);

    float half_w = width * 0.5f;
    float half_h = height * 0.5f;

    Orientation_SDL dir = as->board[row][col].piece->orientation;
    float angle = (dir == NORTH_SDL || dir == SOUTH_SDL) ? -M_PI / 4.0f : M_PI / 4.0f;

    float cos_a = cos(angle);
    float sin_a = sin(angle);

    SDL_Vertex vertices[4] = {
        {{cp.x + (-half_w * cos_a - -half_h * sin_a), cp.y + (-half_w * sin_a + -half_h * cos_a)}, piece_color, {0, 0}},
        {{cp.x + (half_w * cos_a - -half_h * sin_a), cp.y + (half_w * sin_a + -half_h * cos_a)}, piece_color, {0, 0}},
        {{cp.x + (-half_w * cos_a - half_h * sin_a), cp.y + (-half_w * sin_a + half_h * cos_a)}, piece_color, {0, 0}},
        {{cp.x + (half_w * cos_a - half_h * sin_a), cp.y + (half_w * sin_a + half_h * cos_a)}, piece_color, {0, 0}}};

    int indices[6] = {0, 1, 2, 1, 3, 2};

    SDL_RenderGeometry(ren, NULL, vertices, 4, indices, 6);

    float thickness = 1.2f;

    SDL_FPoint v1 = vertices[0].position;
    SDL_FPoint v2 = vertices[2].position;
    SDL_FPoint v3 = vertices[1].position;
    SDL_FPoint v4 = vertices[3].position;

    draw_mirror(ren, v1, v2, thickness);
    draw_mirror(ren, v3, v4, thickness);
}

void draw_anubis(AppState *as, int row, int col) {
    SDL_Renderer *ren = as->ren;
    SDL_FPoint cp = as->board[row][col].piece->cp;
    SDL_FPoint p = {(cp.x - (PIECE_SIZE * 0.5f)), (cp.y - (PIECE_SIZE * 0.5f))};
    SDL_FColor piece_color = as->board[row][col].piece->color == SILVER_SDL ? SILVER_COLOR : RED_COLOR;

    SDL_Vertex vertices[4] = {
        {{p.x, p.y}, piece_color, {0, 0}},
        {{p.x + PIECE_SIZE, p.y + PIECE_SIZE}, piece_color, {0, 0}},
        {{p.x, p.y + PIECE_SIZE}, piece_color, {0, 0}},
        {{p.x + PIECE_SIZE, p.y}, piece_color, {0, 0}}};

    int indices[6] = {0, 1, 2, 0, 3, 1};

    SDL_RenderGeometry(ren, NULL, vertices, 4, indices, 6);

    float thickness = 7.0f;

    SDL_FPoint v1, v2, v3, v4;
    float half_size = PIECE_SIZE * 0.5f;
    switch (as->board[row][col].piece->orientation) {
    case NORTH_SDL:
        v1 = (SDL_FPoint){cp.x - half_size, cp.y - half_size};
        v2 = (SDL_FPoint){cp.x + half_size, cp.y - half_size};
        v3 = (SDL_FPoint){cp.x + half_size, cp.y - half_size + thickness};
        v4 = (SDL_FPoint){cp.x - half_size, cp.y - half_size + thickness};
        break;
    case EAST_SDL:
        v1 = (SDL_FPoint){cp.x + half_size - thickness, cp.y - half_size};
        v2 = (SDL_FPoint){cp.x + half_size, cp.y - half_size};
        v3 = (SDL_FPoint){cp.x + half_size, cp.y + half_size};
        v4 = (SDL_FPoint){cp.x + half_size - thickness, cp.y + half_size};
        break;
    case SOUTH_SDL:
        v1 = (SDL_FPoint){cp.x - half_size, cp.y + half_size - thickness};
        v2 = (SDL_FPoint){cp.x + half_size, cp.y + half_size - thickness};
        v3 = (SDL_FPoint){cp.x + half_size, cp.y + half_size};
        v4 = (SDL_FPoint){cp.x - half_size, cp.y + half_size};
        break;
    case WEST_SDL:
        v1 = (SDL_FPoint){cp.x - half_size, cp.y - half_size};
        v2 = (SDL_FPoint){cp.x - half_size + thickness, cp.y - half_size};
        v3 = (SDL_FPoint){cp.x - half_size + thickness, cp.y + half_size};
        v4 = (SDL_FPoint){cp.x - half_size, cp.y + half_size};
        break;
    }

    SDL_Vertex black_vertices[4] = {
        {{v1.x, v1.y}, BLACK_COLOR, {0, 0}},
        {{v2.x, v2.y}, BLACK_COLOR, {0, 0}},
        {{v3.x, v3.y}, BLACK_COLOR, {0, 0}},
        {{v4.x, v4.y}, BLACK_COLOR, {0, 0}}};

    int black_indices[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(ren, NULL, black_vertices, 4, black_indices, 6);
}

void draw_pharaoh(AppState *as, int row, int col) {
    SDL_Renderer *ren = as->ren;
    SDL_FPoint cp = as->board[row][col].piece->cp;
    float radius = PIECE_SIZE * 0.5f;
    int segments = 20;
    SDL_FColor piece_color = as->board[row][col].piece->color == SILVER_SDL ? SILVER_COLOR : RED_COLOR;

    SDL_Vertex vertices[segments + 2];
    vertices[0].position.x = cp.x;
    vertices[0].position.y = cp.y;
    vertices[0].color = piece_color;

    for (int i = 0; i < segments; i++) {
        int v = i + 1;
        float angle = (2.0 * M_PI * i) / segments;
        vertices[v].position.x = cp.x + cos(angle) * radius;
        vertices[v].position.y = cp.y + sin(angle) * radius;
        vertices[v].color = piece_color;
    }

    vertices[segments + 1] = vertices[1];

    int num_indices = segments * 3;
    int indices[num_indices];

    for (int i = 0; i < segments; i++) {
        indices[i * 3] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }

    indices[num_indices - 1] = 1;

    SDL_RenderGeometry(ren, NULL, vertices, segments + 2, indices, num_indices);
}

void draw_end_overlay(AppState *as) {
    SDL_Renderer *ren = as->ren;
    SDL_SetRenderDrawColor(ren, 30, 30, 30, 175);
    SDL_FRect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(ren, &overlay);
    SDL_RenderTexture(as->ren, as->play_again_button, NULL, &as->play_again_rect);
}