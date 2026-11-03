#include <float.h>    // Floating-point limits and characteristics
#include <limits.h>   // Sizes and limits of basic integer types
#include <stdarg.h>   // Variable argument lists (va_list, va_start, va_arg, va_end)
#include <stddef.h>   // size_t, ptrdiff_t, NULL, offsetof, max_align_t, nullptr_t
#include <stdint.h>   // Exact-width integer types (uint8_t, int32_t, uintptr_t, etc.)
#include <stdalign.h> // alignas and alignof convenience macros

#include "SDL3/SDL.h" // IWYU pragma: keep

uint64_t bard_strlen(const char *str) {
    uint64_t length = 0;
    while (str[length] != '\0') {length++;}
    return length;
}

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    constexpr uint64_t WINDOW_WIDTH = 800u;
    constexpr uint64_t WINDOW_HEIGHT = 600u;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!SDL_CreateWindowAndRenderer("Hello SDL3", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Failed to create window+renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    bool is_running = true;
    SDL_Event event = {0};

    while (is_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                is_running = false;
            }
        }

        typedef struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        } RGBA_Color;

        RGBA_Color PURE_BLACK = (RGBA_Color){.r = 0, .g = 0, .b = 0, .a= 255};
        SDL_SetRenderDrawColor(renderer, PURE_BLACK.r, PURE_BLACK.g, PURE_BLACK.b, PURE_BLACK.a);
        SDL_RenderClear(renderer); // clears the renderer with the renderer draw color

        RGBA_Color PURE_WHITE = (RGBA_Color){.r = 255, .g = 255, .b = 255, .a= 255};
        SDL_SetRenderDrawColor(renderer, PURE_WHITE.r, PURE_WHITE.g, PURE_WHITE.b, PURE_WHITE.a);
        constexpr uint64_t text_scale = 2;
        SDL_SetRenderScale(renderer, text_scale, text_scale);
        constexpr uint64_t debugtext_charsize = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
        const char* debugtext_str = "This is pure white text...";
        uint64_t debugtext_strlen = bard_strlen(debugtext_str);
        float debugtext_x = ((float)((WINDOW_WIDTH)>>1))/text_scale - (float)((debugtext_strlen*debugtext_charsize)>>1);
        float debugtext_y = ((float)((WINDOW_HEIGHT)>>1))/text_scale;
        SDL_RenderDebugText(renderer, debugtext_x, debugtext_y, debugtext_str);

        SDL_SetRenderScale(renderer, 1.0f, 1.0f);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
