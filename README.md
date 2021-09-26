# vsp
  
<img height=256 src='https://i.imgur.com/ouAMP2B.png'>

Simple, fast and lightweight spectrum visualiser made in C.
It is a successor of [vspectro][1] in terms of performance and features.
It grabs audio from a input device, analyses it through a sliding [DFT][2]
and renders a logarithmic frequency spectrum with linear magnitude which you
can study or just vibe to.

## Features
- Magnitude can be zoomed in with <kbd>↓</kbd> key and out with <kbd>↑</kbd> key.
- Logarithmic horizontal (frequency bins) and linear vertical scaling (magnitude).
- Arbitarary capture device selection.
- Changable background and foreground colors.

## Installation
### Prerequisites
- https://libsdl.org/ (>=2.0)
- http://www.fftw.org/
- https://mesonbuild.com/
```bash
# on debian (or debian based distributions)
$ apt install libsdl2-dev libfftw3-dev

# on gentoo/funtoo
$ emerge media-libs/libsdl2 sci-libs/fftw

# on msys2 (mingw-w64)
$ pacman -S mingw-w64-SDL2 mingw-w64-fftw
```

On Windows it is less painful to install [MSYS2](https://www.msys2.org/) then compile it.

### Compilation
Clone/download the repository and chdir into it ofcourse, then:

```
$ meson builddir -Doptimization=s -Dvsp:default_library=static
$ ninja -C builddir
```

It will produce an executable in `builddir`, which is the program.

## Usage

```
$ ./builddir/vsp --help
Usage:

    -h, --help                show this help message and exit

Audio Options
    -d, --device=<int>        index of the capture device
    -r, --rate=<int>          desired samplerate of audio
    -s, --size=<int>          number of samples to analyse at once
    --list-devices            list available devices
    --audio-bufsize=<int>     size of the SDL audio buffer
    --max-volume=<flt>        initial maximum displayed volume
    

Display Options
    --fg-color=<str>          foreground color of the visualisation window
    --bg-color=<str>          background color of the visualisation window
    --width=<int>             initial width of the visualisation window
    --height=<int>            initial height of the visualisation window
```

`--fg-color` and `--bg-color` is in the hex-color format (optionally prefixed with `#`).

`--list-devices` shows what devices are for capturing with their indices.
```
list of capture devices:
    [index]    [name]
         0      Mic in at front panel (Pink) (Realtek High Definition Audio)
         1      Stereo Mix (Realtek High Definition Audio)
```

If to choose a specific device than default, pass the any of the indices with `-d`.
If passed index is not in that list, it falls back to default.

## Technical details

- No windowing function is applied on the incoming samples, spectral leakage might occur.
- Frequency spectrum starts at 20hZ (TODO: limit to 20000hZ, max).

[1]: about:blank
[2]: https://en.wikipedia.org/wiki/Discrete_Fourier_transform
