#include "drawing.h"
#include "khet-sdl.h"
#include <math.h>

void draw_piece(void *app_state_ptr, int row, int col) {
    AppState *as = (AppState *)app_state_ptr;

    switch(as->board[row][col].piece->piece_type) {
        case PYRAMID: draw_pyramid(app_state_ptr, row, col); break;
        case SCARAB: draw_scarab(app_state_ptr, row, col); break;
        case ANUBIS: draw_anubis(app_state_ptr, row, col); break;
        case PHARAOH: draw_pharaoh(app_state_ptr, row, col); break;
        case LASER: draw_laser(app_state_ptr, row, col); break;
        default: break;
    }
}

void draw_mirror(SDL_Renderer *ren, SDL_FPoint p1, SDL_FPoint p2, double thickness) {
    double edge_dx = p2.x - p1.x;
    double edge_dy = p2.y - p1.y;
    double edge_length = sqrt(edge_dx * edge_dx + edge_dy * edge_dy);

    // Normalize
    double unit_x = edge_dx / edge_length;
    double unit_y = edge_dy / edge_length;

    // Compute perpendicular offset
    double perp_x = -unit_y * thickness;
    double perp_y = unit_x * thickness;

    // Define mirror quad
    SDL_Vertex mirror_vertices[4] = {
        {{p1.x - perp_x, p1.y - perp_y}, MIRROR_COLOR, {0, 0}},
        {{p2.x - perp_x, p2.y - perp_y}, MIRROR_COLOR, {0, 0}},
        {{p1.x + perp_x, p1.y + perp_y}, MIRROR_COLOR, {0, 0}},
        {{p2.x + perp_x, p2.y + perp_y}, MIRROR_COLOR, {0, 0}}};

    int mirror_indices[6] = {0, 1, 2, 1, 3, 2};
    SDL_RenderGeometry(ren, NULL, mirror_vertices, 4, mirror_indices, 6);
}

void draw_laser(void *app_state_ptr, int row, int col) {
    AppState *as = (AppState *)app_state_ptr;
    SDL_Renderer *ren = as->ren;
    Point op = as->board[row][col].point;
    Point cp = {(op.x + SQUARE_SIZE * 0.5), (op.y + SQUARE_SIZE * 0.5)};

    SDL_FColor piece_color = as->board[row][col].piece->color == SILVER ? SILVER_COLOR : RED_COLOR;

    double half_size = PIECE_SIZE * 0.5;
    Point original_vertices[3] = {
        {cp.x, cp.y - half_size},
        {cp.x - half_size, cp.y + half_size},
        {cp.x + half_size, cp.y + half_size}
    };

    double angle = 0;
    switch (as->board[row][col].piece->direction) {
        case NORTH: angle = 0; break;
        case EAST: angle = M_PI / 2; break;
        case SOUTH: angle = M_PI; break;
        case WEST: angle = 3 * M_PI / 2; break;
    }

    double cos_a = cos(angle);
    double sin_a = sin(angle);

    SDL_Vertex vertices[3];
    for (int i = 0; i < 3; i++) {
        double x = original_vertices[i].x - cp.x;
        double y = original_vertices[i].y - cp.y;
        vertices[i].position.x = cp.x + (x * cos_a - y * sin_a);
        vertices[i].position.y = cp.y + (x * sin_a + y * cos_a);
        vertices[i].color = piece_color;
    }

    SDL_RenderGeometry(ren, NULL, vertices, 3, NULL, 0);
}


void draw_pyramid(void *app_state_ptr, int row, int col) {
    AppState *as = (AppState *)app_state_ptr;
    SDL_Renderer *ren = as->ren;
    Point op = as->board[row][col].point;
    Point cp = {(op.x + SQUARE_SIZE * 0.5), (op.y + SQUARE_SIZE * 0.5)};

    SDL_FColor piece_color = as->board[row][col].piece->color == SILVER ? SILVER_COLOR : RED_COLOR;

    double half_size = PIECE_SIZE * 0.5;
    Point original_vertices[3] = {
        {cp.x - half_size, cp.y - half_size},
        {cp.x + half_size, cp.y + half_size},
        {cp.x - half_size, cp.y + half_size}
    };

    double angle = 0;
    switch (as->board[row][col].piece->direction) {
        case NORTH: angle = 0; break;
        case EAST: angle = M_PI / 2; break;
        case SOUTH: angle = M_PI; break;
        case WEST: angle = 3 * M_PI / 2; break;
    }

    double cos_a = cos(angle);
    double sin_a = sin(angle);

    SDL_Vertex vertices[3];
    for (int i = 0; i < 3; i++) {
        double x = original_vertices[i].x - cp.x;
        double y = original_vertices[i].y - cp.y;
        vertices[i].position.x = cp.x + (x * cos_a - y * sin_a);
        vertices[i].position.y = cp.y + (x * sin_a + y * cos_a);
        vertices[i].color = piece_color;
    }

    SDL_RenderGeometry(ren, NULL, vertices, 3, NULL, 0);

    SDL_FPoint v1 = vertices[0].position;
    SDL_FPoint v2 = vertices[1].position;
    double thickness = 2.0;
    draw_mirror(ren, v1, v2, thickness);
}

void draw_scarab(void *app_state_ptr, int row, int col) {
    AppState *as = (AppState *)app_state_ptr;
    SDL_Renderer *ren = as->ren;
    Point op = as->board[row][col].point;
    Point cp = {(op.x + (SQUARE_SIZE * 0.5)), (op.y + (SQUARE_SIZE * 0.5))};
    SDL_FColor piece_color = as->board[row][col].piece->color == SILVER ? SILVER_COLOR : RED_COLOR;

    double width = PIECE_SIZE * 0.2;
    double height = PIECE_SIZE + (PIECE_SIZE * 0.2);

    double half_w = width * 0.5;
    double half_h = height * 0.5;

    Direction dir = as->board[row][col].piece->direction;
    double angle = (dir == NORTH || dir == SOUTH) ? -M_PI / 4 : M_PI / 4;

    double cos_a = cos(angle);
    double sin_a = sin(angle);

    SDL_Vertex vertices[4] = {
        {{cp.x + (-half_w * cos_a - -half_h * sin_a), cp.y + (-half_w * sin_a + -half_h * cos_a)}, piece_color, {0, 0}},
        {{cp.x + (half_w * cos_a - -half_h * sin_a), cp.y + (half_w * sin_a + -half_h * cos_a)}, piece_color, {0, 0}},
        {{cp.x + (-half_w * cos_a - half_h * sin_a), cp.y + (-half_w * sin_a + half_h * cos_a)}, piece_color, {0, 0}},
        {{cp.x + (half_w * cos_a - half_h * sin_a), cp.y + (half_w * sin_a + half_h * cos_a)}, piece_color, {0, 0}}};

    int indices[6] = {0, 1, 2, 1, 3, 2};

    SDL_RenderGeometry(ren, NULL, vertices, 4, indices, 6);

    double thickness = 2.0;

    SDL_FPoint v1 = vertices[0].position;
    SDL_FPoint v2 = vertices[2].position;
    SDL_FPoint v3 = vertices[1].position;
    SDL_FPoint v4 = vertices[3].position;

    draw_mirror(ren, v1, v2, thickness);
    draw_mirror(ren, v3, v4, thickness);
}

void draw_anubis(void *app_state_ptr, int row, int col) {
    AppState *as = (AppState *)app_state_ptr;
    SDL_Renderer *ren = as->ren;
    Point op = as->board[row][col].point;
    Point p = {(op.x + (SQUARE_SIZE - PIECE_SIZE) * 0.5), (op.y + (SQUARE_SIZE - PIECE_SIZE) * 0.5)};

    SDL_FColor piece_color = as->board[row][col].piece->color == SILVER ? SILVER_COLOR : RED_COLOR;

    SDL_Vertex vertices[4] = {
        {{p.x, p.y}, piece_color, {0, 0}},
        {{p.x + PIECE_SIZE, p.y + PIECE_SIZE}, piece_color, {0, 0}},
        {{p.x, p.y + PIECE_SIZE}, piece_color, {0, 0}},
        {{p.x + PIECE_SIZE, p.y}, piece_color, {0, 0}}};

    int indices[6] = {0, 1, 2, 0, 3, 1};

    SDL_RenderGeometry(ren, NULL, vertices, 4, indices, 6);

    Point cp = {(op.x + (SQUARE_SIZE * 0.5)), (op.y + (SQUARE_SIZE * 0.5))};
    double thickness = 4.0;

    SDL_FPoint v1, v2, v3, v4;
    double half_size = PIECE_SIZE * 0.5;
    switch (as->board[row][col].piece->direction) {
        case NORTH:
            v1 = (SDL_FPoint){cp.x - half_size, cp.y - half_size};
            v2 = (SDL_FPoint){cp.x + half_size, cp.y - half_size};
            v3 = (SDL_FPoint){cp.x + half_size, cp.y - half_size + thickness};
            v4 = (SDL_FPoint){cp.x - half_size, cp.y - half_size + thickness};
            break;
        case EAST:
            v1 = (SDL_FPoint){cp.x + half_size - thickness, cp.y - half_size};
            v2 = (SDL_FPoint){cp.x + half_size, cp.y - half_size};
            v3 = (SDL_FPoint){cp.x + half_size, cp.y + half_size};
            v4 = (SDL_FPoint){cp.x + half_size - thickness, cp.y + half_size};
            break;
        case SOUTH:
            v1 = (SDL_FPoint){cp.x - half_size, cp.y + half_size - thickness};
            v2 = (SDL_FPoint){cp.x + half_size, cp.y + half_size - thickness};
            v3 = (SDL_FPoint){cp.x + half_size, cp.y + half_size};
            v4 = (SDL_FPoint){cp.x - half_size, cp.y + half_size};
            break;
        case WEST:
            v1 = (SDL_FPoint){cp.x - half_size, cp.y - half_size};
            v2 = (SDL_FPoint){cp.x - half_size + thickness, cp.y - half_size};
            v3 = (SDL_FPoint){cp.x - half_size + thickness, cp.y + half_size};
            v4 = (SDL_FPoint){cp.x - half_size, cp.y + half_size};
            break;
    }

    SDL_Vertex black_vertices[4] = {
        {{v1.x, v1.y}, BLACK_COLOR, {0, 0}}, // Top-left
        {{v2.x, v2.y}, BLACK_COLOR, {0, 0}}, // Top-right
        {{v3.x, v3.y}, BLACK_COLOR, {0, 0}}, // Bottom-right
        {{v4.x, v4.y}, BLACK_COLOR, {0, 0}}  // Bottom-left
    };

    int black_indices[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(ren, NULL, black_vertices, 4, black_indices, 6);
}

void draw_pharaoh(void *app_state_ptr, int row, int col) {
    AppState *as = (AppState *)app_state_ptr;
    SDL_Renderer *ren = as->ren;
    Point op = as->board[row][col].point;
    Point cp = {(op.x + (SQUARE_SIZE * 0.5)), (op.y + (SQUARE_SIZE * 0.5))};
    double radius = PIECE_SIZE * 0.5;
    int segments = 20;

    SDL_FColor piece_color = as->board[row][col].piece->color == SILVER ? SILVER_COLOR : RED_COLOR;

    SDL_Vertex vertices[segments + 2];
    vertices[0].position.x = cp.x;
    vertices[0].position.y = cp.y;
    vertices[0].color = piece_color;

    for (int i = 0; i < segments; i++) {
        int v = i + 1;
        double angle = (2.0 * M_PI * i) / segments;
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