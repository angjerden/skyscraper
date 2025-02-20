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

        // Write palette (VGA uses RGB but BMP expects RGBA)
        for (int i = 0; i < 256; i++) {
            file.put(palette[i * 3 + 2]); // Blue
            file.put(palette[i * 3 + 1]); // Green
            file.put(palette[i * 3 + 0]); // Red
            file.put(0);                  // Reserved (0)
        }

        // Write pixel data (BMP stores rows bottom-up, so we flip it)
        for (int y = 199; y >= 0; y--) {
            file.write(reinterpret_cast<const char*>(image + y * 320), 320);
        }

        file.close();
        std::cout << "BMP file saved: " << filename << std::endl;
    }

}
