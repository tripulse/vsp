#include "cli.h"
#include "render.h"

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_AudioDeviceID aud;
    SDL_AudioSpec audconf;

    struct vsp_cli_options clopts;
    struct vsp_render_context rndctx;

    vsp_cli_parse_options(&clopts, argc, argv);

    aud = SDL_OpenAudioDevice(NULL, 1,
                &(SDL_AudioSpec) {
                    .freq     = clopts.samplerate,
                    .format   = AUDIO_F32,
                    .channels = 1,
                    .samples  = clopts.audio_bufsize,
                    .callback = &vsp_render_callback,
                    .userdata = &rndctx,
                }, &audconf,
                SDL_AUDIO_ALLOW_SAMPLES_CHANGE |
                SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    
    if(aud < 2) {
        fprintf(stderr, "error: opening audio device: %s", SDL_GetError());
        exit(1);
    }

    rndctx.samplerate = audconf.freq;
    rndctx.maxvol = clopts.init_max_volume;

    rndctx.sample_winsize = clopts.sample_winsize;
    rndctx.dft_outsize = rndctx.sample_winsize / 2 + 1;
    rndctx.rects = malloc(rndctx.dft_outsize * sizeof(SDL_FRect));

    rndctx.sample_win = fftwf_alloc_real(rndctx.sample_winsize * sizeof(float));
    rndctx.dft_out = fftwf_alloc_real(rndctx.sample_winsize);
    rndctx.dft_calc = fftwf_plan_r2r_1d(rndctx.sample_winsize,
                                        rndctx.sample_win,
                                        rndctx.dft_out,

                                        FFTW_R2HC,
                                        FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
    if(rndctx.dft_calc == NULL) {
        fputs("error: fftw3 plan building failed", stderr);
        exit(1);
    }

    rndctx.fore = clopts.fore;
    rndctx.back = clopts.back;

    rndctx.win = SDL_CreateWindow(
            argv[0],
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            clopts.init_width,
            clopts.init_height,
            SDL_WINDOW_RESIZABLE);

    if(rndctx.win == NULL) {
        fprintf(stderr, "error: opening SDL window: %s", SDL_GetError());
        exit(1);
    }

    rndctx.rend = SDL_CreateRenderer(
            rndctx.win, -1,
            SDL_RENDERER_ACCELERATED);

    if(rndctx.rend == NULL) {
        fprintf(stderr, "error: opening SDL renderer: %s", SDL_GetError());
        exit(1);
    }

    SDL_PauseAudioDevice(aud, 0);

    // handles the close event and scales up/down the spectrum.
    //      Key Up:   Scales down
    //      Key Down: Scales up
    for(SDL_Event e; SDL_WaitEvent(&e);) {
        if(e.type == SDL_QUIT) break;
        else if(e.type == SDL_KEYDOWN)
            switch(e.key.keysym.sym) {
                case SDLK_DOWN: rndctx.maxvol *= 0.9f; break;
                case SDLK_UP:   rndctx.maxvol /= 0.9f; break;
            }
    }

    SDL_CloseAudioDevice(aud);
    SDL_DestroyWindow(rndctx.win);
    SDL_DestroyRenderer(rndctx.rend);
    SDL_Quit();

    fftwf_destroy_plan(rndctx.dft_calc);
    fftwf_free(rndctx.sample_win);
    fftwf_free(rndctx.dft_out);
    free(rndctx.rects);
}
