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


//  #include "common/debug.h"
#include "endian.h"
//  #include "common/textconsole.h"

#include "disk.h"
#include "logic.h"
#include "text.h"
//  #include "sky/sky.h"
#include "skydefs.h"
#include "struc.h"
#include "compact.h"

#define FIRST_TEXT_SEC	77
#define	FIRST_TEXT_BUFFER	274
#define LAST_TEXT_BUFFER	284
#define NO_OF_TEXT_SECTIONS	8	// 8 sections per language
#define	CHAR_SET_FILE	60150
#define MAX_SPEECH_SECTION	7
#define CHAR_SET_HEADER	128
#define	MAX_NO_LINES	10

// Text::Text(SkyEngine *vm, Disk *skyDisk, SkyCompact *skyCompact) : _skyDisk(skyDisk), _skyCompact(skyCompact), _vm(vm) {
Text::Text(Disk *skyDisk, SkyCompact *skyCompact) : _skyDisk(skyDisk), _skyCompact(skyCompact) {
    // initHuffTree();
    _huffTree = _huffTree_00372;  //setting hufftree directly to 3.72

    _mainCharacterSet.addr = _skyDisk->loadFile(CHAR_SET_FILE);
    _mainCharacterSet.charHeight = MAIN_CHAR_HEIGHT;
    _mainCharacterSet.charSpacing = 0;

    // fnSetFont(0);

    // if (!SkyEngine::isDemo()) {
    //     _controlCharacterSet.addr = _skyDisk->loadFile(60520);
    //     _controlCharacterSet.charHeight = 12;
    //     _controlCharacterSet.charSpacing = 0;

    //     _linkCharacterSet.addr = _skyDisk->loadFile(60521);
    //     _linkCharacterSet.charHeight = 12;
    //     _linkCharacterSet.charSpacing = 1;
    // } else {
        _controlCharacterSet.addr = NULL;
        _linkCharacterSet.addr = NULL;
    // }
}

Text::~Text() {
    for (int i = FIRST_TEXT_BUFFER; i <= LAST_TEXT_BUFFER; i++)
        // if (SkyEngine::_itemList[i]) {
        //     free(SkyEngine::_itemList[i]);
        //     SkyEngine::_itemList[i] = NULL;
        // }
        if (_skyDisk->_itemList[i]) {
            free(_skyDisk->_itemList[i]);
            _skyDisk->_itemList[i] = NULL;
        }

    free(_mainCharacterSet.addr);
    free(_controlCharacterSet.addr);
    free(_linkCharacterSet.addr);
}

// void Text::fnTextModule(uint32 textInfoId, uint32 textNo) {
//     fnSetFont(1);
//     uint16* msgData = (uint16 *)_skyCompact->fetchCpt(textInfoId);
//     DisplayedText textId = lowTextManager(textNo, msgData[1], msgData[2], 209, false);
//     Logic::_scriptVariables[RESULT] = textId.compactNum;
//     Compact *textCompact = _skyCompact->fetchCpt(textId.compactNum);
//     textCompact->xcood = msgData[3];
//     textCompact->ycood = msgData[4];
//     fnSetFont(0);
// }

char* Text::getText(uint32 textNr) { //load text #"textNr" into textBuffer
    uint32 sectionNo = (textNr & 0x0F000) >> 12;

    // removed logic for chinese traditional

    if (_skyDisk->_itemList[FIRST_TEXT_SEC + sectionNo] == NULL) { //check if already loaded
        std::cout << "Loading Text item(s) for Section " << (sectionNo >> 2) << std::endl;

        uint16 language = SKY_ENGLISH;
        // uint32 fileNo = sectionNo + ((SkyEngine::_systemVars->language * NO_OF_TEXT_SECTIONS) + 60600);
        uint32 fileNo = sectionNo + ((language * NO_OF_TEXT_SECTIONS) + 60600);
        _skyDisk->_itemList[FIRST_TEXT_SEC + sectionNo] = (void **)_skyDisk->loadFile((uint16)fileNo);
    }
    uint8 *textDataPtr = (uint8 *)_skyDisk->_itemList[FIRST_TEXT_SEC + sectionNo];

    uint32 offset = 0;

    uint32 blockNr = textNr & 0xFE0;
    textNr &= 0x1F;

    if (blockNr) {
        uint16 *blockPtr = (uint16 *)(textDataPtr + 4);
        uint32 nr32MsgBlocks = blockNr >> 5;

        do {
            offset += READ_LE_UINT16(blockPtr);
            blockPtr++;
        } while (--nr32MsgBlocks);
    }

    if (textNr) {
        uint8 *blockPtr = textDataPtr + blockNr + READ_LE_UINT16(textDataPtr);
        do {
            uint16 skipBytes = *blockPtr++;
            if (skipBytes & 0x80) {
                skipBytes &= 0x7F;
                skipBytes <<= 3;
            }
            offset += skipBytes;
        } while (--textNr);
    }

    uint32 bitPos = offset & 3;
    offset >>= 2;
    offset += READ_LE_UINT16(textDataPtr + 2);

    textDataPtr += offset;

    //bit pointer: 0->8, 1->6, 2->4 ...
    bitPos ^= 3;
    bitPos++;
    bitPos <<= 1;

    char *dest = (char *)_textBuffer;
    char textChar;

    do {
        textChar = getTextChar(&textDataPtr, &bitPos);
        *dest++ = textChar;
    } while (textChar);

    return (char*)_textBuffer;
}

bool Text::getTextBit(uint8 **data, uint32 *bitPos) {
    if (*bitPos) {
        (*bitPos)--;
    } else {
        (*data)++;
        *bitPos = 7;
    }

    return (bool)(((**data) >> (*bitPos)) & 1);
}

char Text::getTextChar(uint8 **data, uint32 *bitPos) {
    int pos = 0;
    while (1) {
        if (getTextBit(data, bitPos))
            pos = _huffTree[pos].rChild;
        else
            pos = _huffTree[pos].lChild;

        if (_huffTree[pos].lChild == 0 && _huffTree[pos].rChild == 0) {
            return _huffTree[pos].value;
        }
    }
}

// DisplayedText Text::lowTextManager(uint32 textNum, uint16 width, uint16 logicNum, uint8 color, bool center) {
//     getText(textNum);
//     DisplayedText textInfo = displayText(_textBuffer, sizeof(_textBuffer), NULL, center, width, color);

//     uint32 compactNum = FIRST_TEXT_COMPACT;
//     Compact *cpt = _skyCompact->fetchCpt(compactNum);
//     while (cpt->status != 0) {
//         compactNum++;
//         cpt = _skyCompact->fetchCpt(compactNum);
//     }

//     cpt->flag = (uint16)(compactNum - FIRST_TEXT_COMPACT) + FIRST_TEXT_BUFFER;

//     if (_skyDisk->_itemList[cpt->flag])
//         free(_skyDisk->_itemList[cpt->flag]);

//     _skyDisk->_itemList[cpt->flag] = textInfo.textData;

//     cpt->logic = logicNum;
//     cpt->status = ST_LOGIC | ST_FOREGROUND | ST_RECREATE;
//     cpt->screen = (uint16) Logic::_scriptVariables[SCREEN];

//     textInfo.compactNum = (uint16)compactNum;
//     return textInfo;
// }

