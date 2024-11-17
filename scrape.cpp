/*
  Scraping assets from Beneath A Steel Sky v0.0372 CD/DOS
*/

#include "assert.h"
#include "struc.h"
#include "types.h"
#include "compact.h"
#include "skydefs.h"
#include <stdio.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

static const char* const dinnerFilename = "../../sky/sky.dnr";
// static const char* const compactFilename = "../../sky/sky.cpt";
// static const char* const diskFilename = "../../sky/sky.dsk";

// TODO: Send Skypath as an argument
// 
int main(int argc, char **argv){
    char* skyPath = nullptr;
    if (argc == 2) {
      skyPath = argv[1];
    }

    std::cout << argv[1] << std::endl;
    // Open dinner file
    assert((dinnerFilename != nullptr));  // Pointless??
    std::FILE* dinnerFile = fopen(dinnerFilename, "rb");
    std::fseek(dinnerFile, 0, SEEK_SET);

    // Read no of entries
    uint32_t noEntries;
    std::fread(&noEntries, 4, 1, dinnerFile);

    std::cout << "Found " << noEntries << " dinner table entries." << std::endl;
    

    // Read entries into dinnerTable
    uint8_t* dinnerTable = (uint8_t*)malloc(noEntries * 8);
    std::fread(dinnerTable, 8 * noEntries, 1, dinnerFile);

    fclose(dinnerFile);

    SkyCompact* skyCompact = new SkyCompact(skyPath);
    // Loop through each entry in sky.dsk
    // Store entry in some data structure
    // Write result to file in clear text
    return 0;
}