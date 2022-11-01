# CImageOpener
Basic Image Processing engine written in C, used to be the CImageOpener.c file, but have now re-structured it to be much cleaner and better organized.

# Build
- STB_Image.h
- OpenGL
- GLFW

# Features
## Rendering
- Read & Display Images
## Masks
- RGB & Grayscale/single-channel mask
- BitwiseAnd & BitwiseNot
## Conversions
- RGB to HSV and back
- Inversion
- Resizing (Nearest Neighbor)
## Binary
- Creating kernel
- Hit-Miss
- Binary Dilation & Erosion
## Tools
- Blur
- Channel split & Re-Merge

# To-do Features
- Crop/Zoom
- Grayscale Dilation
- Create contrast-stretching method
- Image convolution??

# To-do Fixes
- Add better debugging/error messages (more detail)
- Add & fix up resizing methods
- Find better way to free splits
- Create more dynamic kernels

