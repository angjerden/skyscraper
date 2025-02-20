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

#include "endian.h"
#include "logic.h"
#include "skydefs.h"
#include "util.h"
#include <string.h>
#include <stdio.h>
#include <iostream>


uint32 Logic::_scriptVariables[NUM_SKY_SCRIPTVARS];

void Logic::setupLogicTable() {
	// static const LogicTable logicTable[] = {
	// 	&Logic::nop,
	// 	&Logic::logicScript,	 // 1  script processor
	// 	&Logic::autoRoute,	 // 2  Make a route
	// 	&Logic::arAnim,	 // 3  Follow a route
	// 	&Logic::arTurn,	 // 4  Mega turns araound
	// 	&Logic::alt,		 // 5  Set up new get-to script
	// 	&Logic::anim,	 // 6  Follow a sequence
	// 	&Logic::turn,	 // 7  Mega turning
	// 	&Logic::cursor,	 // 8  id tracks the pointer
	// 	&Logic::talk,	 // 9  count down and animate
	// 	&Logic::listen,	 // 10 player waits for talking id
	// 	&Logic::stopped,	 // 11 wait for id to move
	// 	&Logic::choose,	 // 12 wait for player to click
	// 	&Logic::frames,	 // 13 animate just frames
	// 	&Logic::pause,	 // 14 Count down to 0 and go
	// 	&Logic::waitSync,	 // 15 Set to l_script when sync!=0
	// 	&Logic::simpleAnim,	 // 16 Module anim without x,y's
	// };

	// _logicTable = logicTable;
}

// Logic::Logic(SkyCompact *skyCompact, Screen *skyScreen, Disk *skyDisk, Text *skyText, MusicBase *skyMusic, Mouse *skyMouse, Sound *skySound)
Logic::Logic(SkyCompact *skyCompact, Disk *skyDisk) {

	_skyCompact = skyCompact;
	// _skyScreen = skyScreen;
	_skyDisk = skyDisk;
	// _skyText = skyText;
	// _skyMusic = skyMusic;
	// _skySound = skySound;
	// _skyMouse = skyMouse;
	// _skyGrid = new Grid(_skyDisk, _skyCompact);
	// _skyAutoRoute = new AutoRoute(_skyGrid, _skyCompact);

	setupLogicTable();
	setupMcodeTable();

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
