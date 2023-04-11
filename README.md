# CImageOpener
Basic Image Processing engine written in C, used to be the CImageOpener.c file, but have now re-structured it to be much cleaner and better organized.

# Build
- STB_Image.h
- OpenGL
    - GLEW
- OpenCL
- GLFW
- FFMPEG

# Features
- Processing
    - CPU & GPU Processing (in progress)
- Rendering
    - Read & Display Images
    - Read & Display Videos
- Masks
    - RGB & Grayscale/single-channel mask
    - BitwiseAnd & BitwiseNot
- Conversions
    - RGB to HSV and back
    - RGB to Gray
    - Inversion
    - Resizing (Nearest Neighbor & Billinear)
- Binary
    - Creating kernel
    - Hit-Miss
    - Binary Dilation & Erosion
    - Thinning (Zhangsuen)
- Tools
    - Basic tools:
        - Blur
        - Channel split & Re-Merge
        - Adding & Subtracting Images
        - Crop/Zoom w/ fill & no fill
        - Dilation & Erosion
    - Filters:
        - Median Filter

# To-do Features
- Finish adding OpenCL functions for rest of current features

# To-do Fixes
- Add better debugging/error messages (more detail)
- Add & fix up resizing methods
- Create more dynamic kernels

