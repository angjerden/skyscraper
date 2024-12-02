#include "types.h"
#include <iostream>

namespace Writer {
    void writeWav(std::string fileName, uint32 fileSize, uint8* speechData);
};