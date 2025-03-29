#include "disk.h"
#include <iostream>
#include <fstream>

void *Disk::_itemList[300];

Disk::Disk(char* skyPath) {
    std::string diskFilePath(skyPath);
    diskFilePath.append(_diskFilename);
	_dataDiskHandle = fopen(diskFilePath.c_str(), "rb");

    std::string dnrFilePath(skyPath);
    dnrFilePath.append(_dnrFilename);
    std::FILE* dnrFile = fopen(dnrFilePath.c_str(), "rb");
    std::fseek(dnrFile, 0, SEEK_SET);

    // Read no of entries
    std::fread(&_dinnerTableEntries, 1, 4, dnrFile);

    std::cout << "Found " << _dinnerTableEntries << " dinner table entries." << std::endl;
    
    // Read entries into dinnerTable
    _dinnerTableArea = (uint8_t*)malloc(_dinnerTableEntries * 8);
    std::fread(_dinnerTableArea, 1, 8 * _dinnerTableEntries, dnrFile);
	
    memset(_buildList, 0, 60 * 2);
	memset(_loadedFilesList, 0, 60 * 4);

    fclose(dnrFile);

	// From sky.cpp
	for (int i = 0; i < 300; i++)
		_itemList[i] = NULL;

}

Disk::~Disk() {
	if (_dataDiskHandle != NULL)
		delete _dataDiskHandle;
        _dataDiskHandle = NULL;
	fnFlushBuffers();
	free(_dinnerTableArea);
	delete _dataDiskHandle;

	// From sky.cpp
	for (int i = 0; i < 300; i++)
		if (_itemList[i])
			free(_itemList[i]);
}

bool Disk::fileExists(uint16 fileNr) {
	return (getFileInfo(fileNr) != NULL);
}

// allocate memory, load the file and return a pointer
uint8 *Disk::loadFile(uint16 fileNr) {
	uint8 cflag;

	std::cout << "load file " << (fileNr >> 11) << ", " << (fileNr & 2047) << " (" << fileNr << ")" << std::endl;

	uint8 *fileInfoPtr = getFileInfo(fileNr);
	if (fileInfoPtr == NULL) {
		std::cout << "File " << fileNr << "not found" << std::endl;
		return NULL;
	}

	uint32 fileFlags = READ_LE_UINT24(fileInfoPtr + 5);
	uint32 fileSize = fileFlags & 0x03fffff;
	uint32 fileOffset = READ_LE_UINT32(fileInfoPtr + 2) & 0x0ffffff;

	_lastLoadedFileSize = fileSize;
	cflag = (uint8)((fileOffset >> 23) & 0x1);
	fileOffset &= 0x7FFFFF;

	if (cflag) {
		fileOffset <<= 4;
	}

	uint8 *fileDest = (uint8 *)malloc(fileSize + 4); // allocate memory for file

	std::fseek(_dataDiskHandle, fileOffset, SEEK_SET);

	//now read in the data
	int32 bytesRead = std::fread(fileDest, 1, fileSize, _dataDiskHandle);

	if (bytesRead != (int32)fileSize)
		std::cout << "Unable to read " << fileSize << " bytes from datadisk (" << bytesRead << " bytes read)" << std::endl;

	cflag = (uint8)((fileFlags >> 23) & 0x1);
	//if cflag == 0 then file is compressed, 1 == uncompressed

	DataFileHeader *fileHeader = (DataFileHeader *)fileDest;

	if ((!cflag) && ((fileHeader->flag >> 7) & 1)) {
		std::cout << "File is RNC compressed." << std::endl;

		uint32 decompSize = (fileHeader->flag & ~0xFF) << 8;
		decompSize |= fileHeader->s_tot_size;

		uint8 *uncompDest = (uint8 *)malloc(decompSize);

		int32 unpackLen;
		void *output, *input = fileDest + sizeof(DataFileHeader);

		if ((fileFlags >> 22) & 0x1) { //do we include the header?
			// don't return the file's header
			output = uncompDest;
			unpackLen = _rncDecoder.unpackM1(input, fileSize - sizeof(DataFileHeader), output);
		} else {
			memcpy(uncompDest, fileDest, sizeof(DataFileHeader));
			output = uncompDest + sizeof(DataFileHeader);
			unpackLen = _rncDecoder.unpackM1(input, fileSize - sizeof(DataFileHeader), output);
			if (unpackLen)
				unpackLen += sizeof(DataFileHeader);
		}

		std::cout << "UnpackM1 returned: " << unpackLen << std::endl;

		if (unpackLen == 0) { //Unpack returned 0: file was probably not packed.
			free(uncompDest);
			return fileDest;
		} else {
			if (unpackLen != (int32)decompSize)
				std::cout << "ERROR: File " << fileNr << ": invalid decomp size! (was: " << unpackLen << ", should be: " << decompSize << ")" << std::endl;
			_lastLoadedFileSize = decompSize;

			free(fileDest);
			return uncompDest;
		}
	} else {
		return fileDest;
	}
}

void Disk::fnFlushBuffers() {
	// dump all loaded sprites
	uint8 lCnt = 0;
	while (_loadedFilesList[lCnt]) {
		free(_itemList[_loadedFilesList[lCnt] & 2047]);
		_itemList[_loadedFilesList[lCnt] & 2047] = NULL;
		lCnt++;
	}
	_loadedFilesList[0] = 0;
}

void Disk::loadFixedItems() {
	_itemList[49] = loadFile(49);
	_itemList[50] = loadFile(50);
	_itemList[73] = loadFile(73);
	_itemList[262] = loadFile(262);

	_itemList[36] = loadFile(36);
	_itemList[263] = loadFile(263);
	_itemList[264] = loadFile(264);
	_itemList[265] = loadFile(265);
	_itemList[266] = loadFile(266);
	_itemList[267] = loadFile(267);
	_itemList[269] = loadFile(269);
	_itemList[271] = loadFile(271);
	_itemList[272] = loadFile(272);
}

void *Disk::fetchItem(uint32 num) {
	return _itemList[num];
}

uint16 *Disk::loadScriptFile(uint16 fileNr) {
	return (uint16 *)loadFile(fileNr);
}

uint8 *Disk::getFileInfo(uint16 fileNr) {
	uint16 i;
	uint16 *dnrTbl16Ptr = (uint16 *)_dinnerTableArea;

	for (i = 0; i < _dinnerTableEntries; i++) {
		uint16 fNr = READ_LE_UINT16(dnrTbl16Ptr);
		if (fNr == fileNr) {
			std::cout << "file " << fileNr << " found" << std::endl;
			return (uint8 *)dnrTbl16Ptr;
		}
		dnrTbl16Ptr += 4;
	}

	return 0; //not found
}

void Disk::fnMiniLoad(uint16 fileNum) {
	uint16 cnt = 0;
	while (_loadedFilesList[cnt]) {
		if (_loadedFilesList[cnt] == fileNum)
			return;
		cnt++;
	}
	_loadedFilesList[cnt] = fileNum & 0x7FFFU;
	_loadedFilesList[cnt + 1] = 0;
	_itemList[fileNum & 2047] = (void**)loadFile(fileNum);
}

void Disk::writeDinnerTableToFile() {
	uint16 i;
	uint8 *dnrTblPtr = (uint8 *)_dinnerTableArea;

	std::ofstream outfile;
	outfile.open("dinnerTableFile.txt");

	for (i = 0; i < _dinnerTableEntries; i++) {
		uint16 fNr = READ_LE_UINT16(dnrTblPtr);
		std::cout << "file " << fNr << "" << std::endl;

		uint32 fileFlags = READ_LE_UINT24(dnrTblPtr + 5);
		uint32 fileSize = fileFlags & 0x03fffff;
		uint32 fileOffset = READ_LE_UINT32(dnrTblPtr + 2) & 0x0ffffff;
		uint8 cflag = (uint8)((fileOffset >> 23) & 0x1);
		fileOffset &= 0x7FFFFF;
		if (cflag) {
			fileOffset <<= 4;
		}
		cflag = (uint8)((fileFlags >> 23) & 0x1);

		uint8 *fileDest = (uint8 *)malloc(fileSize + 4); // allocate memory for file

		std::fseek(_dataDiskHandle, fileOffset, SEEK_SET);

		//now read in the data
		int32 bytesRead = std::fread(fileDest, 1, fileSize, _dataDiskHandle);

		DataFileHeader *fileHeader = (DataFileHeader *)fileDest;

		outfile << "File nr " << fNr << std::endl;
		outfile << "File flags " << fileFlags << std::endl;
		outfile << "File size " << fileSize << std::endl;
		outfile << "File offset " << fileOffset << std::endl;
		outfile << "Compressed flag " << !cflag << std::endl; //if cflag == 0 then file is compressed, 1 == uncompressed
		outfile << "Header flag " << fileHeader->flag << std::endl;
		outfile << "Header s_x " << fileHeader->s_x << std::endl;
		outfile << "Header s_y " << fileHeader->s_y << std::endl;
		outfile << "Header s_width " << fileHeader->s_width << std::endl;
		outfile << "Header s_height " << fileHeader->s_height << std::endl;
		outfile << "Header s_sp_size " << fileHeader->s_sp_size << std::endl;
		outfile << "Header s_tot_size " << fileHeader->s_tot_size << std::endl;
		outfile << "Header s_n_sprites " << fileHeader->s_n_sprites << std::endl;
		outfile << "Header s_offset_x " << fileHeader->s_offset_x << std::endl;
		outfile << "Header s_offset_y " << fileHeader->s_offset_y << std::endl;
		outfile << "Header s_compressed_size " << fileHeader->s_compressed_size << std::endl;
		outfile << std::endl;

		dnrTblPtr += 8;
	}

	outfile.close();
}
