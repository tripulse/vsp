#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <argparse/argparse.h>

struct vsp_cli_options {
    size_t samplerate;
    size_t audio_bufsize;
    size_t sample_winsize;
    
    float init_max_volume;

    SDL_Color fore;
    SDL_Color back;

    unsigned init_width;
    unsigned init_height;

    bool list_devs;
    int cap_dev;
};

void vsp_cli_parse_SDL_Color(const char* str, SDL_Color* out) {
    int error = 0;
    switch(strlen(str)) {
        case 6:
            error = sscanf(str, "%2hhx%2hhx%2hhx", &out->r, &out->g, &out->b) < 3;
            out->a = 0xFF;  // considers as opaque.
        break;
        case 8:
            error = sscanf(str, "%2hhx%2hhx%2hhx%2hhx", &out->r, &out->g, &out->b, &out->a) < 4;
        break;
        default: error = 1;
        break;
    }
    if(error) {
        fputs("error: bad (back/fore)ground color", stderr);
        exit(1);
    }
}

void vsp_cli_parse_options(struct vsp_cli_options* opts, int argc, const char** argv) {
    const char *foreg = NULL,
               *backg = NULL;

    opts->cap_dev         = -1;
    opts->list_devs       = false;
    opts->samplerate      = 0; // let SDL choose.
    opts->sample_winsize  = 8192;
    opts->audio_bufsize   = 512;
    opts->init_max_volume = 1;
    opts->fore            = (SDL_Color){255, 255, 255, 255};
    opts->back            = (SDL_Color){  0,   0,   0,   0};
    opts->init_width      = 0;  // we will select.
    opts->init_height     = 0;  // ,,

    struct argparse_option cli_opts[] = {
        OPT_HELP(),
        OPT_GROUP("Audio Options"),
        OPT_INTEGER('d', "device", &opts->cap_dev, "index of the capture device", NULL, 0, 0),
        OPT_INTEGER('r', "rate", &opts->samplerate, "desired samplerate of audio", NULL, 0, 0),
        OPT_INTEGER('s', "size", &opts->sample_winsize, "number of samples to analyse at once", NULL, 0, 0),
        OPT_BOOLEAN('\0', "list-devices", &opts->list_devs, "list available devices", NULL, 0, 0),
        OPT_INTEGER('\0', "audio-bufsize", &opts->audio_bufsize, "size of the SDL audio buffer", NULL, 0, 0),
        OPT_FLOAT('\0', "max-volume", &opts->init_max_volume, "initial maximum displayed volume", NULL, 0, 0),

        OPT_GROUP("Display Options"),
        OPT_STRING('\0', "fg-color", &foreg, "foreground color of the visualisation window", NULL, 0, 0),
        OPT_STRING('\0', "bg-color", &backg, "background color of the visualisation window", NULL, 0, 0),

        OPT_INTEGER('\0', "width", &opts->init_width, "initial width of the visualisation window", NULL, 0, 0),
        OPT_INTEGER('\0', "height", &opts->init_height, "initial height of the visualisation window", NULL, 0, 0),
        OPT_END()
    };

    struct argparse parser;

    argparse_init(&parser, cli_opts, NULL, 0);
    argparse_parse(&parser, argc, argv);

    if(foreg) vsp_cli_parse_SDL_Color(foreg, &opts->fore);
    if(backg) vsp_cli_parse_SDL_Color(backg, &opts->back);

    if(!(opts->init_width || opts->init_height)) {
        SDL_Rect dispb;
        SDL_GetDisplayUsableBounds(0, &dispb);

        if(!opts->init_width)
            opts->init_width = dispb.w - (float)dispb.w * 0.15;
        if(!opts->init_height)
            opts->init_height = dispb.h - (float)dispb.h * 0.15;
    }

    if(!opts->sample_winsize) {
        fputs("error: bad sample window size", stderr);
        exit(1);
    }
}