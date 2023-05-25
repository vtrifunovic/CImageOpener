# CImageOpener
Basic Image Processing engine written in C, used to be the CImageOpener.c file, but have now re-structured it to be much cleaner and better organized.

<div align="left">
    <img src="/photos/convolution.png" width="500px"</img> 
</div>

# Build
- STB_Image.h
- OpenGL
    - GLEW
- OpenCL
- GLFW

# Dependencies
- FFMPEG

# Features
- Processing
    - CPU & GPU Processing
- Rendering
    - Read & Display Images
    - Saving Images
    - Read & Display Videos
- Masks
    - RGB & Grayscale/single-channel mask
    - BitwiseAnd & BitwiseNot
- Conversions
    - Channel Conversions
        - RGB <-> HSV
        - RGB <-> YUV
        - RGB -> Gray
        - Inversion (255 - x)
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
        - Convolution
        - Gaussian Blur
        - Contour Detection & Analysis (CPU Only)
        - LUT & LUT Table Creation

# To-do Features
- Finish adding OpenCL functions for rest of current features
- Transformations

# To-do Fixes
- Add better debugging/error messages (more detail)
