/*
  Scraping assets from Beneath A Steel Sky v0.0372 CD/DOS
*/

#include "assert.h"
#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <fstream>

static const char *const dinnerFilename = "../../sky/sky.dnr";

int main(int argc, char **argv){
    // Open file
    assert((dinnerFilename != nullptr));  // Pointless??
    std::FILE* fp = fopen(dinnerFilename, "rb");
    std::fseek(fp, 0, SEEK_SET);

    // Read no of entries
    uint32_t noEntries;
    std::fread(&noEntries, 1, 4, fp);

    std::cout << "Found " << noEntries << " dinner table entries." << std::endl;
    
    fclose(fp);

    // Loop through each entry in sky.dsk
    // Store entry in some data structure
    // Write result to file in clear text
    return 0;
}

