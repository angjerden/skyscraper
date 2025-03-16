#include "types.h"
#include "compact.h"
#include "disk.h"
#include "logic.h"
#include "writer.h"
#include <fstream>

#ifndef SKY_SCRAPER_H
#define SKY_SCRAPER_H

class Disk;
class SkyCompact;
class Logic;


class Scraper {
    public:
        Scraper(char* skyPath);
        ~Scraper();
        void scrapeAssetsLogically();
        void scrapeAssetsHardcoded();
        void writeDinnerTableToFile();
        void writeCompactsToFile();
        void scrapeMIDI();
    protected:
        Disk* _skyDisk;
        SkyCompact* _skyCompact;
        Logic* _skyLogic;
};

#endif