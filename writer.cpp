#include "struc.h"
#include "writer.h"
#include <fstream>
namespace Writer {
    void writeWav(std::string fileName, uint8* speechData){
        std::ofstream outfile;
        outfile.open(fileName, std::ios::binary);

        uint32 audioSize = ((DataFileHeader *)speechData)->s_tot_size - sizeof(DataFileHeader);
        uint8* audioData = speechData + sizeof(DataFileHeader);

        uint16 num_channels = 1;
        uint32 sample_rate = 11025;
        uint16 bits_per_sample = 8;

        // Create WAV header
        WavHeader header = {
        {'R', 'I', 'F', 'F'},
        static_cast<uint32_t>(sizeof(WavHeader)) - 8 + audioSize,
        {'W', 'A', 'V', 'E'},
        {'f', 'm', 't', ' '},
        16,
        1, // PCM
        num_channels,
        sample_rate,
        num_channels * sample_rate * bits_per_sample / 8,
        static_cast<uint16_t>(num_channels * bits_per_sample / 8),
        bits_per_sample,
        {'d', 'a', 't', 'a'},
        audioSize
        };

        if (!outfile.is_open()) {
            std::cout << "Failed to open " << fileName << std::endl;
        } else {
            outfile.write(reinterpret_cast<char*>(&header), sizeof(WavHeader));
            outfile.write(reinterpret_cast<char*>(audioData), audioSize);
        }
        outfile.close();
        std::cout << "Wrote wav file " << fileName << std::endl;
    }

    

}
