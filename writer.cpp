#include "struc.h"
#include "writer.h"
#include <fstream>
namespace Writer {
    void writeWav(std::string fileName, uint32 fileSize, uint8* speechData){
        std::ofstream outfile;
        outfile.open(fileName, std::ios::binary);

        uint16 num_channels = 1;
        uint32 sample_rate = 11025;
        uint16 bits_per_sample = 8;

        // Create WAV header
        WavHeader header = {
        {'R', 'I', 'F', 'F'},
        sizeof(WavHeader) - 8 + fileSize,
        {'W', 'A', 'V', 'E'},
        {'f', 'm', 't', ' '},
        16,
        1, // PCM
        num_channels,
        sample_rate,
        num_channels * sample_rate * bits_per_sample / 8,
        num_channels * bits_per_sample / 8,
        bits_per_sample,
        {'d', 'a', 't', 'a'},
        fileSize
        };

        if (!outfile.is_open()) {
            std::cout << "Failed to open " << fileName << std::endl;
        } else {
            outfile.write(reinterpret_cast<char*>(&header), sizeof(WavHeader));
            outfile.write(reinterpret_cast<char*>(speechData), fileSize);
        }
        outfile.close();
        std::cout << "Wrote wav file " << fileName << std::endl;
    }
}
