#include <errno.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <chip8.h>
#include <interpreter/interpreter.h>
#include <recompiler/recompiler.h>

static bool process_events(Chip8 *state)
{
    SDL_Event e;
    bool key_is_down;

    while (SDL_PollEvent(&e) > 0) {
        switch (e.type) {
            case SDL_QUIT:
                return true;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                key_is_down = e.type == SDL_KEYDOWN;
                switch (e.key.keysym.sym) {
                    case SDLK_1: state->keyboard[0x1] = key_is_down; break;
                    case SDLK_2: state->keyboard[0x2] = key_is_down; break; 
                    case SDLK_3: state->keyboard[0x3] = key_is_down; break;
                    case SDLK_4: state->keyboard[0xC] = key_is_down; break;
                    case SDLK_q: state->keyboard[0x4] = key_is_down; break;
                    case SDLK_w: state->keyboard[0x5] = key_is_down; break;
                    case SDLK_e: state->keyboard[0x6] = key_is_down; break;
                    case SDLK_r: state->keyboard[0xD] = key_is_down; break;
                    case SDLK_a: state->keyboard[0x7] = key_is_down; break;
                    case SDLK_s: state->keyboard[0x8] = key_is_down; break;
                    case SDLK_d: state->keyboard[0x9] = key_is_down; break;
                    case SDLK_f: state->keyboard[0xE] = key_is_down; break;
                    case SDLK_z: state->keyboard[0xA] = key_is_down; break;
                    case SDLK_x: state->keyboard[0x0] = key_is_down; break;
                    case SDLK_c: state->keyboard[0xB] = key_is_down; break;
                    case SDLK_v: state->keyboard[0xF] = key_is_down; break;
                }
                break;
        }
    }

    return false;
}

/** Naively scales pixel buffer, to make chip8 resolution more bearable */
static void scale2x(bool* sm_pb, bool* lg_pb, int width, int height) {
    for (int i = 0; i < width * height; ++i) {
        bool b = sm_pb[i < width ? i : i - width];
        bool d = sm_pb[i % width == 0 ? i : i - 1];
        bool e = sm_pb[i];
        bool f = sm_pb[i % width == width - 1 ? i : i + 1];
        bool h = sm_pb[i + width < width * height ? i + width : i];

        bool e0, e1, e2, e3;
        if (b != h && d != f) {
            e0 = d == b ? d : e;
            e1 = b == f ? f : e;
            e2 = d == h ? d : e;
            e3 = h == f ? f : e;
        } else {
            e0 = e;
            e1 = e;
            e2 = e;
            e3 = e;
        }

        int x = i % width;
        int y = i / width;
        lg_pb[y * 4 * width + 2 * x] = e0;
        lg_pb[y * 4 * width + 2 * x + 1] = e1;
        lg_pb[y * 4 * width + 2 * width + 2 * x] = e2;
        lg_pb[y * 4 * width + 2 * width + 2 * x + 1] = e3;
    }
}

static void render(SDL_Window *window, bool* pixel_buffer, int pb_width, int pb_height)
{
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    uint32_t fgcolor = SDL_MapRGB(surface->format, 0xfe, 0xe7, 0x15);
    uint32_t bgcolor = SDL_MapRGB(surface->format, 0x10, 0x18, 0x20);

    uint32_t width = surface->w;
    uint32_t height = surface->h;
    uint32_t *pixels = surface->pixels;

    for (uint32_t i = 0; i < width * height; ++i)
    {
        uint32_t x_window = i % width;
        uint32_t y_window = i / width;
        uint32_t x_chip8 = x_window * pb_width / width;
        uint32_t y_chip8 = y_window * pb_height / height;

        pixels[i] = pixel_buffer[y_chip8 * pb_width + x_chip8] ? fgcolor : bgcolor;
    }

    SDL_UpdateWindowSurface(window);
}


int main(int argc, const char **argv)
{
    (void) argc;
    (void) argv;
    
    // Init Chip8 & Recompiler
    Chip8 state;
    chip8_init(&state, 64, 32, 500);
    chip8_load_rom(&state, "/home/eloims/Projects/Personal/Chip8/roms/test_opcode.ch8");
    
    CodeCacheRepository repository;
    recompiler_init(&repository);

    // Init SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_RESIZABLE);

    // Main loop
    while (true)
    {
        if (process_events(&state))
            break;

        // if (interpreter_run(&state, ticks))
        if (recompiler_run(&repository, &state, SDL_GetTicks()))
            return 1;

        if (state.display_dirty) {
            bool pb_large[4096 * 4];
            scale2x(state.display, pb_large, state.screen_width, state.screen_height);
            render(window, pb_large, 2 * state.screen_width, 2 * state.screen_height);
            state.display_dirty = false;
        }

        SDL_Delay(1);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
