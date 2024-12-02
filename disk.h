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

#ifndef SKY_DISK_H
#define SKY_DISK_H

#include "struc.h"
#include "skydefs.h"
#include "types.h"
#include "rnc_deco.h"
#include <iostream>

#define MAX_FILES_IN_LIST 60


class Disk {
public:
	Disk(char* skyPath);
	~Disk();

	uint8 *loadFile(uint16 fileNr);
	uint16 *loadScriptFile(uint16 fileNr);
	bool fileExists(uint16 fileNr);

	uint32 determineGameVersion();

	uint32 _lastLoadedFileSize;

	void fnMiniLoad(uint16 fileNum);
	void fnCacheFast(uint16 *fList);
	void fnCacheChip(uint16 *fList);
	void fnCacheFiles();
	void fnFlushBuffers();
	uint32 *giveLoadedFilesList() { return _loadedFilesList; }
	void refreshFilesList(uint32 *list);
	
	// From sky.h
	static void *fetchItem(uint32 num);
	static void *_itemList[300];

protected:
	uint8 *getFileInfo(uint16 fileNr);
	void dumpFile(uint16 fileNr);

	uint32 _dinnerTableEntries;
	uint8 *_dinnerTableArea;
	// TODO: Replace std::FILE with std::ifstream?
	std::FILE *_dataDiskHandle;
	RncDecoder _rncDecoder;

	uint16 _buildList[MAX_FILES_IN_LIST];
	uint32 _loadedFilesList[MAX_FILES_IN_LIST];

    std::string _dnrFilename = "sky.dnr";
    std::string _diskFilename = "sky.dsk";

	void loadFixedItems();
};

#endif
