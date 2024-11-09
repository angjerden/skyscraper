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
static const char* const compactFilename = "../../sky/sky.cpt";
static const char* const diskFilename = "../../sky/sky.dsk";

Compact* fetchCpt(uint16 cptId, Compact*** compacts) {
	return compacts[cptId >> 12][cptId & 0xFFF];
}

/* WORKAROUND for bug #2687:
	The first release of scummvm with externalized, binary compact data has one broken 16 bit reference.
	When talking to Officer Blunt on ground level while in a crouched position, the game enters an
	unfinishable state because Blunt jumps into the lake and can no longer be interacted with.
	This fixes the problem when playing with a broken sky.cpt */
#define SCUMMVM_BROKEN_TALK_INDEX 158
void checkAndFixOfficerBluntError(Compact*** compacts) {
  // Retrieve the table with the animation ids to use for talking
  uint16 *talkTable = (uint16*)fetchCpt(CPT_TALK_TABLE_LIST, compacts);
  if (talkTable[SCUMMVM_BROKEN_TALK_INDEX] == ID_SC31_GUARD_TALK) {
    talkTable[SCUMMVM_BROKEN_TALK_INDEX] = ID_SC31_GUARD_TALK2;
  }
}

int main(int argc, char **argv){
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

    // Open compact file
    std::FILE* compactFile = fopen(compactFilename, "rb");
    uint16_t compactFileVersion;
    std::fread(&compactFileVersion, 2, 1, compactFile);
    if (compactFileVersion != 0){
      std::cout << "Unknown sky.cpt version" << std::endl;
    }

    // Get number of data lists in compact
    uint16_t numDataLists;
    std::fread(&numDataLists, 2, 1, compactFile);

    std::cout << "Found " << numDataLists << " compact data lists." << std::endl;

    // Set up compact data structures
    char*** cptNames = (char***)malloc(numDataLists * sizeof(char**));
    uint16* dataListLen = (uint16 *)malloc(numDataLists * sizeof(uint16));
    uint16** cptSizes	= (uint16 **)malloc(numDataLists * sizeof(uint16 *));
    uint16** cptTypes	= (uint16 **)malloc(numDataLists * sizeof(uint16 *));
    Compact*** compacts	= (Compact***)malloc(numDataLists * sizeof(Compact**));

    // do memory allocation for each data list
    for (int i = 0; i < numDataLists; i++) {
      std::fread(&dataListLen[i], 2, 1, compactFile);
      cptNames[i] = (char**)malloc(dataListLen[i] * sizeof(char *));
      cptSizes[i] = (uint16 *)malloc(dataListLen[i] * sizeof(uint16));
      cptTypes[i] = (uint16 *)malloc(dataListLen[i] * sizeof(uint16));
      compacts[i] = (Compact**)malloc(dataListLen[i] * sizeof(Compact *));
    }

    uint32 rawNumInts;
    std::fread(&rawNumInts, 4, 1, compactFile);
    uint32 rawSize = rawNumInts * sizeof(uint16);
    if (rawSize != 297206) {
      std::cout << "Wrong compact raw size." << std::endl;
    }
	  uint16 *rawPos = (uint16 *)malloc(rawSize);

    uint32 srcSizeInts;
    std::fread(&srcSizeInts, 4, 1, compactFile);
    uint32 srcSize = srcSizeInts * sizeof(uint16);
    if (srcSize != 310992) {
      std::cout << "Wrong compact source size." << std::endl;
    }
	  uint16 *srcBuf = (uint16 *)malloc(srcSize);
	  uint16 *srcPos = srcBuf;
	  
    std::fread(srcBuf, srcSize, 1, compactFile);

    uint32 asciiSize;
    std::fread(&asciiSize, 4, 1, compactFile);
    if (asciiSize != 42395) {
      std::cout << "Wrong compact ascii size." << std::endl;
    }
    char* asciiBuf; // Not sure this is necessary
    char *asciiPos = asciiBuf = (char *)malloc(asciiSize);
    std::fread(asciiBuf, asciiSize, 1, compactFile);

    // fill up with compact data
    for (uint32 lcnt = 0; lcnt < numDataLists; lcnt++) {
      for (uint32 ecnt = 0; ecnt < dataListLen[lcnt]; ecnt++) {
        cptSizes[lcnt][ecnt] = *srcPos++;
        if (cptSizes[lcnt][ecnt]) {
          cptTypes[lcnt][ecnt] = *srcPos++;
          compacts[lcnt][ecnt] = (Compact *)rawPos;
          cptNames[lcnt][ecnt] = asciiPos;
          asciiPos += std::strlen(asciiPos) + 1;

          for (uint16 elemCnt = 0; elemCnt < cptSizes[lcnt][ecnt]; elemCnt++)
            *rawPos++ = *srcPos++;
        } else {
          cptTypes[lcnt][ecnt] = 0;
          compacts[lcnt][ecnt] = NULL;
          cptNames[lcnt][ecnt] = NULL;
        }
      }
    }
    free(srcBuf);

    uint16 numDlincs;
    std::fread(&numDlincs, 2, 1, compactFile);
    uint16 *dlincBuf = (uint16 *)malloc(numDlincs * 2 * sizeof(uint16));
    uint16 *dlincPos = dlincBuf;
    std::fread(dlincBuf, numDlincs * 2 * sizeof(uint16), 1, compactFile);
    // these compacts don't actually exist but only point to other ones...
    uint16 cnt;
    for (cnt = 0; cnt < numDlincs; cnt++) {
      uint16 dlincId = *dlincPos++;
      uint16 destId = *dlincPos++;
      // assert(((dlincId >> 12) < numDataLists) && ((dlincId & 0xFFF) < dataListLen[dlincId >> 12]) && (compacts[dlincId >> 12][dlincId & 0xFFF] == NULL));
      compacts[dlincId >> 12][dlincId & 0xFFF] = compacts[destId >> 12][destId & 0xFFF];

      // assert(cptNames[dlincId >> 12][dlincId & 0xFFF] == NULL);
      cptNames[dlincId >> 12][dlincId & 0xFFF] = asciiPos;
      asciiPos += strlen(asciiPos) + 1;
    }
    free(dlincBuf);

    // fseek instead of reading and throwing diff data?
    uint16 numDiffs;
    std::fread(&numDiffs, 2, 1, compactFile);
    uint16 diffSize;
    std::fread(&diffSize, 2, 1, compactFile);
    uint16 *diffBuf = (uint16 *)malloc(diffSize * sizeof(uint16));
    std::fread(diffBuf, diffSize * sizeof(uint16), 1, compactFile);
    free(diffBuf);

    uint16 numSaveIds;
    std::fread(&numSaveIds, 2, 1, compactFile);
    uint16* saveIds = (uint16 *)malloc(numSaveIds * sizeof(uint16));
    std::fread(saveIds, numSaveIds * sizeof(uint16), 1, compactFile);
    uint32 resetDataPos = std::ftell(compactFile);

    // TODO: Officer Blunt error
	  checkAndFixOfficerBluntError(compacts);

    // Loop through each entry in sky.dsk
    // Store entry in some data structure
    // Write result to file in clear text
    return 0;
}