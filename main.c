#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#include "SDL3/SDL.h"
#include "SDL3_ttf/SDL_ttf.h"

typedef struct State {
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_WindowFlags window_flags;
    const bool *kb_state;

    const char* font_path;
    TTF_Font* font;
    uint32_t window_width;
    uint32_t window_height;
    const char* window_title;

    uint64_t target_fps_ns;
    uint64_t current_time_ns;
    uint64_t previous_time_ns;

    float font_size;
    float dt;
    uint16_t target_fps;
    bool window_should_close;
} State;

// https://wiki.libsdl.org/SDL3/BestKeyboardPractices
void update_state_from_arrowkey_inputs(State *state, SDL_FRect *rect, float step_size) {
    if (state->kb_state[SDL_SCANCODE_RIGHT]) {
        if (rect->x + rect->w + step_size <= state->window_width) {
            rect->x += step_size;
        }
    }
    if (state->kb_state[SDL_SCANCODE_LEFT]) {
        if (rect->x - step_size >= 0) {
            rect->x -= step_size;
        }
    }
    if (state->kb_state[SDL_SCANCODE_UP]) {
        if (rect->y - step_size >= 0 ) {
            rect->y -= step_size;
        }
    }
    if (state->kb_state[SDL_SCANCODE_DOWN]) {
        if (rect->y + rect->h + step_size <= state->window_height) {
            rect->y += step_size;
        }
    }
}

int main()
{
    State state = {
        .window_title="SDL3",
        .window_width=800,
        .window_height=600,
        .font_path="assets/IBM_Plex_Mono/IBMPlexMono-Light.ttf",
        .font_size=32.f,
        .target_fps=60,
        .window_flags=0,

        // to be initialized later
        .renderer=0,
        .window=0,
        .kb_state=0,
        .event={0},
        .target_fps_ns=0,
        .current_time_ns=0,
        .previous_time_ns=0,
        .dt=0,
        .window_should_close=0,
    };

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        SDL_Quit();
    }
    if (!TTF_Init()) {
        SDL_Log("Failed to initialize SDL_ttf: %s", SDL_GetError());
        SDL_Quit();
    }
    if (!SDL_CreateWindowAndRenderer(state.window_title, state.window_width, state.window_height, state.window_flags, &state.window, &state.renderer)) {
        SDL_Log("Failed to create window+renderer: %s", SDL_GetError());
        SDL_Quit();
        TTF_Quit();
    }

    state.font = TTF_OpenFont(state.font_path, state.font_size);
    if (!state.font) {
        SDL_Log("Failed to load font: %s", SDL_GetError());
    }

    TTF_TextEngine *text_engine = TTF_CreateRendererTextEngine(state.renderer);
    if (!text_engine) {
        SDL_Log("Failed to create text engine: %s", SDL_GetError());
    }

    state.kb_state = SDL_GetKeyboardState(NULL);
    state.target_fps_ns = (uint64_t)(1000000000.0 / state.target_fps);
    state.previous_time_ns = SDL_GetTicksNS();

    TTF_Text *greeting_text = TTF_CreateText(text_engine, state.font, "Hello, SDL3 TTF Engine!", 0);
    if (!greeting_text) {
        SDL_Log("Failed to create TTF_Text: %s", SDL_GetError());
    }
    TTF_SetTextColor(greeting_text, 255, 255, 255, 255);

    int32_t text_width = 0;
    int32_t text_height = 0;
    TTF_GetTextSize(greeting_text, &text_width, &text_height);

    SDL_FRect rect = {.x = 50.f, .y = 50.f, .w = 100.f, .h = 100.f};
    SDL_Color rectcolor = {.r=255, .g = 0, .b = 255, .a= 255};

    while (!state.window_should_close)
    {
        state.current_time_ns = SDL_GetTicksNS();
        state.dt = (float)(state.current_time_ns - state.previous_time_ns) / 1e9f;
        if (state.dt > 0.1f) {
            state.dt = 0.1f;
        }
        state.previous_time_ns = state.current_time_ns;

        while (SDL_PollEvent(&state.event)) {
            if (state.event.type == SDL_EVENT_QUIT) {
                // CLOSE WINDOW NEXT FRAME
                state.window_should_close = true;
            }
        }

        // UPDATE
        float speed = 400.0f;
        float step_size = speed * state.dt;
        update_state_from_arrowkey_inputs(&state, &rect, step_size);

        // CLEAR
        SDL_SetRenderScale(state.renderer, 1.0f, 1.0f);
        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
        SDL_RenderClear(state.renderer);

        // DRAW
        SDL_SetRenderDrawColor(state.renderer, rectcolor.r, rectcolor.g, rectcolor.b, rectcolor.a);
        SDL_RenderRect(state.renderer, &(SDL_FRect){.x=rect.x, .y=rect.y, .w=rect.w, .h=rect.h});

        TTF_DrawRendererText(greeting_text, (state.window_width - text_width) / 2.0f, (state.window_height - text_height) / 2.0f);

        // RENDER
        SDL_RenderPresent(state.renderer);

        // SLEEP FOR FPS TARGET
        uint64_t frame_duration_ns = SDL_GetTicksNS() - state.current_time_ns;
        if (frame_duration_ns < state.target_fps_ns) {
            SDL_DelayPrecise(state.target_fps_ns - frame_duration_ns);
        }
    }

    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();

    return 0;
}
