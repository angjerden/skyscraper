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


// #include "common/endian.h"
// #include "common/events.h"
// #include "common/system.h"
// #include "common/textconsole.h"

// #include "graphics/paletteman.h"

#include "disk.h"
#include "logic.h"
#include "screen.h"
#include "compact.h"
// #include "sky.h"
#include "skydefs.h"
#include "struc.h"

#include <fstream>

uint8 Screen::_top16Colors[16*3] = {
	0, 0, 0,
	38, 38, 38,
	63, 63, 63,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	54, 54, 54,
	45, 47, 49,
	32, 31, 41,
	29, 23, 37,
	23, 18, 30,
	49, 11, 11,
	39, 5, 5,
	29, 1, 1,
	63, 63, 63
};

// Screen::Screen(OSystem *pSystem, Disk *pDisk, SkyCompact *skyCompact) {
Screen::Screen(Disk *pDisk, SkyCompact *skyCompact) {
	// _system = pSystem;
	_skyDisk = pDisk;
	_skyCompact = skyCompact;

	int i;
	uint8 tmpPal[VGA_COLORS * 3];

	_gameGrid = (uint8 *)malloc(GRID_X * GRID_Y * 2);
	// forceRefresh();

	_currentScreen = NULL;
	_scrollScreen = NULL;

	//blank the first 240 colors of the palette
	memset(tmpPal, 0, GAME_COLORS * 3);

	//set the remaining colors
	for (i = 0; i < (VGA_COLORS-GAME_COLORS); i++) {
		tmpPal[3 * GAME_COLORS + i * 3 + 0] = (_top16Colors[i * 3 + 0] << 2) + (_top16Colors[i * 3 + 0] >> 4);
		tmpPal[3 * GAME_COLORS + i * 3 + 1] = (_top16Colors[i * 3 + 1] << 2) + (_top16Colors[i * 3 + 1] >> 4);
		tmpPal[3 * GAME_COLORS + i * 3 + 2] = (_top16Colors[i * 3 + 2] << 2) + (_top16Colors[i * 3 + 2] >> 4);
	}

	//set the palette
	// _system->getPaletteManager()->setPalette(tmpPal, 0, VGA_COLORS);
	_currentPalette = 0;

	// _seqInfo.nextFrame = _seqInfo.framesLeft = 0;
	// _seqInfo.seqData = _seqInfo.seqDataPos = NULL;
	// _seqInfo.running = false;
}

Screen::~Screen() {
	free(_gameGrid);
	free(_currentScreen);
	free(_scrollScreen);
}

void Screen::halvePalette() {
	uint8 halfPalette[VGA_COLORS * 3];

	for (uint8 cnt = 0; cnt < GAME_COLORS; cnt++) {
		halfPalette[cnt * 3 + 0] = _palette[cnt * 3 + 0] >> 1;
		halfPalette[cnt * 3 + 1] = _palette[cnt * 3 + 1] >> 1;
		halfPalette[cnt * 3 + 2] = _palette[cnt * 3 + 2] >> 1;
	}
	// _system->getPaletteManager()->setPalette(halfPalette, 0, GAME_COLORS);
}

//convert 3 byte 0..63 rgb to 3 byte 0..255 rgb
void Screen::convertPalette(uint8 *inPal, uint8* outPal) {
	int i;

	for (i = 0; i < VGA_COLORS; i++) {
		outPal[3 * i + 0] = (inPal[3 * i + 0] << 2) + (inPal[3 * i + 0] >> 4);
		outPal[3 * i + 1] = (inPal[3 * i + 1] << 2) + (inPal[3 * i + 1] >> 4);
		outPal[3 * i + 2] = (inPal[3 * i + 2] << 2) + (inPal[3 * i + 2] >> 4);
	}
}

void Screen::recreate() {
	// check the game grid for changed blocks
	if (!Logic::_scriptVariables[LAYER_0_ID])
		return;
	uint8 *gridPos = _gameGrid;
	uint8 *screenData = (uint8 *)_skyDisk->fetchItem(Logic::_scriptVariables[LAYER_0_ID]);
	if (!screenData) {
		std::cout << "Screen::recreate():\nSkyEngine::fetchItem(Logic::_scriptVariables[LAYER_0_ID](" << Logic::_scriptVariables[LAYER_0_ID]<< ")) returned NULL" << std::endl;
	}
	uint8 *screenPos = _currentScreen;

	for (uint8 cnty = 0; cnty < GRID_Y; cnty++) {
		for (uint8 cntx = 0; cntx < GRID_X; cntx++) {
			if (gridPos[0] & 0x80) {
				gridPos[0] &= 0x7F; // reset recreate flag
				gridPos[0] |= 1;    // set bit for flip routine
				uint8 *savedScreenY = screenPos;
				for (uint8 gridCntY = 0; gridCntY < GRID_H; gridCntY++) {
					memcpy(screenPos, screenData, GRID_W);
					screenPos += GAME_SCREEN_WIDTH;
					screenData += GRID_W;
				}
				screenPos = savedScreenY + GRID_W;
			} else {
				screenPos += GRID_W;
				screenData += GRID_W * GRID_H;
			}
			gridPos++;
		}
		screenPos += (GRID_H - 1) * GAME_SCREEN_WIDTH;
	}
}

void Screen::flip(bool doUpdate) {
	uint32 copyX, copyWidth;
	copyX = copyWidth = 0;
	for (uint8 cnty = 0; cnty < GRID_Y; cnty++) {
		for (uint8 cntx = 0; cntx < GRID_X; cntx++) {
			if (_gameGrid[cnty * GRID_X + cntx] & 1) {
				_gameGrid[cnty * GRID_X + cntx] &= ~1;
				if (!copyWidth)
					copyX = cntx * GRID_W;
				copyWidth += GRID_W;
			} else if (copyWidth) {
				// _system->copyRectToScreen(_currentScreen + cnty * GRID_H * GAME_SCREEN_WIDTH + copyX, GAME_SCREEN_WIDTH, copyX, cnty * GRID_H, copyWidth, GRID_H);
				copyWidth = 0;
			}
		}
		if (copyWidth) {
			// _system->copyRectToScreen(_currentScreen + cnty * GRID_H * GAME_SCREEN_WIDTH + copyX, GAME_SCREEN_WIDTH, copyX, cnty * GRID_H, copyWidth, GRID_H);
			copyWidth = 0;
		}
	}
	// if (doUpdate)
		// _system->updateScreen();
}

void Screen::fnDrawScreen(uint32 palette, uint32 scroll) {
	// set up the new screen
	// fnFadeDown(scroll);
	forceRefresh();
	recreate();
	// spriteEngine();
	flip(false);
	// fnFadeUp(palette, scroll);
}

//- sprites.asm routines

void Screen::spriteEngine() {
	// doSprites(BACK);
	// sortSprites();
	// doSprites(FORE);
}

// Function to save 320x200 VGA image with a 256-color palette
void Screen::writeBMP(const char* filename, uint8* image, uint8* palette, uint16 width, uint16 height) {
	BMPHeader bmpHeader;
	DIBHeader dibHeader;
	
	bmpHeader.bfOffBits = sizeof(BMPHeader) + sizeof(DIBHeader) + 256 * 4;
	bmpHeader.bfSize = bmpHeader.bfOffBits + dibHeader.biSizeImage;

	// prepend folder to filename
	std::string imagePath = "img//";
	std::string filenamePath = imagePath + filename;

	std::ofstream file(filenamePath, std::ios::binary);
	if (!file) {
		std::cerr << "Error: Unable to open file for writing!" << std::endl;
		return;
	}

	// Write headers
	file.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(bmpHeader));
	file.write(reinterpret_cast<const char*>(&dibHeader), sizeof(dibHeader));

	uint8 fullyFadedUp = 32;
	byte tmpPal[VGA_COLORS * 3];

	convertPalette(palette, tmpPal);
	
	// Write palette (VGA uses RGB but BMP expects RGBA)
	for (int i = 0; i < VGA_COLORS; i++) {
		file.put((tmpPal[i * 3 + 2] * fullyFadedUp) >> 5); // Blue
		file.put((tmpPal[i * 3 + 1] * fullyFadedUp) >> 5); // Green
		file.put((tmpPal[i * 3 + 0] * fullyFadedUp) >> 5); // Red
		file.put(0);                  // Reserved (0)
	}

	// Write pixel data (BMP stores rows bottom-up, so we flip it)
	for (int y = (height - 1); y >= 0; y--) {
		file.write(reinterpret_cast<const char*>(image + y * width), width);
	}

	file.close();
	std::cout << "BMP file saved: " << filename << std::endl;
}



uint8* Screen::recreateImage(uint16 fileNr){
	uint8* currentScreen = (uint8 *)malloc(FULL_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);
	uint8* gameGrid;
	memset(gameGrid, 0x80, GRID_X * GRID_Y);

	uint8* screenData = _skyDisk->loadFile(fileNr);

	uint8* gridPos = gameGrid;
	uint8* screenPos = currentScreen;

	for (uint8 cnty = 0; cnty < GRID_Y; cnty++) {
		for (uint8 cntx = 0; cntx < GRID_X; cntx++) {
			if (gridPos[0] & 0x80) {
				gridPos[0] &= 0x7F; // reset recreate flag
				gridPos[0] |= 1;    // set bit for flip routine
				uint8 *savedScreenY = screenPos;
				for (uint8 gridCntY = 0; gridCntY < GRID_H; gridCntY++) {
					memcpy(screenPos, screenData, GRID_W);
					screenPos += GAME_SCREEN_WIDTH;
					screenData += GRID_W;
				}
				screenPos = savedScreenY + GRID_W;
			} else {
				screenPos += GRID_W;
				screenData += GRID_W * GRID_H;
			}
			gridPos++;
		}
		screenPos += (GRID_H - 1) * GAME_SCREEN_WIDTH;
	}

	return currentScreen;
}