#include "types.h"
#include <iostream>

namespace Writer {
    void writeWav(std::string fileName, uint32 fileSize, uint8* speechData);
    void writeImage(std::string fileName, uint8* img, uint8* palette);
};