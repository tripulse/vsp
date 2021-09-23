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
    SDL_FRect    *rects;

    SDL_Color fore;
    SDL_Color back;
};

void vsp_render_callback(struct vsp_render_context* ctx, Uint8* stream, int len) {
    vsp_push_sliding(ctx->sample_win, ctx->sample_winsize, stream, len / sizeof(float));
    fftwf_execute(ctx->dft_calc);

    SDL_SetRenderDrawColor(ctx->rend, ctx->back.r, ctx->back.g, ctx->back.b, ctx->back.a);
    SDL_RenderClear(ctx->rend);

    int width, height;
    SDL_GetWindowSize(ctx->win, &width, &height);

    // 20 hZ as the hardcoded limit, should it be tinkered?
    const static float LOG_MIN = 1.3010299956639813f;

    // precalculations to save some CPU cycles, premature optimization?
    const float pc0_ = (width-1.f) / (log10f(ctx->samplerate / 2) - LOG_MIN);
    const float pc1_ = (float)ctx->samplerate / (ctx->sample_winsize - 1);

    // beginning of the imaginary portion in the R2HC output.
    const size_t imag_beg = ctx->sample_winsize - 1;

    float x0 = 0;
    for(size_t i = 1; i < ctx->dft_outsize; ++i) {
        float y = hypotf(ctx->dft_out[imag_beg - i], ctx->dft_out[i]),
              x1 = (log10f(i * pc1_) - LOG_MIN) * pc0_;

        y /= ctx->sample_winsize;  // 1/N
        y *= height/ctx->maxvol; // scale to screen

        ctx->rects[i] = (SDL_FRect){x0, height - y, x1 - x0, y};
        x0 = x1;
    }

    SDL_SetRenderDrawColor(ctx->rend, ctx->fore.r, ctx->fore.g, ctx->fore.b, ctx->fore.a);
    SDL_RenderFillRectsF(ctx->rend, ctx->rects, ctx->dft_outsize);
    SDL_RenderPresent(ctx->rend);
}
