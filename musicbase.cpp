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

#include "musicbase.h"
#include "disk.h"
// #include "common/util.h"
#include "endian.h"
// #include "common/textconsole.h"
// #include "audio/audiostream.h"

MusicBase::MusicBase(Disk *pDisk) {
	_musicData = NULL;
	// _mixer = pMixer;
	_skyDisk = pDisk;
	_currentMusic = 0;
	_musicVolume = 127;
	_numberOfChannels = _currentMusic = 0;
}

MusicBase::~MusicBase() {
	stopMusic();
	free(_musicData);
}

void MusicBase::loadSection(uint8 pSection) {
	if (_currentMusic)
		stopMusicInternal();
	// Common::StackLock lock(_mutex);
	free(_musicData);
	_currentSection = pSection;
	_musicData = _skyDisk->loadFile(_driverFileBase + FILES_PER_SECTION * pSection);

	_musicTempo0 = 0x78; // init constants taken from idb file, area ~0x1060
	_musicTempo1 = 0xC0;
	_onNextPoll.musicToProcess = 0;
	_tempo = _aktTime = 0x10001;
	_numberOfChannels = _currentMusic = 0;
	setupPointers();
	startDriver();
}

// bool MusicBase::musicIsPlaying() {
// 	if (_mixer->isSoundHandleActive(_musicHandle))
// 		return true;
// 	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++)
// 		if (_channels[cnt]->isActive())
// 			return true;
// 	return false;
// }

void MusicBase::stopMusic() {
	stopMusicInternal();
}

void MusicBase::stopMusicInternal() {
	// _mixer->stopHandle(_musicHandle);

	// Common::StackLock lock(_mutex);

	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++)
		delete _channels[cnt];
	_numberOfChannels = 0;
}

void MusicBase::updateTempo() {
	uint16 tempoMul = _musicTempo0 * _musicTempo1;
	uint16 divisor = 0x4446390/ 23864;
	_tempo = (tempoMul / divisor) << 16;
	_tempo |= (((tempoMul % divisor) << 16) | (tempoMul / divisor)) / divisor;
}

void MusicBase::loadNewMusic() {
	uint16 musicPos;
	if (_onNextPoll.musicToProcess > _musicData[_musicDataLoc]) {
		// error("Music %d requested but doesn't exist in file.", _onNextPoll.musicToProcess);
		std::cout << "Music " << _onNextPoll.musicToProcess << " requested but doesn't exist in file." << std::endl;
		return;
	}
	if (_currentMusic != 0)
		stopMusicInternal();

	_currentMusic = _onNextPoll.musicToProcess;

	if (_currentMusic == 0)
		return;

	// removed logic for digital audio from the Music Enhancement Project

	// no digital audio, resort to MIDI playback
	musicPos = READ_LE_UINT16(_musicData + _musicDataLoc + 1);
	musicPos += _musicDataLoc + ((_currentMusic - 1) << 1);
	musicPos = READ_LE_UINT16(_musicData + musicPos) + _musicDataLoc;

	_musicTempo0 = _musicData[musicPos];
	_musicTempo1 = _musicData[musicPos+1];

	setupChannels(_musicData + musicPos + 2);

	updateTempo();
}

// void MusicBase::pollMusic() {
// 	Common::StackLock lock(_mutex);
// 	uint8 newTempo;
// 	if (_onNextPoll.musicToProcess != _currentMusic)
// 		loadNewMusic();

// 	_aktTime += _tempo;

// 	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
// 		newTempo = _channels[cnt]->process((uint16)(_aktTime >> 16));
// 		if (newTempo) {
// 			_musicTempo1 = newTempo;
// 			updateTempo();
// 		}
// 	}
// 	_aktTime &= 0xFFFF;
// }

void MusicBase::startMusic(uint16 param) {
	_onNextPoll.musicToProcess = param & 0xF;
}

uint8 MusicBase::giveVolume() {
	return (uint8)_musicVolume;
}

uint8 MusicBase::giveCurrentMusic() {
	return _currentMusic;
}
