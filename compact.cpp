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
    uint16 fileVersion = readUint16LE();
    if (fileVersion != 0){
      std::cout << "Unknown sky.cpt version" << std::endl;
    }

    // Get number of data lists in compact
    _numDataLists = readUint16LE();

    std::cout << "Found " << _numDataLists << " compact data lists." << std::endl;

    // Set up compact data structures
    _cptNames = (char***)malloc(_numDataLists * sizeof(char**));
    _dataListLen = (uint16 *)malloc(_numDataLists * sizeof(uint16));
    _cptSizes	= (uint16 **)malloc(_numDataLists * sizeof(uint16 *));
    _cptTypes	= (uint16 **)malloc(_numDataLists * sizeof(uint16 *));
    _compacts	= (Compact***)malloc(_numDataLists * sizeof(Compact**));

    // do memory allocation for each data list
    for (int i = 0; i < _numDataLists; i++) {
      _dataListLen[i] = readUint16LE();
      _cptNames[i] = (char**)malloc(_dataListLen[i] * sizeof(char *));
      _cptSizes[i] = (uint16 *)malloc(_dataListLen[i] * sizeof(uint16));
      _cptTypes[i] = (uint16 *)malloc(_dataListLen[i] * sizeof(uint16));
      _compacts[i] = (Compact**)malloc(_dataListLen[i] * sizeof(Compact *));
    }

    uint32 rawSize = readUint32LE() * sizeof(uint16);
    if (rawSize != 297206) {
      std::cout << "Wrong compact raw size." << std::endl;
    }
	  uint16 *rawPos = _rawBuf = (uint16 *)malloc(rawSize);

    uint32 srcSize = readUint32LE() * sizeof(uint16);
    if (srcSize != 310992) {
      std::cout << "Wrong compact source size." << std::endl;
    }
	  uint16 *srcBuf = (uint16 *)malloc(srcSize);
	  uint16 *srcPos = srcBuf;
    readRandom(srcBuf, srcSize);

    uint32 asciiSize = readUint32LE();
    if (asciiSize != 42395) {
      std::cout << "Wrong compact ascii size." << std::endl;
    }
    
    char *asciiPos = _asciiBuf = (char *)malloc(asciiSize);
    readRandom(_asciiBuf, asciiSize);
    
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

    uint16 numDlincs = readUint16LE();
    uint16 *dlincBuf = (uint16 *)malloc(numDlincs * 2 * sizeof(uint16));
    uint16 *dlincPos = dlincBuf;
    readRandom(dlincBuf, numDlincs * 2 * sizeof(uint16));
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
    uint16 numDiffs = readUint16LE();
    uint16 diffSize = readUint16LE();
    uint16 *diffBuf = (uint16 *)malloc(diffSize * sizeof(uint16));
    readRandom(diffBuf, diffSize * sizeof(uint16));
    // not bothering checking for game version 288
    free(diffBuf);

    _numSaveIds = readUint16LE();
    _saveIds = (uint16 *)malloc(_numSaveIds * sizeof(uint16));
    readRandom(_saveIds, _numSaveIds * sizeof(uint16));
    // Don't think it's necessary to convert to LE_16
    // for (cnt = 0; cnt < _numSaveIds; cnt++)
    //   _saveIds[cnt] = FROM_LE_16(_saveIds[cnt]);
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

uint16 SkyCompact::readUint16LE() {
  uint16 value;
  std::fread(&value, 1, 2, _cptFile);
  return value;
}

uint32 SkyCompact::readUint32LE() {
  uint32 value;
  std:fread(&value, 1, 4, _cptFile);
  return value;
}

uint32 SkyCompact::readRandom(void *ptr, uint32 len) {
  return std::fread(ptr, 1, len, _cptFile);
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

const char *SkyCompact::nameForType(uint16 type) {
	if (type >= NUM_CPT_TYPES)
		return "unknown";
	else
		return _typeNames[type];
}

Compact* SkyCompact::fetchCpt(uint16 cptId) {

  char* cptName = _cptNames[cptId >> 12][cptId & 0xFFF];
  const char* typeName = nameForType(_cptTypes[cptId >> 12][cptId & 0xFFF]);
  std::cout << "Loading Compact " << *cptName << " [" << *typeName << "] (" << cptId << "=" << (cptId >> 12) << "," << (cptId & 0xFFF) << ")" << std::endl;
	return _compacts[cptId >> 12][cptId & 0xFFF];
}

const char *const SkyCompact::_typeNames[NUM_CPT_TYPES] = {
	"null",
	"COMPACT",
	"TURNTABLE",
	"ANIM SEQ",
	"UNKNOWN",
	"GETTOTABLE",
	"AR BUFFER",
	"MAIN LIST"
};

void SkyCompact::writeCompactsToFile() {
  std::ofstream outfile;
  outfile.open("compacts.txt");

  for (uint16 i = 0; i < _numDataLists; i++) {
    std::cout << "Writing compacts for list " << i << std::endl;
    for (uint16 j = 0; j < _dataListLen[i]; j++) {
      uint16 cptType = _cptTypes[i][j];
      const char* cptName = _cptNames[i][j];
      const char* typeName = _typeNames[cptType];
      uint16 cptSize = _cptSizes[i][j]; 
      Compact* compact = _compacts[i][j];
      if (cptSize == 0) continue;
      outfile << "Compact " << i << " " << j << " [" << typeName << "] (" << cptName << ") " << "Size " << cptSize << std::endl;
      outfile << "Logic " << compact->logic << std::endl;
      outfile << "Status " << compact->status << std::endl;
      outfile << "Sync " << compact->sync << std::endl;
      outfile << "Screen " << compact->screen << std::endl;
      outfile << "Place " << compact->place << std::endl;
      outfile << "GetToTableId " << compact->getToTableId << std::endl;
      outfile << "Xcood " << compact->xcood << std::endl;
      outfile << "Ycood " << compact->ycood << std::endl;
      outfile << "Frame " << compact->frame << std::endl;
      outfile << "CursorText " << compact->cursorText << std::endl;
      outfile << "MouseOn " << compact->mouseOn << std::endl;
      outfile << "MouseOff " << compact->mouseOff << std::endl;
      outfile << "MouseClick " << compact->mouseClick << std::endl;
      outfile << "MouseRelX " << compact->mouseRelX << std::endl;
      outfile << "MouseRelY " << compact->mouseRelY << std::endl;
      outfile << "MouseSizeX " << compact->mouseSizeX << std::endl;
      outfile << "MouseSizeY " << compact->mouseSizeY << std::endl;
      outfile << "ActionScript " << compact->actionScript << std::endl;
      outfile << "UpFlag " << compact->upFlag << std::endl;
      outfile << "DownFlag " << compact->downFlag << std::endl;
      outfile << "GetToFlag " << compact->getToFlag << std::endl;
      outfile << "Flag " << compact->flag << std::endl;
      outfile << "Mood " << compact->mood << std::endl;
      outfile << "GrafixProgId " << compact->grafixProgId << std::endl;
      outfile << "GrafixProgPos " << compact->grafixProgPos << std::endl;
      outfile << "Offset " << compact->offset << std::endl;
      outfile << "Mode " << compact->mode << std::endl;
      outfile << "BaseSub " << compact->baseSub << std::endl;
      outfile << "BaseSub_Off " << compact->baseSub_off << std::endl;
      outfile << "ActionSub " << compact->actionSub << std::endl;
      outfile << "ActionSub_Off " << compact->actionSub_off << std::endl;
      outfile << "GetToSub " << compact->getToSub << std::endl;
      outfile << "GetToSub_Off " << compact->getToSub_off << std::endl;
      outfile << "ExtraSub " << compact->extraSub << std::endl;
      outfile << "ExtraSub_Off " << compact->extraSub_off << std::endl;
      outfile << "Dir " << compact->dir << std::endl;
      outfile << "StopScript " << compact->stopScript << std::endl;
      outfile << "MiniBump " << compact->miniBump << std::endl;
      outfile << "Leaving " << compact->leaving << std::endl;
      outfile << "AtWatch " << compact->atWatch << std::endl;
      outfile << "AtWas " << compact->atWas << std::endl;
      outfile << "Alt " << compact->alt << std::endl;
      outfile << "Request " << compact->request << std::endl;
      outfile << "SpWidth_xx " << compact->spWidth_xx << std::endl;
      outfile << "SpColor " << compact->spColor << std::endl;
      outfile << "SpTextId " << compact->spTextId << std::endl;
      outfile << "SpTime " << compact->spTime << std::endl;
      outfile << "ArAnimIndex " << compact->arAnimIndex << std::endl;
      outfile << "TurnProgId " << compact->turnProgId << std::endl;
      outfile << "TurnProgPos " << compact->turnProgPos << std::endl;
      outfile << "WaitingFor " << compact->waitingFor << std::endl;
      outfile << "ArTargetX " << compact->arTargetX << std::endl;
      outfile << "ArTargetY " << compact->arTargetY << std::endl;
      outfile << "AnimScratchId " << compact->animScratchId << std::endl;
      outfile << "MegaSet " << compact->megaSet << std::endl;
      // outfile << "MegaSet0 " << compact->megaSet0 << std::endl;
      // outfile << "MegaSet1 " << compact->megaSet1 << std::endl;
      // outfile << "MegaSet2 " << compact->megaSet2 << std::endl;
      // outfile << "MegaSet3 " << compact->megaSet3 << std::endl;
      outfile << std::endl;
      // outfile << "Compact " << cptName << " [" << typeName << "] (" << cptId << "=" << (cptId >> 12) << "," << (cptId & 0xFFF) << ")" << std::endl;
    }
  }

  outfile.close();

}