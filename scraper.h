#include "types.h"
#include "compact.h"
#include "disk.h"
#include "logic.h"
#include "sound.h"
#include "text.h"
#include "writer.h"
#include <fstream>

#ifndef SKY_SCRAPER_H
#define SKY_SCRAPER_H

class Disk;
class SkyCompact;
class Logic;
class Text;
class Sound;

class Scraper {
    public:
        Scraper(char* skyPath);
        ~Scraper();
        void scrapeAssetsByRunningTheWholeDamnEngine();
        void scrapeAssetsFromCompacts();
        void scrapeAssetsHardcoded();
        void writeDinnerTableToFile();
        void writeCompactsToFile();
        void scrapeMIDI();
        void scrapeTextAndSpeech();
    protected:
        Disk* _skyDisk;
        SkyCompact* _skyCompact;
        Logic* _skyLogic;
        Text* _skyText;
        Sound* _skySound;
};

#endif