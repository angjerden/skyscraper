/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SKY_COMPACT_H
#define SKY_COMPACT_H

#include "endian.h"
#include "struc.h"
#include "skydefs.h"
#include "types.h"
#include <iostream>

enum CptIds {
	CPT_JOEY = 1,
	CPT_FOSTER = 3,
	CPT_TEXT_1 = 0x17,
	CPT_TEXT_11 = 0x21,
	CPT_MENU_BAR = 0x2E,
	CPT_REICH_DOOR_20 = 0x30AB,
	CPT_MOVE_LIST = 0xBD,
	CPT_TALK_TABLE_LIST = 0xBC
};

enum CptTypeIds {
	CPT_NULL = 0,
	COMPACT,
	TURNTAB,
	ANIMSEQ,
	MISCBIN,
	GETTOTAB,
	ROUTEBUF,
	MAINLIST,
	NUM_CPT_TYPES
};

class SkyCompact {
public:
	SkyCompact(char* skyPath);
	~SkyCompact();
	Compact *fetchCpt(uint16 cptId);
	Compact *fetchCptInfo(uint16 cptId, uint16 *elems = NULL, uint16 *type = NULL, char *name = NULL, size_t nameSize = 0);
	static uint16 getSub(Compact *cpt, uint16 mode);
	static void setSub(Compact *cpt, uint16 mode, uint16 value);
	static MegaSet *getMegaSet(Compact *cpt);
	uint16 *getGrafixPtr(Compact *cpt);
	uint16 *getTurnTable(Compact *cpt, uint16 dir);
	void *getCompactElem(Compact *cpt, uint16 off);
	bool cptIsId(Compact *cpt, uint16 id);
	uint8	*createResetData(uint16 gameVersion);
	uint16	_numSaveIds;
	uint16	*_saveIds;
	// - debugging functions
	uint16 findCptId(void *cpt);
	uint16 findCptId(const char *cptName);
	uint16 giveNumDataLists();
	uint16 giveDataListLen(uint16 listNum);
	const char *nameForType(uint16 type);
	// skyscraper functions
	uint16 readUint16LE();
	uint32 readUint32LE();
	uint32 readRandom(void *ptr, uint32 len);
	void writeCompactsToFile();
	Compact* getCompactByIndexes(uint16 list, uint16 index);
	uint16 getCompactSize(uint16 list, uint16 index);
private:
	void checkAndFixOfficerBluntError();

	uint16  _numDataLists;
	uint16  *_dataListLen;
	uint16  *_rawBuf;
	char	*_asciiBuf;
	Compact ***_compacts;
	char    ***_cptNames;
	uint16	**_cptSizes;
	uint16  **_cptTypes;
	char* skyPath;
	std::string _cptFilename = "sky.cpt";
	std::FILE* _cptFile;
	uint32	_resetDataPos;
	static const char *const _typeNames[NUM_CPT_TYPES];
};

#endif
