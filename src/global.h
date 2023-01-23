#pragma once
#include <stdbool.h>
#include "opencl_support/gpu_setup.h"

typedef struct global {
    bool enable_gpu;
    GPU_Values gpu_values;
} Global;

extern Global global;