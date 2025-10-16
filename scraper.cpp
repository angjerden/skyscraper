#include "scraper.h"
#include "writer.h"

Scraper::Scraper(char* skyPath) {
    _skyDisk = new Disk(skyPath);
    _skyCompact = new SkyCompact(skyPath);
    _skyText = new Text(_skyDisk, _skyCompact);
    _skyLogic = new Logic(_skyCompact, _skyDisk, _skyText);
    _skySound = new Sound(_skyDisk);

}

Scraper::~Scraper() {
    delete _skyCompact;
    delete _skyDisk;
    delete _skyLogic;
    delete _skyText;
}

void Scraper::scrapeAssetsByRunningTheWholeDamnEngine() {
    _skyLogic->initScreen0();

    // get texts and speeches
    // uint16* talkTable = (uint16*)_skyCompact->fetchCpt(CPT_TALK_TABLE_LIST);

    _skyLogic->engine();
}

// get hardcoded assets from the intro and elsewhere
void Scraper::scrapeAssetsHardcoded() {
    uint8* virginPal = _skyDisk->loadFile(60111);
    uint8* virginScr = _skyDisk->loadFile(60110);

    Writer::writeBMP("virgin.bmp", virginScr, virginPal);

    uint8* shamanPal = _skyDisk->loadFile(59501);
    // uint8* shamanScr = _skyDisk->loadFile(59502);

    // Writer::writeBMP("shaman.bmp", shamanScr, shamanPal);
    // linc mouse cursors
    uint8* lincCursors = _skyDisk->loadFile(60302);

    uint8* file49 = _skyDisk->loadFile(49);
    Writer::writeBMP("file49.bmp", file49, shamanPal);

    uint8* startRoom = _skyDisk->loadFile(64);
    uint8* startRoomPal = (uint8*)_skyCompact->fetchCpt(4316);
    Writer::writeBMP("file64.bmp", startRoom, startRoomPal);

    uint8* securityTerrace = _skyDisk->loadFile(92);
    uint8* securityTerracePal = (uint8*)_skyCompact->fetchCpt(4317);
    Writer::writeBMP("file92.bmp", securityTerrace, securityTerracePal);

    uint8* sprite89 = _skyDisk->loadFile(89);
    Writer::writeBMP("sprite89.bmp", sprite89, shamanPal);
}

void Scraper::writeDinnerTableToFile() {
    _skyDisk->writeDinnerTableToFile();
}

void Scraper::writeCompactsToFile() {
    _skyCompact->writeCompactsToFile();
}

// Not working at all at the moment
void Scraper::scrapeMIDI() {
    uint16 driverFileBase = 60202;
    uint8 FILES_PER_SECTION = 4;
    uint8 section = 1;
    uint8 currentMusic = 2; // song

    // musicbase.loadSection() - load music data for section
    uint8* _musicData = _skyDisk->loadFile(driverFileBase + FILES_PER_SECTION * section);
    uint32 fileSize = _skyDisk->_lastLoadedFileSize;
    std::ofstream outfile("midifile.mid", std::ios::binary);
    outfile.write(reinterpret_cast<char*>(_musicData), fileSize);
    outfile.close();

    // adlibmusic.setupPointers()
    uint16 _musicDataLoc = READ_LE_UINT16(_musicData + 0x1201);
    uint8* _initSequence = _musicData + 0xE91;

    // musicbase.loadNewMusic()
    uint16 musicPos = READ_LE_UINT16(_musicData + _musicDataLoc + 1);
    musicPos += _musicDataLoc + ((currentMusic - 1) << 1);
    musicPos = READ_LE_UINT16(_musicData + musicPos) + _musicDataLoc;

    // in function call to adlibmusic.setupChannels()
    uint8* channelData = _musicData + musicPos + 2;

    // adlibmusic.setupChannels()
    uint16 numberOfChannels = channelData[0];
    channelData++;
    for (int i = 0; i < numberOfChannels; i++) {
        uint16 channelDataStart = READ_LE_UINT16((uint16*)channelData + i) + _musicDataLoc;
        // _channels
    }
}

void Scraper::scrapeAssetsFromCompacts() {
    _skyLogic->scrapeAssetsFromCompacts();
}

void Scraper::scrapeTextAndSpeech() {
    // loop through each section, there are 8 sections
    std::map<uint16, uint16> numTextsPerSection {
        {0, 564}, 
        {1, 483}, 
        {2, 1303},
        {3, 922}, 
        {4, 1140},
        {5, 531},  
        {6, 120}, 
        {7, 96}   
    };

    std::ofstream outfile;
    outfile.open("texts.txt");

    for (const auto& [sectionNo, numTexts] : numTextsPerSection) {
        std::map<uint32, std::string> textMap = _skyText->scrapeTextForSection(sectionNo, numTexts);
        for (const auto& [textNr, text] : textMap) {
            outfile << textNr << " " << text << std::endl;

            uint8* speechData = _skySound->getSpeech(textNr);
            if (speechData != NULL) {
                Writer::writeWav("speech//" + std::to_string(textNr) + ".wav", speechData);
            }
        }
    }
    outfile.close();
}