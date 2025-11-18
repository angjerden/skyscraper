#include "types.h"
#include <iostream>

#ifndef SKY_WRITER_H
#define SKY_WRITER_H


namespace Writer {
    void writeWav(std::string fileName, uint8* speechData);
    // void writeBMP(const char* filename, uint8* image, uint8* palette);
    // void writeImage(std::string fileName, uint8* img, uint8* palette);
    // void convertPalette(uint8 *inPal, uint8* outPal);
};

#endif