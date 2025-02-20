#include "types.h"
#include <iostream>

#ifndef SKY_WRITER_H
#define SKY_WRITER_H

#pragma pack(push, 1) // Ensure proper struct alignment for BMP format

struct BMPHeader {
    uint16_t bfType = 0x4D42;  // 'BM' identifier
    uint32_t bfSize;           // File size
    uint16_t bfReserved1 = 0;
    uint16_t bfReserved2 = 0;
    uint32_t bfOffBits;        // Offset to pixel data
};

struct DIBHeader {
    uint32_t biSize = 40;      // DIB header size
    int32_t biWidth = 320;     // Image width
    int32_t biHeight = 200;    // Image height (positive, bottom-up)
    uint16_t biPlanes = 1;
    uint16_t biBitCount = 8;   // 8-bit indexed color
    uint32_t biCompression = 0;
    uint32_t biSizeImage = 320 * 200; // Raw pixel data size
    int32_t biXPelsPerMeter = 2835;   // 72 DPI
    int32_t biYPelsPerMeter = 2835;
    uint32_t biClrUsed = 256;   // Number of colors in palette
    uint32_t biClrImportant = 256;
};
#pragma pack(pop)

namespace Writer {
    void writeWav(std::string fileName, uint32 fileSize, uint8* speechData);
    void writeBMP(const char* filename, uint8* image, uint8* palette);
    // void writeImage(std::string fileName, uint8* img, uint8* palette);
};

#endif