#pragma once
#include <stdbool.h>
#include "opencl_support/gpu_setup.h"

typedef struct global {
    bool enable_gpu;
    char past_prog[50];
    char past_func[20];
    int totalsize;
    GPU_Values gpu_values;
} Global;

extern Global global;