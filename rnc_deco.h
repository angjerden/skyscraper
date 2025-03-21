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

#ifndef COMMON_RNC_DECO_H
#define COMMON_RNC_DECO_H

#include "types.h"
#include "endian.h"
#include "string.h"


// Decoder for RNC (Rob Norton Computing) ProPack compression.
class RncDecoder {

protected:
	uint16 _rawTable[64];
	uint16 _posTable[64];
	uint16 _lenTable[64];
	uint16 _crcTable[256];

	uint16 _bitBuffl;
	uint16 _bitBuffh;
	uint8 _bitCount;

	const uint8 *_srcPtr;
	uint8 *_dstPtr;

	int32 _inputByteLeft;

public:
	RncDecoder();
	~RncDecoder();
	int32 unpackM1(const void *input, uint inputSize, void *output);
	int32 unpackM2(const void *input, void *output);

	static const uint32 kRnc1Signature = 0x524E4301; // "RNC\001"
	static const uint32 kRnc2Signature = 0x524E4302; // "RNC\002"

protected:
	void initCrc();
	uint16 crcBlock(const uint8 *block, uint32 size);
	uint16 inputBits(uint8 amount);
	void makeHufftable(uint16 *table);
	uint16 inputValue(uint16 *table);
	int getbit();
};

#endif
