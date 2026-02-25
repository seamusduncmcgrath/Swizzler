# GPU Texture Swizzler

A C++ command-line tool that intentionally "swizzles" normal images to mimic how console GPU textures (like those on the PS4) look in raw memory dumps. 

Unlike standard noise or randomized glitch art, this tool physically rearranges 4x4 pixel blocks into macrotiled Z-order (Morton) curves and simulates hardware-level texture format mismatches. The result is a highly structured, authentic digital mosaic aesthetic that preserves large-scale image recognition while creating chaotic local artifacts.

## Features
* **Accurate Block Swizzling:** Operates on 4x4 pixel blocks to mimic standard BCn texture compression layouts.
* **Macrotile Architecture:** Limits the Z-order curve to local 64x64 pixel chunks (configurable) to preserve overall image structure.
* **Format Mismatch Simulation:** Replicates the look of incorrect bit-depths, signed/unsigned errors, and wrong channel mappings.
* **Zero Dependencies:** Uses only standard C++17 and the lightweight `stb` single-file libraries.
* **Batch Automation:** Includes a drag-and-drop Windows `.bat` script to instantly generate 96 variations of an image.

## Usage

swizzle <input.png> <output.png> [swizzle_mode] [color_mode] [macrotile_size]
(Note: Input images must have widths and heights that are multiples of 4).

1. Swizzle Modes (Physical Memory Layout)
Controls how the 4x4 blocks are physically rearranged inside the macrotile.

morton (default): Interleaves X and Y coordinates to create a hardware-accurate 2D Z-order curve.

transpose: Flips X and Y coordinates, reading row-data as column-data.

column_major: Reads blocks top-to-bottom instead of left-to-right, creating vertical striping.

scramble: Performs a bitwise reversal on the local coordinates, simulating a severe hardware cache miss.

2. Color Modes (Data Mismatch)
Applies a secondary pass of per-block or per-pixel corruption.

none (default): No color corruption.

channel_swap: Swaps R, G, and B channels (e.g., misreading RGBA as BGRA).

block_xor: Applies a deterministic bitwise XOR based on the block's coordinates.

block_tint: Adds an organic, clamped color tint based on spatial block coordinates.

channel_rotate: Rotates RGB channels locally depending on the block position.

bit_crush: Strips lower data bits, simulating reading a 16-bit texture (like RGB565) as 32-bit.

grayscale: Forces NTSC luminance, mimicking reading a 1-channel alpha/roughness mask as RGBA.

invert: Performs a bitwise NOT operation on the color channels.

3. Macrotile Size
Controls the scale of the localized glitch chunks. Must be a power of 2.

16 (default): Simulates 64x64 pixel memory pages (16 blocks × 4 pixels).

8: Tighter, older hardware look (32x32 pixels).

32: Massive, modern page-boundary look (128x128 pixels).

Example Commands
Generate a classic console memory dump look:

Bash
swizzle input.png output.png morton none 16
Simulate a severe cache miss combined with a 16-bit format error, using large tiles:

Bash
swizzle input.png output.png scramble bit_crush 32
🎞️ Batch Processing (Windows)
Don't want to type commands all day? Use the included generate_all.bat!

Drag and drop any valid input.png onto the generate_all.bat file.

It will automatically create a swizzle_results folder.

It generates 96 unique variations combining every size, swizzle, and color mode in seconds.

Building the Project
Requirements
A C++17 compatible compiler (GCC, Clang, or MSVC).

Download stb_image.h and stb_image_write.h from the stb repository and place them in the same directory as swizzle.cpp.

Compilation
Windows (MSVC via Developer Command Prompt):

DOS
cl /std:c++17 /O2 swizzle.cpp
Linux / Mac (GCC / Clang):

Bash
g++ -std=c++17 -O3 swizzle.cpp -o swizzle
