/*
  Scraping assets from Beneath A Steel Sky v0.0372 CD/DOS
*/

#include "assert.h"
#include "struc.h"
#include "types.h"
#include "compact.h"
#include "disk.h"
#include "skydefs.h"
#include <stdio.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

int main(int argc, char **argv){
    char* skyPath = NULL;
    if (argc == 2) {
      skyPath = argv[1];
    }

    Disk* skyDisk = new Disk(skyPath);
    
    SkyCompact* skyCompact = new SkyCompact(skyPath);

    // Loop through each entry in sky.dsk
    // Store entry in some data structure
    // Write result to file in clear text
    return 0;
}