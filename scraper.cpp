#include "scraper.h"

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

void Scraper::scrapeAssetsHardcoded() {

}