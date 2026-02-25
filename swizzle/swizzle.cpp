#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

enum SwizzleMode {
    Morton,
    Transpose,
    ColumnMajor,
    Scramble
};

enum ColorMode {
    None,
    ChannelSwap,
    BlockXor,
    BlockTint,
    ChannelRotate,
    BitCrush,    
    Grayscale,   
    Invert       
};

// helper for Morton curve
inline uint32_t expandBits(uint32_t v) {
    v &= 0x0000ffff;
    v = (v ^ (v << 8)) & 0x00ff00ff;
    v = (v ^ (v << 4)) & 0x0f0f0f0f;
    v = (v ^ (v << 2)) & 0x33333333;
    v = (v ^ (v << 1)) & 0x55555555;
    return v;
}

// computes the 2D Morton index from (x, y)
inline uint32_t morton2D(uint32_t x, uint32_t y) {
    return (expandBits(y) << 1) | expandBits(x);
}

inline uint32_t bitReverse(uint32_t n, int bits) {
    uint32_t res = 0;
    for (int i = 0; i < bits; ++i) {
        res = (res << 1) | (n & 1);
        n >>= 1;
    }
    return res;
}

// ---------------------------------------------------------
// Main Application
// ---------------------------------------------------------

int main(int argc, char** argv) {
    if (argc < 3 || argc > 6) {
        std::cerr << "Usage: " << argv[0] << " <input.png> <output.png> [swizzle_mode] [color_mode] [macrotile_size]\n";
        std::cerr << "Swizzle modes: morton (default), transpose, column_major, scramble\n";
        std::cerr << "Color modes:   none (default), channel_swap, block_xor, block_tint, channel_rotate, bit_crush, grayscale, invert\n";
        std::cerr << "Macrotile size: 16 (default). Must be a power of 2 (e.g., 4, 8, 16, 32)\n";
        return EXIT_FAILURE;
    }

    const std::string inputPath = argv[1];
    const std::string outputPath = argv[2];

    // Parse modes with defaults
    std::string swizzleStr = (argc >= 4) ? argv[3] : "morton";
    std::string colorStr = (argc >= 5) ? argv[4] : "none";

    // Parse Macrotile Size
    uint32_t macroBlockSize = 16;
    if (argc >= 6) {
        try {
            macroBlockSize = std::stoi(argv[5]);
        }
        catch (...) {
            std::cerr << "Error: Invalid macrotile_size.\n";
            return EXIT_FAILURE;
        }

        if (macroBlockSize < 2 || (macroBlockSize & (macroBlockSize - 1)) != 0) {
            std::cerr << "Error: macrotile_size must be a power of 2 (e.g., 4, 8, 16, 32).\n";
            return EXIT_FAILURE;
        }
    }

    int scrambleBits = 0;
    for (uint32_t temp = macroBlockSize; temp > 1; temp >>= 1) {
        scrambleBits++;
    }

    SwizzleMode swizzleMode = Morton;
    if (swizzleStr == "morton")       swizzleMode = Morton;
    else if (swizzleStr == "transpose") swizzleMode = Transpose;
    else if (swizzleStr == "column_major") swizzleMode = ColumnMajor;
    else if (swizzleStr == "scramble")  swizzleMode = Scramble;
    else {
        std::cerr << "Error: Unknown swizzle_mode '" << swizzleStr << "'\n";
        return EXIT_FAILURE;
    }

    ColorMode colorMode = None;
    if (colorStr == "none")             colorMode = None;
    else if (colorStr == "channel_swap") colorMode = ChannelSwap;
    else if (colorStr == "block_xor")    colorMode = BlockXor;
    else if (colorStr == "block_tint")   colorMode = BlockTint;
    else if (colorStr == "channel_rotate") colorMode = ChannelRotate;
    else if (colorStr == "bit_crush")    colorMode = BitCrush;
    else if (colorStr == "grayscale")    colorMode = Grayscale;
    else if (colorStr == "invert")       colorMode = Invert;
    else {
        std::cerr << "Error: Unknown color_mode '" << colorStr << "'\n";
        return EXIT_FAILURE;
    }

    // Load the image
    int width, height, channels;
    unsigned char* srcData = stbi_load(inputPath.c_str(), &width, &height, &channels, 4);
    if (!srcData) {
        std::cerr << "Error: Failed to load image '" << inputPath << "'\n";
        return EXIT_FAILURE;
    }

    if (width % 4 != 0 || height % 4 != 0) {
        std::cerr << "Error: Image dimensions must be multiples of 4.\n";
        stbi_image_free(srcData);
        return EXIT_FAILURE;
    }

    std::vector<uint8_t> dstData(width * height * 4, 0);

    const uint32_t blocksX = width / 4;
    const uint32_t blocksY = height / 4;

    for (uint32_t by = 0; by < blocksY; ++by) {
        for (uint32_t bx = 0; bx < blocksX; ++bx) {

            uint32_t macroX = bx / macroBlockSize;
            uint32_t macroY = by / macroBlockSize;
            uint32_t localX = bx % macroBlockSize;
            uint32_t localY = by % macroBlockSize;

            uint32_t swizzledLocalX = localX;
            uint32_t swizzledLocalY = localY;

            // Apply selected Swizzle Mode
            switch (swizzleMode) {
            case Morton: {
                uint32_t mortonIdx = morton2D(localX, localY);
                swizzledLocalX = mortonIdx % macroBlockSize;
                swizzledLocalY = mortonIdx / macroBlockSize;
                break;
            }
            case Transpose: {
                swizzledLocalX = localY;
                swizzledLocalY = localX;
                break;
            }
            case ColumnMajor: {
                uint32_t linearIdx = localX * macroBlockSize + localY;
                swizzledLocalX = linearIdx % macroBlockSize;
                swizzledLocalY = linearIdx / macroBlockSize;
                break;
            }
            case Scramble: {
                swizzledLocalX = bitReverse(localX, scrambleBits);
                swizzledLocalY = bitReverse(localY, scrambleBits);
                break;
            }
            }

            uint32_t dstBlockX = macroX * macroBlockSize + swizzledLocalX;
            uint32_t dstBlockY = macroY * macroBlockSize + swizzledLocalY;

            if (dstBlockX < blocksX && dstBlockY < blocksY) {
                for (uint32_t py = 0; py < 4; ++py) {
                    for (uint32_t px = 0; px < 4; ++px) {

                        uint32_t srcPxX = bx * 4 + px;
                        uint32_t srcPxY = by * 4 + py;
                        uint32_t dstPxX = dstBlockX * 4 + px;
                        uint32_t dstPxY = dstBlockY * 4 + py;

                        uint32_t srcIdx = (srcPxY * width + srcPxX) * 4;
                        uint32_t dstIdx = (dstPxY * width + dstPxX) * 4;

                        uint8_t r = srcData[srcIdx + 0];
                        uint8_t g = srcData[srcIdx + 1];
                        uint8_t b = srcData[srcIdx + 2];
                        uint8_t a = srcData[srcIdx + 3];

                        // Apply color corruption
                        switch (colorMode) {
                        case ChannelSwap: {
                            uint8_t tempR = r; r = b; b = g; g = tempR; break;
                        }
                        case BlockXor: {
                            r ^= (bx * 17) & 0xFF; g ^= (by * 29) & 0xFF; b ^= ((bx + by) * 13) & 0xFF; break;
                        }
                        case BlockTint: {
                            r = static_cast<uint8_t>(std::clamp<int>(r + (bx * 3), 0, 255));
                            g = static_cast<uint8_t>(std::clamp<int>(g + (by * 5), 0, 255));
                            b = static_cast<uint8_t>(std::clamp<int>(b + ((bx ^ by) * 7), 0, 255));
                            break;
                        }
                        case ChannelRotate: {
                            int rotation = (bx + by) % 3;
                            if (rotation == 1) { uint8_t tempR = r; r = g; g = b; b = tempR; }
                            else if (rotation == 2) { uint8_t tempR = r; r = b; b = g; g = tempR; }
                            break;
                        }
                        case BitCrush: {
                            // Mask out the lower 4 bits to simulate a 16-bit texture format
                            r &= 0xF0; g &= 0xF0; b &= 0xF0;
                            break;
                        }
                        case Grayscale: {
                            // Standard NTSC luminance conversion
                            uint8_t luma = (r * 77 + g * 150 + b * 29) >> 8;
                            r = luma; g = luma; b = luma;
                            a = 255;
                            break;
                        }
                        case Invert: {
                            // Bitwise inversion
                            r = ~r; g = ~g; b = ~g;
                            break;
                        }
                        case None: default: break;
                        }

                        dstData[dstIdx + 0] = r;
                        dstData[dstIdx + 1] = g;
                        dstData[dstIdx + 2] = b;
                        dstData[dstIdx + 3] = a;
                    }
                }
            }
        }
    }

    int stride = width * 4;
    if (!stbi_write_png(outputPath.c_str(), width, height, 4, dstData.data(), stride)) {
        std::cerr << "Error: Failed to save image to '" << outputPath << "'\n";
        stbi_image_free(srcData);
        return EXIT_FAILURE;
    }

    std::cout << "Success! Swizzle: '" << swizzleStr << "', Color: '" << colorStr
        << "', Size: " << macroBlockSize << ". Saved to: " << outputPath << "\n";

    stbi_image_free(srcData);
    return EXIT_SUCCESS;
}