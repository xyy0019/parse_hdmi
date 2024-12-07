#include <iostream>
#include <fstream>
#include <vector>
#include "bmp.h"
void readRgbDataAndCreateImage(const std::string& inputBinFile, const std::string& outputBmpFile, int width, int height) {
	std::ifstream binFile(inputBinFile, std::ios::binary);
	if (!binFile.is_open()) {
		std::cerr << "Error: The file '" << inputBinFile << "' could not be opened." << std::endl;
		return;
	}
 
	std::vector<uint8_t> bgrData(width * height * 4);
 
	for (int y = height - 1; y >= 0; --y) {
		std::vector<uint8_t> rowBuffer(width * 4);
		if (!binFile.read(reinterpret_cast<char*>(rowBuffer.data()), rowBuffer.size())) {
			std::cerr << "Error: Not enough data in '" << inputBinFile << "' to fill the image." << std::endl;
			return;
		}
 
		for (int x = 0; x < width; ++x) {
			bgrData[(y * width + x) * 3 + 0] = rowBuffer[(x * 4 + 3)]; // B
			bgrData[(y * width + x) * 3 + 1] = rowBuffer[(x * 4 + 2)]; // G
			bgrData[(y * width + x) * 3 + 2] = rowBuffer[(x * 4 + 1)]; // R
		}
	}
	binFile.close();
 
	BMPFileHeader fileHeader;
	BMPInfoHeader infoHeader;
	fileHeader.bfType = 0x4D42;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
 
	infoHeader.biSize = sizeof(BMPInfoHeader);
	infoHeader.biWidth = width;
	infoHeader.biHeight = height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 24;
	infoHeader.biCompression = 0;
	infoHeader.biSizeImage = width * height * 3;
	infoHeader.biXPelsPerMeter = 0;
	infoHeader.biYPelsPerMeter = 0;
	infoHeader.biClrUsed = 0;
	infoHeader.biClrImportant = 0;
 
	fileHeader.bfSize = fileHeader.bfOffBits + infoHeader.biSizeImage;
 
	std::ofstream bmpFile(outputBmpFile, std::ios::binary);
	if (!bmpFile.is_open()) {
		std::cerr << "Error: The file '" << outputBmpFile << "' could not be created." << std::endl;
		return;
	}
 
	bmpFile.write(reinterpret_cast<const char*>(&fileHeader), sizeof(BMPFileHeader));
	bmpFile.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));
	bmpFile.write(reinterpret_cast<const char*>(bgrData.data()), bgrData.size());
 
	bmpFile.close();
	std::cout << "BMP file created successfully: " << outputBmpFile << std::endl;
}

