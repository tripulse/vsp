#include <string.h>

void vsp_push_sliding(float* window, size_t window_size,
                      float* data,   size_t data_size)
{
    if(window_size > data_size) {
        size_t window_offset = window_size - data_size;

        memmove(window, window + data_size, window_offset * sizeof(float));
        memcpy(window + window_offset, data, data_size * sizeof(float));
    } else if(data_size > window_size)
        memcpy(window, data + (data_size - window_size), window_size * sizeof(float));
    else
        memcpy(window, data, window_size * sizeof(float));  // window_size == data_size
}