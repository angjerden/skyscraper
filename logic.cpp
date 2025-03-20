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

#include "assert.h"
#include "compact.h"
#include "disk.h"
#include "endian.h"
#include <iterator>
#include "logic.h"
#include "skydefs.h"
#include "util.h"
#include <string.h>
#include <stdio.h>
#include "text.h"
#include <iostream>
// #include <map>


uint32 Logic::_scriptVariables[NUM_SKY_SCRIPTVARS];

void Logic::setupLogicTable() {
	static const LogicTable logicTable[] = {
		&Logic::nop,
		&Logic::logicScript,	 // 1  script processor
		&Logic::nop, //		&Logic::autoRoute,	 // 2  Make a route
		&Logic::nop, // &Logic::arAnim,	 // 3  Follow a route
		&Logic::nop, //	&Logic::arTurn,	 // 4  Mega turns araound
		&Logic::nop, //	&Logic::alt,		 // 5  Set up new get-to script
		&Logic::nop, //	&Logic::anim,	 // 6  Follow a sequence
		&Logic::nop, //	&Logic::turn,	 // 7  Mega turning
		&Logic::nop, //	&Logic::cursor,	 // 8  id tracks the pointer
		&Logic::nop, //	&Logic::talk,	 // 9  count down and animate
		&Logic::nop, //	&Logic::listen,	 // 10 player waits for talking id
		&Logic::nop, //	&Logic::stopped,	 // 11 wait for id to move
		&Logic::nop, //	&Logic::choose,	 // 12 wait for player to click
		&Logic::nop, //	&Logic::frames,	 // 13 animate just frames
		&Logic::nop, //	&Logic::pause,	 // 14 Count down to 0 and go
		&Logic::nop, //	&Logic::waitSync,	 // 15 Set to l_script when sync!=0
		&Logic::nop, //	&Logic::simpleAnim,	 // 16 Module anim without x,y's
	};

	_logicTable = logicTable;
}

// Logic::Logic(SkyCompact *skyCompact, Screen *skyScreen, Disk *skyDisk, Text *skyText, MusicBase *skyMusic, Mouse *skyMouse, Sound *skySound)
Logic::Logic(SkyCompact *skyCompact, Disk *skyDisk, Text* skyText) {

	_skyCompact = skyCompact;
	// _skyScreen = skyScreen;
	_skyDisk = skyDisk;
	_skyText = skyText;
	// _skyMusic = skyMusic;
	// _skySound = skySound;
	// _skyMouse = skyMouse;
	// _skyGrid = new Grid(_skyDisk, _skyCompact);
	// _skyAutoRoute = new AutoRoute(_skyGrid, _skyCompact);

	setupLogicTable();
	setupMcodeTable();
	setupMcodeMap();

	memset(_objectList, 0, 30 * sizeof(uint32));

	for (int i = 0; i < ARRAYSIZE(_moduleList); i++)
		_moduleList[i] = 0;
	_stackPtr = 0;

	_currentSection = 0xFF; //force music & sound reload
	initScriptVariables();
}

Logic::~Logic() {
	// delete _skyGrid;
	// delete _skyAutoRoute;

	for (int i = 0; i < ARRAYSIZE(_moduleList); i++)
		if (_moduleList[i])
			free(_moduleList[i]);
}

void Logic::initScreen0() {
	fnEnterSection(0, 0, 0);
	// _skyMusic->startMusic(2);
	// SkyEngine::_systemVars->currentMusic = 2;
}

bool Logic::fnEnterSection(uint32 sectionNo, uint32 b, uint32 c) {
	_scriptVariables[CUR_SECTION] = sectionNo;
	// SkyEngine::_systemVars->currentMusic = 0;

	// if (sectionNo == 5) //linc section - has different mouse icons
		// _skyMouse->replaceMouseCursors(60302);

	// if ((sectionNo != _currentSection) || (SkyEngine::_systemVars->systemFlags & SF_GAME_RESTORED)) {
		// _currentSection = sectionNo;

		// sectionNo++;
		// _skyMusic->loadSection((byte)sectionNo);
		// _skySound->loadSection((byte)sectionNo);
		// _skyGrid->loadGrids();
		// SkyEngine::_systemVars->systemFlags &= ~SF_GAME_RESTORED;
	// }

	return true;
}

void Logic::nop() {}

void Logic::logicScript() {}

void Logic::scrapeCompact(uint16 list, uint16 index) {
	/// Process the current mega's script
	/// If the script finishes then drop back a level

	_compact = _skyCompact->getCompactByIndexes(list, index);
	uint16 size = _skyCompact->getCompactSize(list, index);
	if (size == 0) return;

	uint16 mode = _compact->mode; // get pointer to current script
	uint16 scriptNo = SkyCompact::getSub(_compact, mode);
	uint16 offset   = SkyCompact::getSub(_compact, mode + 2);

	offset = scrapeScript(scriptNo, offset);
	SkyCompact::setSub(_compact, mode + 2, offset);

	if (!offset) // script finished
		_compact->mode -= 4;
	else if (_compact->mode == mode)
		return;

}

void Logic::scrapeAssetsFromCompacts() {
	uint16 numDataLists = _skyCompact->giveNumDataLists();
	uint16 i = 1;
	// uint16 j = 4; // get mini_so compact to test with¨
	// _compact = _skyCompact->getCompactByIndexes(i, j);
	// scrapeCompact();

	// for (uint16 i = 0; i < numDataLists; i++) {
		uint16 dataListLen = _skyCompact->giveDataListLen(i);
		for (uint16 j = 0; j < dataListLen; j++) {
			scrapeCompact(i, j);
		}
	// }
}

uint16 Logic::scrapeScript(uint16 scriptNo, uint16 offset) {
	do {
		bool restartScript = false;

		/// process a script
		/// low level interface to interpreter
		uint16 moduleNo = scriptNo >> 12;
		uint16 *scriptData = _moduleList[moduleNo]; // get module address

		if (!scriptData) { // We need to load the script module
			_moduleList[moduleNo] = (uint16*)_skyDisk->loadFile(moduleNo + F_MODULE_0);
			scriptData = _moduleList[moduleNo];
		}

		uint16* moduleStart = scriptData;

		// Check whether we have an offset or what
		if (offset)
			scriptData = moduleStart + offset;
		else
			scriptData += scriptData[scriptNo & 0x0FFF];

		uint32 a = 0, b = 0, c = 0;
		uint16 command, s;

		while(!restartScript) {
			command = *scriptData++; // get a command
			// Debug::script(command, scriptData);

			switch (command) {
			case 0: // push_variable
				push( _scriptVariables[*scriptData++ / 4] );
				break;
			case 1: // less_than
				a = pop();
				b = pop();
				if (a > b)
					push(1);
				else
					push(0);
				break;
			case 2: // push_number
				push(*scriptData++);
				break;
			case 3: // not_equal
				a = pop();
				b = pop();
				if (a != b)
					push(1);
				else
					push(0);
				break;
			case 4: // if_and
				a = pop();
				b = pop();
				if (a && b)
					push(1);
				else
					push(0);
				break;
			case 5: // skip_zero
				s = *scriptData++;

				a = pop();
				if (!a)
					scriptData += s / 2;
				break;
			case 6: // pop_var
				b = _scriptVariables[*scriptData++ / 4] = pop();
				break;
			case 7: // minus
				a = pop();
				b = pop();
				push(b-a);
				break;
			case 8: // plus
				a = pop();
				b = pop();
				push(b+a);
				break;
			case 9: // skip_always
				s = *scriptData++;
				scriptData += s / 2;
				break;
			case 10: // if_or
				a = pop();
				b = pop();
				if (a || b)
					push(1);
				else
					push(0);
				break;
			case 11: // call_mcode
				{
					a = *scriptData++;
					// assert(a <= 3);
					// No, I did not forget the "break"s
					switch (a) {
					case 3:
						c = pop();
						// fall through
					case 2:
						b = pop();
						// fall through
					case 1:
						a = pop();
						// fall through
					default:
						break;
					}

					uint16 mcode = *scriptData++ / 4; // get mcode number
					// Debug::mcode(mcode, a, b, c);

					Compact *saveCpt = _compact;
					// bool ret = (this->*_mcodeTable[mcode]) (a, b, c);
					// get value of key in map
					
					if(_mcodeMap.size() > mcode){
						std::cout << _mcodeMap[mcode] << " " << a << " " << b << " " << c << std::endl;
						if (mcode > 34 && mcode < 39){
							writeText(b);
						}
					}
					_compact = saveCpt;

					// TODO: Fix infinite loop
					bool ret = true;

					if (!ret)
						offset = scriptData - moduleStart;
				}
				break;
			case 12: // more_than
				a = pop();
				b = pop();
				if (a < b)
					push(1);
				else
					push(0);
				break;
			case 14: // switch
				c = s = *scriptData++; // get number of cases

				a = pop(); // and value to switch on

				do {
					if (a == *scriptData) {
						scriptData += scriptData[1] / 2;
						scriptData++;
						break;
					}
					scriptData += 2;
				} while (--s);

				if (s == 0)
					scriptData += *scriptData / 2; // use the default
				break;
			case 15: // push_offset
				push( *(uint16 *)_skyCompact->getCompactElem(_compact, *scriptData++) );
				break;
			case 16: // pop_offset
				// pop a value into a compact
				*(uint16 *)_skyCompact->getCompactElem(_compact, *scriptData++) = (uint16)pop();
				break;
			case 17: // is_equal
				a = pop();
				b = pop();
				if (a == b)
					push(1);
				else
					push(0);
				break;
			case 18: { // skip_nz
					int16 t = *scriptData++;
					a = pop();
					if (a)
						scriptData += t / 2;
					break;
				}
			case 13:
			case 19: // script_exit
				return 0;
			case 20: // restart_script
				offset = 0;
				restartScript = true;
				break;
			default:
				std::cout << "Unknown script command: " << command << std::endl;
				// error("Unknown script command: %d", command);
			}
		}
	} while (true);
}

void Logic::writeText(uint16 textNr) {
	char* textBuffer = _skyText->getText(textNr);
	std::cout << textBuffer << std::endl;
}


static uint16 clickTable[46] = {
	ID_FOSTER,
	ID_JOEY,
	ID_JOBS,
	ID_LAMB,
	ID_ANITA,
	ID_SON,
	ID_DAD,
	ID_MONITOR,
	ID_SHADES,
	MINI_SS,
	FULL_SS,
	ID_FOREMAN,
	ID_RADMAN,
	ID_GALLAGER_BEL,
	ID_BURKE,
	ID_BODY,
	ID_HOLO,
	ID_TREVOR,
	ID_ANCHOR,
	ID_WRECK_GUARD,
	ID_SKORL_GUARD,

	// BASE LEVEL
	ID_SC30_HENRI,
	ID_SC31_GUARD,
	ID_SC32_VINCENT,
	ID_SC32_GARDENER,
	ID_SC32_BUZZER,
	ID_SC36_BABS,
	ID_SC36_BARMAN,
	ID_SC36_COLSTON,
	ID_SC36_GALLAGHER,
	ID_SC36_JUKEBOX,
	ID_DANIELLE,
	ID_SC42_JUDGE,
	ID_SC42_CLERK,
	ID_SC42_PROSECUTION,
	ID_SC42_JOBSWORTH,

	// UNDERWORLD
	ID_MEDI,
	ID_WITNESS,
	ID_GALLAGHER,
	ID_KEN,
	ID_SC76_ANDROID_2,
	ID_SC76_ANDROID_3,
	ID_SC81_FATHER,
	ID_SC82_JOBSWORTH,

	// LINC WORLD
	ID_HOLOGRAM_B,
	12289
};

void Logic::push(uint32 a) {
	if (_stackPtr > ARRAYSIZE(_stack) - 2)
		std::cout << "Stack overflow" << std::endl;
		// error("Stack overflow");
	_stack[_stackPtr++] = a;
}

uint32 Logic::pop() {
	if (_stackPtr < 1 || _stackPtr > ARRAYSIZE(_stack) - 1)
		std::cout << "No items on Stack to pop" << std::endl;
		// error("No items on Stack to pop");
	return _stack[--_stackPtr];
}

void Logic::setupMcodeMap() {
	_mcodeMap = {
		{0, "fnCacheChip"},
		{1, "fnCacheFast"},
		{2, "fnDrawScreen"},
		{3, "fnAr"},
		{4, "fnArAnimate"},
		{5, "fnIdle"},
		{6, "fnInteract"},
		{7, "fnStartSub"},
		{8, "fnTheyStartSub"},
		{9, "fnAssignBase"},
		{10, "fnDiskMouse"},
		{11, "fnNormalMouse"},
		{12, "fnBlankMouse"},
		{13, "fnCrossMouse"},
		{14, "fnCursorRight"},
		{15, "fnCursorLeft"},
		{16, "fnCursorDown"},
		{17, "fnOpenHand"},
		{18, "fnCloseHand"},
		{19, "fnGetTo"},
		{20, "fnSetToStand"},
		{21, "fnTurnTo"},
		{22, "fnArrived"},
		{23, "fnLeaving"},
		{24, "fnSetAlternate"},
		{25, "fnAltSetAlternate"},
		{26, "fnKillId"},
		{27, "fnNoHuman"},
		{28, "fnAddHuman"},
		{29, "fnAddButtons"},
		{30, "fnNoButtons"},
		{31, "fnSetStop"},
		{32, "fnClearStop"},
		{33, "fnPointerText"},
		{34, "fnQuit"},
		{35, "fnSpeakMe"},
		{36, "fnSpeakMeDir"},
		{37, "fnSpeakWait"},
		{38, "fnSpeakWaitDir"},
		{39, "fnChooser"},
		{40, "fnHighlight"},
		{41, "fnTextKill"},
		{42, "fnStopMode"},
		{43, "fnWeWait"},
		{44, "fnSendSync"},
		{45, "fnSendFastSync"},
		{46, "fnSendRequest"},
		{47, "fnClearRequest"},
		{48, "fnCheckRequest"},
		{49, "fnStartMenu"},
		{50, "fnUnhighlight"},
		{51, "fnFaceId"},
		{52, "fnForeground"},
		{53, "fnBackground"},
		{54, "fnNewBackground"},
		{55, "fnSort"},
		{56, "fnNoSpriteEngine"},
		{57, "fnNoSpritesA6"},
		{58, "fnResetId"},
		{59, "fnToggleGrid"},
		{60, "fnPause"},
		{61, "fnRunAnimMod"},
		{62, "fnSimpleMod"},
		{63, "fnRunFrames"},
		{64, "fnAwaitSync"},
		{65, "fnIncMegaSet"},
		{66, "fnDecMegaSet"},
		{67, "fnSetMegaSet"},
		{68, "fnMoveItems"},
		{69, "fnNewList"},
		{70, "fnAskThis"},
		{71, "fnRandom"},
		{72, "fnPersonHere"},
		{73, "fnToggleMouse"},
		{74, "fnMouseOn"},
		{75, "fnMouseOff"}

	};
}

void Logic::setupMcodeTable() {
	// static const McodeTable mcodeTable[] = {
	// 	&Logic::fnCacheChip,
	// 	&Logic::fnCacheFast,
	// 	&Logic::fnDrawScreen,
	// 	&Logic::fnAr,
	// 	&Logic::fnArAnimate,
	// 	&Logic::fnIdle,
	// 	&Logic::fnInteract,
	// 	&Logic::fnStartSub,
	// 	&Logic::fnTheyStartSub,
	// 	&Logic::fnAssignBase,
	// 	&Logic::fnDiskMouse,
	// 	&Logic::fnNormalMouse,
	// 	&Logic::fnBlankMouse,
	// 	&Logic::fnCrossMouse,
	// 	&Logic::fnCursorRight,
	// 	&Logic::fnCursorLeft,
	// 	&Logic::fnCursorDown,
	// 	&Logic::fnOpenHand,
	// 	&Logic::fnCloseHand,
	// 	&Logic::fnGetTo,
	// 	&Logic::fnSetToStand,
	// 	&Logic::fnTurnTo,
	// 	&Logic::fnArrived,
	// 	&Logic::fnLeaving,
	// 	&Logic::fnSetAlternate,
	// 	&Logic::fnAltSetAlternate,
	// 	&Logic::fnKillId,
	// 	&Logic::fnNoHuman,
	// 	&Logic::fnAddHuman,
	// 	&Logic::fnAddButtons,
	// 	&Logic::fnNoButtons,
	// 	&Logic::fnSetStop,
	// 	&Logic::fnClearStop,
	// 	&Logic::fnPointerText,
	// 	&Logic::fnQuit,
	// 	&Logic::fnSpeakMe,
	// 	&Logic::fnSpeakMeDir,
	// 	&Logic::fnSpeakWait,
	// 	&Logic::fnSpeakWaitDir,
	// 	&Logic::fnChooser,
	// 	&Logic::fnHighlight,
	// 	&Logic::fnTextKill,
	// 	&Logic::fnStopMode,
	// 	&Logic::fnWeWait,
	// 	&Logic::fnSendSync,
	// 	&Logic::fnSendFastSync,
	// 	&Logic::fnSendRequest,
	// 	&Logic::fnClearRequest,
	// 	&Logic::fnCheckRequest,
	// 	&Logic::fnStartMenu,
	// 	&Logic::fnUnhighlight,
	// 	&Logic::fnFaceId,
	// 	&Logic::fnForeground,
	// 	&Logic::fnBackground,
	// 	&Logic::fnNewBackground,
	// 	&Logic::fnSort,
	// 	&Logic::fnNoSpriteEngine,
	// 	&Logic::fnNoSpritesA6,
	// 	&Logic::fnResetId,
	// 	&Logic::fnToggleGrid,
	// 	&Logic::fnPause,
	// 	&Logic::fnRunAnimMod,
	// 	&Logic::fnSimpleMod,
	// 	&Logic::fnRunFrames,
	// 	&Logic::fnAwaitSync,
	// 	&Logic::fnIncMegaSet,
	// 	&Logic::fnDecMegaSet,
	// 	&Logic::fnSetMegaSet,
	// 	&Logic::fnMoveItems,
	// 	&Logic::fnNewList,
	// 	&Logic::fnAskThis,
	// 	&Logic::fnRandom,
	// 	&Logic::fnPersonHere,
	// 	&Logic::fnToggleMouse,
	// 	&Logic::fnMouseOn,
	// 	&Logic::fnMouseOff,
	// 	&Logic::fnFetchX,
	// 	&Logic::fnFetchY,
	// 	&Logic::fnTestList,
	// 	&Logic::fnFetchPlace,
	// 	&Logic::fnCustomJoey,
	// 	&Logic::fnSetPalette,
	// 	&Logic::fnTextModule,
	// 	&Logic::fnChangeName,
	// 	&Logic::fnMiniLoad,
	// 	&Logic::fnFlushBuffers,
	// 	&Logic::fnFlushChip,
	// 	&Logic::fnSaveCoods,
	// 	&Logic::fnPlotGrid,
	// 	&Logic::fnRemoveGrid,
	// 	&Logic::fnEyeball,
	// 	&Logic::fnCursorUp,
	// 	&Logic::fnLeaveSection,
	// 	&Logic::fnEnterSection,
	// 	&Logic::fnRestoreGame,
	// 	&Logic::fnRestartGame,
	// 	&Logic::fnNewSwingSeq,
	// 	&Logic::fnWaitSwingEnd,
	// 	&Logic::fnSkipIntroCode,
	// 	&Logic::fnBlankScreen,
	// 	&Logic::fnPrintCredit,
	// 	&Logic::fnLookAt,
	// 	&Logic::fnLincTextModule,
	// 	&Logic::fnTextKill2,
	// 	&Logic::fnSetFont,
	// 	&Logic::fnStartFx,
	// 	&Logic::fnStopFx,
	// 	&Logic::fnStartMusic,
	// 	&Logic::fnStopMusic,
	// 	&Logic::fnFadeDown,
	// 	&Logic::fnFadeUp,
	// 	&Logic::fnQuitToDos,
	// 	&Logic::fnPauseFx,
	// 	&Logic::fnUnPauseFx,
	// 	&Logic::fnPrintf
	// };

	// _mcodeTable = mcodeTable;
}

static const uint32 forwardList1b[] = {
	JOBS_SPEECH,
	JOBS_S4,
	JOBS_ALARMED,
	JOEY_RECYCLE,
	SHOUT_SSS,
	JOEY_MISSION,
	TRANS_MISSION,
	SLOT_MISSION,
	CORNER_MISSION,
	JOEY_LOGIC,
	GORDON_SPEECH,
	JOEY_BUTTON_MISSION,
	LOB_DAD_SPEECH,
	LOB_SON_SPEECH,
	GUARD_SPEECH,
	MANTRACH_SPEECH,
	WRECK_SPEECH,
	ANITA_SPEECH,
	LAMB_FACTORY,
	FORE_SPEECH,
	JOEY_42_MISS,
	JOEY_JUNCTION_MISS,
	WELDER_MISSION,
	JOEY_WELD_MISSION,
	RADMAN_SPEECH,
	LINK_7_29,
	LINK_29_7,
	LAMB_TO_3,
	LAMB_TO_2,
	BURKE_SPEECH,
	BURKE_1,
	BURKE_2,
	DR_BURKE_1,
	JASON_SPEECH,
	JOEY_BELLEVUE,
	ANCHOR_SPEECH,
	ANCHOR_MISSION,
	JOEY_PC_MISSION,
	HOOK_MISSION,
	TREVOR_SPEECH,
	JOEY_FACTORY,
	HELGA_SPEECH,
	JOEY_HELGA_MISSION,
	GALL_BELLEVUE,
	GLASS_MISSION,
	LAMB_FACT_RETURN,
	LAMB_LEAVE_GARDEN,
	LAMB_START_29,
	LAMB_BELLEVUE,
	CABLE_MISSION,
	FOSTER_TOUR,
	LAMB_TOUR,
	FOREMAN_LOGIC,
	LAMB_LEAVE_FACTORY,
	LAMB_BELL_LOGIC,
	LAMB_FACT_2,
	START90,
	0,
	0,
	LINK_28_31,
	LINK_31_28,
	EXIT_LINC,
	DEATH_SCRIPT
};

static const uint32 forwardList2b[] = {
	STD_ON,
	STD_EXIT_LEFT_ON,
	STD_EXIT_RIGHT_ON,
	ADVISOR_188,
	SHOUT_ACTION,
	MEGA_CLICK,
	MEGA_ACTION
};

static const uint32 forwardList3b[] = {
	DANI_SPEECH,
	DANIELLE_GO_HOME,
	SPUNKY_GO_HOME,
	HENRI_SPEECH,
	BUZZER_SPEECH,
	FOSTER_VISIT_DANI,
	DANIELLE_LOGIC,
	JUKEBOX_SPEECH,
	VINCENT_SPEECH,
	EDDIE_SPEECH,
	BLUNT_SPEECH,
	DANI_ANSWER_PHONE,
	SPUNKY_SEE_VIDEO,
	SPUNKY_BARK_AT_FOSTER,
	SPUNKY_SMELLS_FOOD,
	BARRY_SPEECH,
	COLSTON_SPEECH,
	GALL_SPEECH,
	BABS_SPEECH,
	CHUTNEY_SPEECH,
	FOSTER_ENTER_COURT
};

static const uint32 forwardList4b[] = {
	WALTER_SPEECH,
	JOEY_MEDIC,
	JOEY_MED_LOGIC,
	JOEY_MED_MISSION72,
	KEN_LOGIC,
	KEN_SPEECH,
	KEN_MISSION_HAND,
	SC70_IRIS_OPENED,
	SC70_IRIS_CLOSED,
	FOSTER_ENTER_BOARDROOM,
	BORED_ROOM,
	FOSTER_ENTER_NEW_BOARDROOM,
	HOBS_END,
	SC82_JOBS_SSS
};

static const uint32 forwardList5b[] = {
	SET_UP_INFO_WINDOW,
	SLAB_ON,
	UP_MOUSE,
	DOWN_MOUSE,
	LEFT_MOUSE,
	RIGHT_MOUSE,
	DISCONNECT_FOSTER
};

void Logic::initScriptVariables() {
	for (int i = 0; i < ARRAYSIZE(_scriptVariables); i++)
		_scriptVariables[i] = 0;

	_scriptVariables[LOGIC_LIST_NO] = 141;
	_scriptVariables[LAMB_GREET] = 62;
	_scriptVariables[JOEY_SECTION] = 1;
	_scriptVariables[LAMB_SECTION] = 2;
	_scriptVariables[S15_FLOOR] = 8371;
	_scriptVariables[GUARDIAN_THERE] = 1;
	_scriptVariables[DOOR_67_68_FLAG] = 1;
	_scriptVariables[SC70_IRIS_FLAG] = 3;
	_scriptVariables[DOOR_73_75_FLAG] = 1;
	_scriptVariables[SC76_CABINET1_FLAG] = 1;
	_scriptVariables[SC76_CABINET2_FLAG] = 1;
	_scriptVariables[SC76_CABINET3_FLAG] = 1;
	_scriptVariables[DOOR_77_78_FLAG] = 1;
	_scriptVariables[SC80_EXIT_FLAG] = 1;
	_scriptVariables[SC31_LIFT_FLAG] = 1;
	_scriptVariables[SC32_LIFT_FLAG] = 1;
	_scriptVariables[SC33_SHED_DOOR_FLAG] = 1;
	_scriptVariables[BAND_PLAYING] = 1;
	_scriptVariables[COLSTON_AT_TABLE] = 1;
	_scriptVariables[SC36_NEXT_DEALER] = 16731;
	_scriptVariables[SC36_DOOR_FLAG] = 1;
	_scriptVariables[SC37_DOOR_FLAG] = 2;
	_scriptVariables[SC40_LOCKER_1_FLAG] = 1;
	_scriptVariables[SC40_LOCKER_2_FLAG] = 1;
	_scriptVariables[SC40_LOCKER_3_FLAG] = 1;
	_scriptVariables[SC40_LOCKER_4_FLAG] = 1;
	_scriptVariables[SC40_LOCKER_5_FLAG] = 1;

	// if (SkyEngine::_systemVars->gameVersion == 288)
	// 	memcpy(_scriptVariables + 352, forwardList1b288, sizeof(forwardList1b288));
	// else
	memcpy(_scriptVariables + 352, forwardList1b, sizeof(forwardList1b));

	memcpy(_scriptVariables + 656, forwardList2b, sizeof(forwardList2b));
	memcpy(_scriptVariables + 721, forwardList3b, sizeof(forwardList3b));
	memcpy(_scriptVariables + 663, forwardList4b, sizeof(forwardList4b));
	memcpy(_scriptVariables + 505, forwardList5b, sizeof(forwardList5b));
}
