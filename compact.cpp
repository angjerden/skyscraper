#include "assert.h"
#include "compact.h"
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <fstream>

SkyCompact::SkyCompact(char* skyPath) {
    // Open compact file
    std::string cptFilePath(skyPath);
    cptFilePath.append(_cptFilename);
    _cptFile = fopen(cptFilePath.c_str(), "rb");
    uint16 compactFileVersion;
    std::fread(&compactFileVersion, 2, 1, _cptFile);
    if (compactFileVersion != 0){
      std::cout << "Unknown sky.cpt version" << std::endl;
    }

    // Get number of data lists in compact
    std::fread(&_numDataLists, 2, 1, _cptFile);

    std::cout << "Found " << _numDataLists << " compact data lists." << std::endl;

    // Set up compact data structures
    _cptNames = (char***)malloc(_numDataLists * sizeof(char**));
    _dataListLen = (uint16 *)malloc(_numDataLists * sizeof(uint16));
    _cptSizes	= (uint16 **)malloc(_numDataLists * sizeof(uint16 *));
    _cptTypes	= (uint16 **)malloc(_numDataLists * sizeof(uint16 *));
    _compacts	= (Compact***)malloc(_numDataLists * sizeof(Compact**));

    // do memory allocation for each data list
    for (int i = 0; i < _numDataLists; i++) {
      std::fread(&_dataListLen[i], 2, 1, _cptFile);
      _cptNames[i] = (char**)malloc(_dataListLen[i] * sizeof(char *));
      _cptSizes[i] = (uint16 *)malloc(_dataListLen[i] * sizeof(uint16));
      _cptTypes[i] = (uint16 *)malloc(_dataListLen[i] * sizeof(uint16));
      _compacts[i] = (Compact**)malloc(_dataListLen[i] * sizeof(Compact *));
    }

    uint32 rawNumInts;
    std::fread(&rawNumInts, 4, 1, _cptFile);
    uint32 rawSize = rawNumInts * sizeof(uint16);
    if (rawSize != 297206) {
      std::cout << "Wrong compact raw size." << std::endl;
    }
	  uint16 *rawPos = _rawBuf = (uint16 *)malloc(rawSize);

    uint32 srcSizeInts;
    std::fread(&srcSizeInts, 4, 1, _cptFile);
    uint32 srcSize = srcSizeInts * sizeof(uint16);
    if (srcSize != 310992) {
      std::cout << "Wrong compact source size." << std::endl;
    }
	  uint16 *srcBuf = (uint16 *)malloc(srcSize);
	  uint16 *srcPos = srcBuf;
    std::fread(srcBuf, srcSize, 1, _cptFile);

    uint32 asciiSize;
    std::fread(&asciiSize, 4, 1, _cptFile);
    if (asciiSize != 42395) {
      std::cout << "Wrong compact ascii size." << std::endl;
    }
    
    char *asciiPos = _asciiBuf = (char *)malloc(asciiSize);
    std::fread(_asciiBuf, asciiSize, 1, _cptFile);

    // fill up with compact data
    for (uint32 lcnt = 0; lcnt < _numDataLists; lcnt++) {
      for (uint32 ecnt = 0; ecnt < _dataListLen[lcnt]; ecnt++) {
        _cptSizes[lcnt][ecnt] = READ_LE_UINT16(srcPos++);
        if (_cptSizes[lcnt][ecnt]) {
          _cptTypes[lcnt][ecnt] = READ_LE_UINT16(srcPos++);
          _compacts[lcnt][ecnt] = (Compact *)rawPos;
          _cptNames[lcnt][ecnt] = asciiPos;
          asciiPos += std::strlen(asciiPos) + 1;

          for (uint16 elemCnt = 0; elemCnt < _cptSizes[lcnt][ecnt]; elemCnt++)
            *rawPos++ = READ_LE_UINT16(srcPos++);
        } else {
          _cptTypes[lcnt][ecnt] = 0;
          _compacts[lcnt][ecnt] = NULL;
          _cptNames[lcnt][ecnt] = NULL;
        }
      }
    }
    free(srcBuf);

    uint16 numDlincs;
    std::fread(&numDlincs, 2, 1, _cptFile);
    uint16 *dlincBuf = (uint16 *)malloc(numDlincs * 2 * sizeof(uint16));
    uint16 *dlincPos = dlincBuf;
    std::fread(dlincBuf, numDlincs * 2 * sizeof(uint16), 1, _cptFile);
    // these compacts don't actually exist but only point to other ones...
    uint16 cnt;
    for (cnt = 0; cnt < numDlincs; cnt++) {
      uint16 dlincId = READ_LE_UINT16(dlincPos++);
      uint16 destId = READ_LE_UINT16(dlincPos++);
      // assert(((dlincId >> 12) < _numDataLists) && ((dlincId & 0xFFF) < _dataListLen[dlincId >> 12]) && (_compacts[dlincId >> 12][dlincId & 0xFFF] == NULL));
      _compacts[dlincId >> 12][dlincId & 0xFFF] = _compacts[destId >> 12][destId & 0xFFF];

      // assert(_cptNames[dlincId >> 12][dlincId & 0xFFF] == NULL);
      _cptNames[dlincId >> 12][dlincId & 0xFFF] = asciiPos;
      asciiPos += strlen(asciiPos) + 1;
    }
    free(dlincBuf);

    // fseek instead of reading and throwing diff data?
    uint16 numDiffs;
    std::fread(&numDiffs, 2, 1, _cptFile);
    uint16 diffSize;
    std::fread(&diffSize, 2, 1, _cptFile);
    uint16 *diffBuf = (uint16 *)malloc(diffSize * sizeof(uint16));
    std::fread(diffBuf, diffSize * sizeof(uint16), 1, _cptFile);
    free(diffBuf);

    std::fread(&_numSaveIds, 2, 1, _cptFile);
    _saveIds = (uint16 *)malloc(_numSaveIds * sizeof(uint16));
    std::fread(_saveIds, _numSaveIds * sizeof(uint16), 1, _cptFile);
    _resetDataPos = std::ftell(_cptFile);

    checkAndFixOfficerBluntError();

}

SkyCompact::~SkyCompact() {
	free(_rawBuf);
	free(_asciiBuf);
	free(_saveIds);
	for (int i = 0; i < _numDataLists; i++) {
		free(_cptNames[i]);
		free(_cptSizes[i]);
		free(_cptTypes[i]);
		free(_compacts[i]);
	}
	free(_cptNames);
	free(_dataListLen);
	free(_cptSizes);
	free(_cptTypes);
	free(_compacts);
	fclose(_cptFile);
	delete _cptFile;
}


Compact* SkyCompact::fetchCpt(uint16 cptId) {
	return _compacts[cptId >> 12][cptId & 0xFFF];
}

/* WORKAROUND for bug #2687:
	The first release of scummvm with externalized, binary compact data has one broken 16 bit reference.
	When talking to Officer Blunt on ground level while in a crouched position, the game enters an
	unfinishable state because Blunt jumps into the lake and can no longer be interacted with.
	This fixes the problem when playing with a broken sky.cpt */
#define SCUMMVM_BROKEN_TALK_INDEX 158
void SkyCompact::checkAndFixOfficerBluntError() {
  // Retrieve the table with the animation ids to use for talking
  uint16 *talkTable = (uint16*)fetchCpt(CPT_TALK_TABLE_LIST);
  if (talkTable[SCUMMVM_BROKEN_TALK_INDEX] == ID_SC31_GUARD_TALK) {
    talkTable[SCUMMVM_BROKEN_TALK_INDEX] = ID_SC31_GUARD_TALK2;
  }
}