#include "types.h"
#include "compact.h"
#include "disk.h"
#include "logic.h"
#include "writer.h"
#include <fstream>

class Disk;
class SkyCompact;
class Logic;


class Scraper {
    public:
        Scraper(char* skyPath);
        ~Scraper();
        void scrapeAssetsLogically();
        void scrapeAssetsHardcoded();
    protected:
        Disk* _skyDisk;
        SkyCompact* _skyCompact;
        Logic* _skyLogic;
};