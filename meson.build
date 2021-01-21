project('vsp', 'c')

executable('vsp', 'vsp.c', dependencies : [
    dependency('sdl2'),
    dependency('fftw3f'),
    declare_dependency(
        link_with : static_library('argparse', 'argparse/argparse.c'),
        include_directories : include_directories('argparse')),
    meson.get_compiler('c').find_library('m', required : false)
])