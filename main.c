#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#include "SDL3/SDL.h" // IWYU pragma: keep

static struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    const bool *kb_state;
    uint32_t window_width;
    uint32_t window_height;

    uint64_t target_fps_ns;
    uint64_t current_time_ns;
    uint64_t previous_time_ns;
    float dt;

    bool window_should_close;
} global;

#define BARD_WRAP static inline

BARD_WRAP void bard_init_window(uint32_t window, uint32_t height, const char* screen_title) {
    global.window_width = window;
    global.window_height = height;
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        SDL_Quit();
    }

    if (!SDL_CreateWindowAndRenderer(screen_title, window, height, 0, &global.window, &global.renderer)) {
        SDL_Log("Failed to create window+renderer: %s", SDL_GetError());
        SDL_Quit();
    }
    global.kb_state = SDL_GetKeyboardState(NULL);
    global.target_fps_ns = (uint64_t)(1000000000.0 / 60.f); // aim for 60fps by default
    global.previous_time_ns = SDL_GetTicksNS();
}

BARD_WRAP void bard_target_fps(float target_fps) {
    global.target_fps_ns = (uint64_t)(1000000000.0 / target_fps);
    global.previous_time_ns = SDL_GetTicksNS();
}

BARD_WRAP float bard_dt() {
    return global.dt;
}

BARD_WRAP bool bard_window_should_close() {
    global.current_time_ns = SDL_GetTicksNS();
    global.dt = (float)(global.current_time_ns - global.previous_time_ns) / 1e9f;
    if (global.dt > 0.1f) {
        global.dt = 0.1f;
    }
    global.previous_time_ns = global.current_time_ns;
    // idk if it's bad to put these dt calculations to here
    // maybe dt calc should happen in begin_drawing, but what if user asks for dt before that? (like I did)
    return global.window_should_close;
}

BARD_WRAP void bard_quit_next_frame() {
    global.window_should_close = true;
}

BARD_WRAP void bard_clear_window() {
    SDL_SetRenderDrawColor(global.renderer, 0, 0, 0, 255);
    SDL_RenderClear(global.renderer);
}

BARD_WRAP void bard_begin_drawing() {
    SDL_SetRenderScale(global.renderer, 1.0f, 1.0f);
    SDL_SetRenderDrawColor(global.renderer, 0, 0, 0, 255);
}

BARD_WRAP void bard_draw_rectangle(SDL_FRect rect, SDL_Color color) {
    SDL_SetRenderDrawColor(global.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderRect(global.renderer, &rect);
}

BARD_WRAP void bard_end_drawing() {
    SDL_RenderPresent(global.renderer);

    uint64_t frame_duration_ns = SDL_GetTicksNS() - global.current_time_ns;
    if (frame_duration_ns < global.target_fps_ns) {
        SDL_DelayPrecise(global.target_fps_ns - frame_duration_ns);
    }
}

BARD_WRAP void bard_close_window() {
    SDL_DestroyRenderer(global.renderer);
    SDL_DestroyWindow(global.window);
    SDL_Quit();
}

int main()
{
    bard_init_window(800, 600, "Hello SDL3");
    SDL_Event event = {0};

    struct {
        SDL_FRect rect;
        SDL_Color color;
    } bo = {
        .rect = (SDL_FRect){.x = 50.f, .y = 50.f, .w = 100.f, .h = 100.f},
        .color = (SDL_Color){.r=255, .g = 0, .b = 255, .a= 255}
    };

    bard_target_fps(60);

    while (!bard_window_should_close()) {

        // https://wiki.libsdl.org/SDL3/BestKeyboardPractices
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                bard_quit_next_frame();
            }
        }

        float speed = 400.0f;
        float step_size = speed * bard_dt();

        if (global.kb_state[SDL_SCANCODE_RIGHT]) {
            if (bo.rect.x + bo.rect.w + step_size <= global.window_width) {
                bo.rect.x += step_size;
            }
        }
        if (global.kb_state[SDL_SCANCODE_LEFT]) {
            if (bo.rect.x - step_size >= 0) {
                bo.rect.x -= step_size;
            }
        }
        if (global.kb_state[SDL_SCANCODE_UP]) {
            if (bo.rect.y - step_size >= 0 ) {
                bo.rect.y -= step_size;
            }
        }
        if (global.kb_state[SDL_SCANCODE_DOWN]) {
            if (bo.rect.y + bo.rect.h + step_size <= global.window_height) {
                bo.rect.y += step_size;
            }
        }

        bard_begin_drawing();
        bard_clear_window();

        bard_draw_rectangle(bo.rect, bo.color);

        bard_end_drawing();
    }

    bard_close_window();

    return 0;
}
