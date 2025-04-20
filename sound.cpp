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

#include "sound.h"

#define SOUND_FILE_BASE 60203
#define MAX_FX_NUMBER 393
#define SFXF_START_DELAY 0x80
#define SFXF_SAVE 0x20

uint16 Sound::_speechConvertTable[8] = {
	0,									//;Text numbers to file numbers
	600,								//; 553 lines in section 0
	600+500,							//; 488 lines in section 1
	600+500+1330,						//;1303 lines in section 2
	600+500+1330+950,					//; 922 lines in section 3
	600+500+1330+950+1150,				//;1140 lines in section 4
	600+500+1330+950+1150+550,			//; 531 lines in section 5
	600+500+1330+950+1150+550+150,		//; 150 lines in section 6
};

Sound::Sound(Disk *pDisk) {
	_skyDisk = pDisk;

}

Sound::~Sound() {}

uint8* Sound::getSpeech(uint16 textNum) {
    uint16 speechFileNum = _speechConvertTable[textNum >> 12] + (textNum & 0xFFF);
    uint16 diskFileNum = speechFileNum + 50000;
	uint8* speechData = _skyDisk->loadFile(diskFileNum);

    if (!speechData) {
		// debug(9,"File %d (speechFile %d from section %d) wasn't found", speechFileNum + 50000, textNum & 0xFFF, textNum >> 12);
		std::cout << "File " << diskFileNum << " (speechFile " << (textNum & 0xFFF) << " from section " << (textNum >> 12) << ") wasn't found" << std::endl; 
		return NULL;
	}

    return speechData;
}
