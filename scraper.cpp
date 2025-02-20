#include "scraper.h"
#include "writer.h"

Scraper::Scraper(char* skyPath) {
    _skyDisk = new Disk(skyPath);
    _skyCompact = new SkyCompact(skyPath);
    _skyLogic = new Logic(_skyCompact, _skyDisk);

}

Scraper::~Scraper() {
    delete _skyCompact;
    delete _skyDisk;
    delete _skyLogic;
}

void Scraper::scrapeAssetsLogically() {
    _skyLogic->initScreen0();
}

// get hardcoded assets from the intro and elsewhere
void Scraper::scrapeAssetsHardcoded() {
    // virgin palette
    uint8* virginPal = _skyDisk->loadFile(60111);
    // virgin screen
    uint8* virginScr = _skyDisk->loadFile(60110);

    Writer::writeBMP("virgin.bmp", virginScr, virginPal);

    uint8* shamanPal = _skyDisk->loadFile(59501);
    uint8* shamanScr = _skyDisk->loadFile(59502);

    Writer::writeBMP("shaman.bmp", shamanScr, shamanPal);
    // linc mouse cursors
    uint8* lincCursors = _skyDisk->loadFile(60302);
}