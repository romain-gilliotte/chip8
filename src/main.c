#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include "chip8.h"
#include "interpreter/interpreter.h"

static int process_events(Chip8 *state)
{
    SDL_Event e;
    while (SDL_PollEvent(&e) > 0)
    {
    }

    return 0;
}

/** Redraw full screen at each frame */
static void render(SDL_Surface *surface, Chip8 *state)
{
    const uint32_t width = surface->w;
    const uint32_t height = surface->h;
    uint32_t *pixels = surface->pixels;

    for (uint32_t i = 0; i < width * height; ++i)
    {
        const uint32_t x_window = i % width;
        const uint32_t y_window = i / width;
        const uint32_t x_chip8 = x_window * state->screen_width / width;
        const uint32_t y_chip8 = y_window * state->screen_height / height;

        pixels[i] = state->display[y_chip8 * state->screen_width + x_chip8] ? 0xffffffff : 0;
    }
}

static uint64_t get_elapsed(struct timeval *begin)
{
    struct timeval end;

    gettimeofday(&end, 0);
    long seconds = end.tv_sec - begin->tv_sec;
    long microseconds = end.tv_usec - begin->tv_usec;

    return seconds * 1000000 + microseconds;
}

int main(const int argc, const char **argv)
{
    // Init Chip8 & SDL
    Chip8 state;
    chip8_init(&state, 64, 32, 500, "../roms/demos/Maze [David Winter, 199x].ch8");

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Chip8",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          640, 480,
                                          SDL_WINDOW_RESIZABLE);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    // Loop
    struct timeval begin;
    gettimeofday(&begin, 0);
    while (1)
    {
        process_events(&state);

        double elapsed = get_elapsed(&begin);
        if (chip8_run(&state, elapsed))
            return 1;

        render(surface, &state);

        SDL_UpdateWindowSurface(window);

        // Sleep 16ms
        struct timespec ts = {.tv_sec = 0, .tv_nsec = 32000000};
        nanosleep(&ts, NULL);
    }
}
