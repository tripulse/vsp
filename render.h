#include <fftw3.h>
#include <SDL2/SDL.h>

#include "sliding.h"

struct vsp_render_context {
    fftwf_plan dft_calc;
    
    float *sample_win;
    size_t sample_winsize;

    float *dft_out;
    size_t dft_outsize;

    size_t samplerate;
    float  maxvol;

    SDL_Window   *win;
    SDL_Renderer *rend;
    SDL_Point    *points;

    SDL_Color fore;
    SDL_Color back;
};

void vsp_render_callback(struct vsp_render_context* ctx, Uint8* stream, int len) {
    vsp_push_sliding(ctx->sample_win, ctx->sample_winsize,
                     stream, len / sizeof(float));
    fftwf_execute(ctx->dft_calc);

    SDL_SetRenderDrawColor(ctx->rend,
                           ctx->back.r,
                           ctx->back.g,
                           ctx->back.b,
                           ctx->back.a);
    SDL_RenderClear(ctx->rend);

    int width, height;
    SDL_GetWindowSize(ctx->win, &width, &height);

    // 20 hZ as the hardcoded limit, should it be tinkered?
    const static float MIN_LOG_FREQUENCY = 1.3010299956639813f;

    // precalculations to save some CPU cycles, premature optimization?
    const float log_xs_pc0 = (width-1.f) / (log10f(ctx->samplerate / 2) - MIN_LOG_FREQUENCY);
    const float log_xs_pc1 = (float)ctx->samplerate / ctx->sample_winsize;

    const size_t imag_beg = ctx->sample_winsize - 1;

    for(size_t i = 0; i < ctx->dft_outsize; ++i) {
        float y = hypot(ctx->dft_out[imag_beg - i], ctx->dft_out[i]) / ctx->sample_winsize,
              f = i * log_xs_pc1;

        ctx->points[i] = (SDL_Point)
            {(int)((log10f(f) - MIN_LOG_FREQUENCY) * log_xs_pc0),
             (int)(y * -height/ctx->maxvol + height)};
    }

    SDL_SetRenderDrawColor(ctx->rend,
                           ctx->fore.r,
                           ctx->fore.g,
                           ctx->fore.b,
                           ctx->fore.a);
    SDL_RenderDrawLines(ctx->rend, ctx->points, ctx->dft_outsize);
    SDL_RenderPresent(ctx->rend);
}
