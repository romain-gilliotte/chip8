#include <errno.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <chip8.h>
#include <interpreter/interpreter.h>

static bool process_events(Chip8 *state)
{
    SDL_Event e;

    while (SDL_PollEvent(&e) > 0) {
        switch (e.type) {
            case SDL_QUIT:
                return true;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                switch (e.key.keysym.sym) {
                    case SDLK_1: state->keyboard[0x1] = e.type == SDL_KEYDOWN; break;
                    case SDLK_2: state->keyboard[0x2] = e.type == SDL_KEYDOWN; break; 
                    case SDLK_3: state->keyboard[0x3] = e.type == SDL_KEYDOWN; break;
                    case SDLK_4: state->keyboard[0xC] = e.type == SDL_KEYDOWN; break;
                    case SDLK_q: state->keyboard[0x4] = e.type == SDL_KEYDOWN; break;
                    case SDLK_w: state->keyboard[0x5] = e.type == SDL_KEYDOWN; break;
                    case SDLK_e: state->keyboard[0x6] = e.type == SDL_KEYDOWN; break;
                    case SDLK_r: state->keyboard[0xD] = e.type == SDL_KEYDOWN; break;
                    case SDLK_a: state->keyboard[0x7] = e.type == SDL_KEYDOWN; break;
                    case SDLK_s: state->keyboard[0x8] = e.type == SDL_KEYDOWN; break;
                    case SDLK_d: state->keyboard[0x9] = e.type == SDL_KEYDOWN; break;
                    case SDLK_f: state->keyboard[0xE] = e.type == SDL_KEYDOWN; break;
                    case SDLK_z: state->keyboard[0xA] = e.type == SDL_KEYDOWN; break;
                    case SDLK_x: state->keyboard[0x0] = e.type == SDL_KEYDOWN; break;
                    case SDLK_c: state->keyboard[0xB] = e.type == SDL_KEYDOWN; break;
                    case SDLK_v: state->keyboard[0xF] = e.type == SDL_KEYDOWN; break;
                }
                break;
        }
    }

    return false;
}

/** Redraw full screen at each frame */
static void render(SDL_Window *window, Chip8 *state)
{
    chip8_init()
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    uint32_t width = surface->w;
    uint32_t height = surface->h;
    uint32_t *pixels = surface->pixels;

    for (uint32_t i = 0; i < width * height; ++i)
    {
        uint32_t x_window = i % width;
        uint32_t y_window = i / width;
        uint32_t x_chip8 = x_window * state->screen_width / width;
        uint32_t y_chip8 = y_window * state->screen_height / height;

        pixels[i] = state->display[y_chip8 * state->screen_width + x_chip8] ? 0xffffffff : 0;
    }

    SDL_UpdateWindowSurface(window);
}


int main(const int argc, const char **argv)
{
    // Init Chip8 & SDL
    Chip8 state;
    chip8_init(&state, 64, 32, 500);
    chip8_load_rom(&state, "/home/eloims/Projects/Personal/Chip8/roms/demos/Trip8 Demo (2008) [Revival Studios].ch8");

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_RESIZABLE);

    // Loop
    bool finished = false;
    uint32_t started_at = SDL_GetTicks();
    while (!finished)
    {
        // Process events.
        finished = process_events(&state);

        // Run Chip8
        uint32_t ticks = SDL_GetTicks() - started_at;
        if (interpreter_run(&state, ticks))
            return 1;

        // Render
        if (state.display_dirty) {
            render(window, &state);
            state.display_dirty = false;
        }

        SDL_Delay(1);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
