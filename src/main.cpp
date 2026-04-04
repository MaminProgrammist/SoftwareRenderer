#include <SDL3/SDL.h>
#include <vector>
#include <cstdlib>
#include <array>
#include "math.h"
#include <limits>

const int WIDTH = 600;
const int HEIGHT = 600;
const float D = 200;

struct Cube
{
    float side;
    Vec4 center;
    Vec4 vertices[8];

    int order[36] = {
        0, 1, 2,
        2, 3, 0,
        0, 3, 4,
        4, 5, 0,
        0, 5, 6,
        6, 0, 1,
        1, 2, 7,
        7, 1, 6,
        6, 5, 7,
        7, 5, 4,
        4, 7, 2,
        2, 3, 4
    };

    Cube(float side, Vec4 center)
    {
        this->side = side;
        this->center = center;

        vertices[0] = Vec4();
        vertices[1] = Vec4(side, 0 ,0);
        vertices[2] = Vec4(side, side, 0);
        vertices[3] = Vec4(0, side, 0);
        vertices[4] = Vec4(0, side, side);
        vertices[5] = Vec4(0, 0, side);
        vertices[6] = Vec4(side, 0, side);
        vertices[7] = Vec4(side, side, side);

        Vec4 shift_v = Vec4::invert(center);
        Mat4 shift_m = Mat4::mat4_translation(shift_v.x, shift_v.y, shift_v.z);

        for (int i = 0; i < 8; i++)
            vertices[i] = Mat4::mat4_multiply_vec4(shift_m, vertices[i]);
    }
};

Vec4 project(Vec4& p, float d)
{
    Vec4 result;

    result.x = (p.x * d) / p.z + WIDTH / 2.0f;
    result.y = (p.y * d) / p.z + HEIGHT / 2.0f;
    result.z = p.z;
    result.w = 1;

    return result;
}

// принимает z трёх вершин, возвращает яркость от 0.0 до 1.0
float calc_brightness(float z0, float z1, float z2, float z_min, float z_max)
{
    float br = (z0 + z1 + z2) / 3.0f;
    br = (z_max - br) / (z_max - z_min);
    return br;
}

// принимает базовый цвет и яркость, возвращает затемнённый цвет
uint32_t apply_brightness(uint32_t color, float brightness)
{
    // взяли каждый канал
    uint32_t r = (color>>24) & 0xFF;
    uint32_t g = (color>>16) & 0xFF;
    uint32_t b = (color>>8) & 0xFF;

    // применяем яркость и каст обратно
    r = static_cast<uint32_t>(brightness * r) << 24;
    g = static_cast<uint32_t>(brightness * g) << 16;
    b = static_cast<uint32_t>(brightness * b) << 8;

    return r | g | b | 0xFF;
}

void set_pixel(std::vector<uint32_t>& framebuffer, int x, int y, uint32_t color)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;

    framebuffer[y * WIDTH + x] = color;
}

void draw_line(std::vector<uint32_t>& framebuffer, int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1; // направление по X
    int sy = (y0 < y1) ? 1 : -1; // направление по Y
    int err = dx - dy;

    while (true)
    {
        set_pixel(framebuffer, x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

int check_side(int xPoint, int yPoint, int x0, int y0, int x1, int y1)
{
    int side = (x1 - x0) * (yPoint - y0) - (y1 - y0) * (xPoint - x0);
    return side >= 0 ? 1 : -1;
}

void fill_triangle(std::vector<uint32_t>& framebuffer, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    int xmin = std::min(std::min(x0, x1), x2);
    int ymin = std::min(std::min(y0, y1), y2);
    int xmax = std::max(std::max(x0, x1), x2);
    int ymax = std::max(std::max(y0, y1), y2);

    for (int x = xmin; x < xmax; x++)
    {
        for (int y = ymin; y < ymax; y++)
        {
            int s0 = check_side(x, y, x0, y0, x1, y1);
            int s1 = check_side(x, y, x1, y1, x2, y2);
            int s2 = check_side(x, y, x2, y2, x0, y0);

            if ((s0 == s1 && s1 == s2))
                set_pixel(framebuffer, x, y, color);
        }
    }
}

void draw_triangle(std::vector<uint32_t>& framebuffer, const Vec4& p0, const Vec4& p1, const Vec4& p2, uint32_t color)
{
    fill_triangle(framebuffer, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, color);
}

void draw_wired_triangle(
    std::vector<uint32_t>& framebuffer,
    const Vec4& p0, const Vec4& p1, const Vec4& p2,
    uint32_t color)
{
    draw_line(framebuffer, p0.x, p0.y, p1.x, p1.y, color);
    draw_line(framebuffer, p1.x, p1.y, p2.x, p2.y, color);
    draw_line(framebuffer, p2.x, p2.y, p0.x, p0.y, color);
}

void draw_wired_cube(std::vector<uint32_t>& framebuffer, const Cube& cube, uint32_t color, bool depthShade = false)
{
    float z_min = std::numeric_limits<float>::max();
    float z_max = std::numeric_limits<float>::lowest();

    for (int i = 0 ; i < 8 ; i++)
    {
        z_min = std::min(z_min, cube.vertices[i].z);
        z_max = std::max(z_max, cube.vertices[i].z);
    }

    for (int i = 0 ; i < 12 ; i++)
    {
        Vec4 indexes[3];

        for (int j = 0 ; j < 3 ; j++)
        {
            indexes[j] = cube.vertices[cube.order[i*3 + j]];
            indexes[j] = project(indexes[j], D);
        }

        uint32_t filtered_color = color;

        if (depthShade)
        {
            float brightness = calc_brightness(indexes[0].z, indexes[1].z, indexes[2].z, z_min, z_max);
            filtered_color = apply_brightness(color, brightness);
            //filtered_color = 0x0000FFFF;
        }

        draw_wired_triangle(framebuffer, indexes[0], indexes[1], indexes[2], filtered_color);
    }
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Software Renderer", WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    SDL_Texture* texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,WIDTH, HEIGHT);

    // Наш framebuffer - просто массив пикселей
    std::vector<uint32_t> framebuffer(WIDTH * HEIGHT);

    // вершины треугольника
    Vec4 A = Vec4(300, 100, 0, 1);
    Vec4 B = Vec4(500, 500, 0, 1);
    Vec4 C = Vec4(100, 500, 0, 1);
    Mat4 T = Mat4::mat4_translation(100, 0, 0);

    Cube cube(200, Vec4(0, 0, 350));

    uint32_t color = 0xFFFFFFFF;

    bool running = true;
    bool swap = false;

    while (running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                color = 0xFF0000FF;
                // A = Mat4::mat4_multiply_vec4(T, A);
                // B = Mat4::mat4_multiply_vec4(T, B);
                // C = Mat4::mat4_multiply_vec4(T, C);
                swap = true;
            }

            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        for (int i = 0; i < WIDTH * HEIGHT; i++)
            framebuffer[i] = 0x000000FF;

        if (swap)
            draw_wired_cube(framebuffer, cube, color, true);
        else
            draw_wired_triangle(framebuffer, A, B, C, color);

        SDL_UpdateTexture(texture, nullptr, framebuffer.data(), WIDTH * sizeof(uint32_t));
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}