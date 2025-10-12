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

    // Function to save 320x200 VGA image with a 256-color palette
    void writeBMP(const char* filename, uint8* image, uint8* palette) {
        BMPHeader bmpHeader;
        DIBHeader dibHeader;
        
        bmpHeader.bfOffBits = sizeof(BMPHeader) + sizeof(DIBHeader) + 256 * 4;
        bmpHeader.bfSize = bmpHeader.bfOffBits + dibHeader.biSizeImage;

        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Unable to open file for writing!" << std::endl;
            return;
        }

        // Write headers
        file.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(bmpHeader));
        file.write(reinterpret_cast<const char*>(&dibHeader), sizeof(dibHeader));

        uint8 fullyFadedUp = 32;
        byte tmpPal[VGA_COLORS * 3];

        convertPalette(palette, tmpPal);
        
        // Write palette (VGA uses RGB but BMP expects RGBA)
        for (int i = 0; i < VGA_COLORS; i++) {
            file.put((tmpPal[i * 3 + 2] * fullyFadedUp) >> 5); // Blue
            file.put((tmpPal[i * 3 + 1] * fullyFadedUp) >> 5); // Green
            file.put((tmpPal[i * 3 + 0] * fullyFadedUp) >> 5); // Red
            file.put(0);                  // Reserved (0)
        }

        // Write pixel data (BMP stores rows bottom-up, so we flip it)
        for (int y = 199; y >= 0; y--) {
            file.write(reinterpret_cast<const char*>(image + y * 320), 320);
        }

        file.close();
        std::cout << "BMP file saved: " << filename << std::endl;
    }

    void convertPalette(uint8 *inPal, uint8* outPal) {
        int i;

        for (i = 0; i < VGA_COLORS; i++) {
            outPal[3 * i + 0] = (inPal[3 * i + 0] << 2) + (inPal[3 * i + 0] >> 4);
            outPal[3 * i + 1] = (inPal[3 * i + 1] << 2) + (inPal[3 * i + 1] >> 4);
            outPal[3 * i + 2] = (inPal[3 * i + 2] << 2) + (inPal[3 * i + 2] >> 4);
        }
}

}
