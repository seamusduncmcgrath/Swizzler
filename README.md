# GPU Texture Swizzler

A C++17 command-line tool that intentionally "swizzles" normal images to mimic how console GPU textures (like those on the PS4) look in raw memory dumps. 

Unlike standard noise or randomized glitch art, this tool physically rearranges 4x4 pixel blocks into macrotiled Z-order (Morton) curves and simulates hardware-level texture format mismatches. The result is a highly structured, authentic digital mosaic aesthetic that preserves large-scale image recognition while creating chaotic local artifacts.



## Features
* **Accurate Block Swizzling:** Operates on 4x4 pixel blocks to mimic standard BCn texture compression layouts.
* **Macrotile Architecture:** Limits the Z-order curve to local 64x64 pixel chunks (configurable) to preserve overall image structure.
* **Format Mismatch Simulation:** Replicates the look of incorrect bit-depths, signed/unsigned errors, and wrong channel mappings.
* **Zero Dependencies:** Uses only standard C++17 and the lightweight `stb` single-file libraries.
* **Batch Automation:** Includes a drag-and-drop Windows `.bat` script to instantly generate 96 variations of an image.

## Building the Project

### Requirements
1. A C++17 compatible compiler (GCC, Clang, or MSVC).
2. Download `stb_image.h` and `stb_image_write.h` from the [stb repository](https://github.com/nothings/stb) and place them in the same directory as `swizzle.cpp`.

### Compilation

**Windows (MSVC via Developer Command Prompt):**
```cmd
cl /std:c++17 /O2 swizzle.cpp
