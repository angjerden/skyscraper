#include "disk.h"

Disk::Disk(char* skyPath) {
    std::string diskFilePath(skyPath);
    diskFilePath.append(_diskFilename);
	_dataDiskHandle = fopen(diskFilePath.c_str(), "rb");

    std::string dnrFilePath(skyPath);
    dnrFilePath.append(_dnrFilename);
    std::FILE* dnrFile = fopen(dnrFilePath.c_str(), "rb");
    std::fseek(dnrFile, 0, SEEK_SET);

    // Read no of entries
    uint32_t noEntries;
    std::fread(&noEntries, 4, 1, dnrFile);

    std::cout << "Found " << noEntries << " dinner table entries." << std::endl;
    
    // Read entries into dinnerTable
    uint8_t* dnrTable = (uint8_t*)malloc(noEntries * 8);
    std::fread(dnrTable, 8 * noEntries, 1, dnrFile);
	
    memset(_buildList, 0, 60 * 2);
	memset(_loadedFilesList, 0, 60 * 4);

    fclose(dnrFile);


}