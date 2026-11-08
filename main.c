#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#include "SDL3/SDL.h" // IWYU pragma: keep
#include "SDL3_ttf/SDL_ttf.h" // IWYU pragma: keep

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
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        SDL_Quit();
    }
    if (!TTF_Init()) {
        SDL_Log("Failed to initialize SDL_ttf: %s", SDL_GetError());
        SDL_Quit();
    }
    if (!SDL_CreateWindowAndRenderer(screen_title, window, height, 0, &global.window, &global.renderer)) {
        SDL_Log("Failed to create window+renderer: %s", SDL_GetError());
        SDL_Quit();
        TTF_Quit();
    }

    global.window_width = window;
    global.window_height = height;
    global.kb_state = SDL_GetKeyboardState(NULL);
    global.target_fps_ns = (uint64_t)(1000000000.0 / 60.f); // aim for 60fps by default
    global.previous_time_ns = SDL_GetTicksNS(); // populate prevtime by default for if target_fps isn't called
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

BARD_WRAP void bard_draw_rectangle(SDL_FRect frect, SDL_Color color) {
    SDL_SetRenderDrawColor(global.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderRect(global.renderer, &(SDL_FRect){.x=frect.x, .y=frect.y, .w=frect.w, .h=frect.h});
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

void update_state_from_arrowkey_inputs(SDL_FRect *rect, float step_size) {
    if (global.kb_state[SDL_SCANCODE_RIGHT]) {
        if (rect->x + rect->w + step_size <= global.window_width) {
            rect->x += step_size;
        }
    }
    if (global.kb_state[SDL_SCANCODE_LEFT]) {
        if (rect->x - step_size >= 0) {
            rect->x -= step_size;
        }
    }
    if (global.kb_state[SDL_SCANCODE_UP]) {
        if (rect->y - step_size >= 0 ) {
            rect->y -= step_size;
        }
    }
    if (global.kb_state[SDL_SCANCODE_DOWN]) {
        if (rect->y + rect->h + step_size <= global.window_height) {
            rect->y += step_size;
        }
    }
}

int main()
{
    bard_init_window(800, 600, "Hello SDL3");
    SDL_Event event = {0};

    const float font_size = 32.0f;
    TTF_Font *font = TTF_OpenFont("assets/IBM_Plex_Mono/IBMPlexMono-Light.ttf", font_size);
    if (!font) {
        SDL_Log("Failed to load font: %s", SDL_GetError());
    }

    TTF_TextEngine *text_engine = TTF_CreateRendererTextEngine(global.renderer);
    if (!text_engine) {
        SDL_Log("Failed to create text engine: %s", SDL_GetError());
    }

    // passing 0 for length measures a null-terminated string
    const char *message = "Hello, SDL3 TTF Engine!";
    TTF_Text *greeting_text = TTF_CreateText(text_engine, font, message, 0);
    if (!greeting_text) {
        SDL_Log("Failed to create TTF_Text: %s", SDL_GetError());
    }

    TTF_SetTextColor(greeting_text, 255, 255, 255, 255);

    int32_t text_width = 0;
    int32_t text_height = 0;
    TTF_GetTextSize(greeting_text, &text_width, &text_height);

    int32_t text_width2 = 0;
    int32_t text_height2 = 0;
    TTF_GetStringSize(font, "Hello, SDL3 TTF Engine!", 0, &text_width2, &text_height2);

    SDL_FRect rect = {.x = 50.f, .y = 50.f, .w = 100.f, .h = 100.f};
    SDL_Color rectcolor = {.r=255, .g = 0, .b = 255, .a= 255};

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

        update_state_from_arrowkey_inputs(&rect, step_size);

        bard_begin_drawing();
            bard_clear_window();
            bard_draw_rectangle(rect, rectcolor);

            TTF_DrawRendererText(greeting_text,
                (global.window_width - text_width) / 2.0f,
                (global.window_height - text_height) / 2.0f);

        bard_end_drawing();
    }

    bard_close_window();

    return 0;
}
