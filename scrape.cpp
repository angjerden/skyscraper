/*
  Scraping assets from Beneath A Steel Sky v0.0372 CD/DOS
*/

#include "scraper.h"

int main(int argc, char **argv){
    char* skyPath = NULL;
    if (argc == 2) {
      skyPath = argv[1];
    }

    std::cout << __cplusplus << std::endl;

    Scraper* scraper = new Scraper(skyPath);
    // scraper->scrapeAssetsByRunningTheWholeDamnEngine();
    // scraper->scrapeAssetsHardcoded();
    // scraper->writeDinnerTableToFile();
    // scraper->writeCompactsToFile();
    // scraper->scrapeMIDI();
    // scraper->scrapeAssetsFromCompacts();
    scraper->scrapeTextAndSpeech();

    // Loop through each entry in sky.dsk
    // Store entry in some data structure
    // Write result to file in clear text
    return 0;
}

