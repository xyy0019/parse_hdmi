#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include "bmp.h"

uint64_t bmp_reverse_byte(uint8_t byte) {
	return byte;
	uint8_t reversed = 0;
	for (int i = 0; i < 8; i++) {
		reversed <<= 1;
		reversed |= (byte & 1);
		byte >>= 1;
	}
	return reversed;
}

// 转换20字节缓冲区为4个像素的函数
std::vector<uint8_t> process_buffer(const uint8_t* buffer) {
    std::vector<uint8_t> pixels(12); // 每个像素3个字节（R, G, B），共4个像素
    // 提取R、G、B分量
        // 注意：这里我们实际上只使用了每个颜色通道的前8位，忽略了低2位
    uint64_t r = bmp_reverse_byte(buffer[1]) << 0 |
    bmp_reverse_byte(buffer[5]) << 8 |
    bmp_reverse_byte(buffer[9]) << 16 |
    bmp_reverse_byte(buffer[13]) << 24 |
    bmp_reverse_byte(buffer[17]) << 32;
	uint16_t r1 = r >> 30 >> 2;
	uint16_t r2 = ((r >> 20) & 0x3ff) >> 2;
	uint16_t r3 = ((r >> 10) & 0x3ff) >> 2;
	uint16_t r4 = (r & 0x3ff) >> 2;

	uint64_t g = bmp_reverse_byte(buffer[2]) << 0 |
    bmp_reverse_byte(buffer[6]) << 8 |
    bmp_reverse_byte(buffer[10]) << 16 |
    bmp_reverse_byte(buffer[14]) << 24 |
    bmp_reverse_byte(buffer[18]) << 32;
	uint16_t g1 = g >> 30 >> 2;
	uint16_t g2 = ((g >> 20) & 0x3ff) >> 2;
	uint16_t g3 = ((g >> 10) & 0x3ff) >> 2;
	uint16_t g4 = (g & 0x3ff) >> 2;

	uint64_t b = bmp_reverse_byte(buffer[3]) << 0 |
    bmp_reverse_byte(buffer[7]) << 8 |
    bmp_reverse_byte(buffer[11]) << 16 |
    bmp_reverse_byte(buffer[15]) << 24 |
    bmp_reverse_byte(buffer[19]) << 32;
	uint16_t b1 = b >> 30 >> 2;
	uint16_t b2 = ((b >> 20) & 0x3ff) >> 2;
	uint16_t b3 = ((b >> 10) & 0x3ff) >> 2;
	uint16_t b4 = (b & 0x3ff) >> 2;
    // 将R、G、B值放入像素数组中
    pixels[0] = b1;
    pixels[1] = g1;
    pixels[2] = r1;

	pixels[3] = b2;
    pixels[4] = g2;
    pixels[5] = r2;

	pixels[6] = b3;
    pixels[7] = g3;
    pixels[8] = r3;

	pixels[9] = b4;
    pixels[10] = g4;
    pixels[11] = r4;
    return pixels;
}
 
// 创建BMP文件的函数
void create_bmp_from_file(const std::string& inputBinFile, const std::string& outputBmpFile, int hactive, int vactive) {
    std::ifstream infile(inputBinFile, std::ios::binary);
    if (!infile) {
        std::cerr << "Error opening input file!" << std::endl;
        return;
    }
 
    std::vector<uint8_t> pixel_data;
    uint8_t buffer[20];
 
    // 读取文件并处理缓冲区
    while (infile.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
        std::vector<uint8_t> pixels = process_buffer(buffer);
        pixel_data.insert(pixel_data.end(), pixels.begin(), pixels.end());
    }
 
    // 确保文件已正确关闭
    infile.close();
 
    // 设置BMP文件头和信息头
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    memset(&fileHeader, 0, sizeof(fileHeader));
    memset(&infoHeader, 0, sizeof(infoHeader));
 
    fileHeader.bfType = 0x4D42; // "BM"
    infoHeader.biSize = sizeof(infoHeader);
    infoHeader.biWidth = hactive; // 假设每个像素一行，但您可以根据需要调整宽度
    infoHeader.biHeight = vactive; // 高度为像素数除以3（每个像素3个字节）
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24; // 每个像素24位（R, G, B各8位）
    infoHeader.biSizeImage = pixel_data.size(); // 图像数据大小
 
    // 计算文件大小（文件头 + 信息头 + 图像数据）
    fileHeader.bfSize = sizeof(fileHeader) + sizeof(infoHeader) + infoHeader.biSizeImage;
    fileHeader.bfOffBits = sizeof(fileHeader) + sizeof(infoHeader);
 
    // 打开输出文件并写入BMP头和数据
    std::ofstream outfile(outputBmpFile, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error opening output file!" << std::endl;
        return;
    }

	int row_size = (infoHeader.biWidth * 3 + 3) & ~3;
 
    // 创建一个新的数组来存储翻转后的像素数据
    std::vector<uint8_t> flipped_pixel_data(pixel_data.size());
 
    // 复制并翻转像素数据
    for (int y = 0; y < infoHeader.biHeight; ++y) {
        int source_index = (infoHeader.biHeight - 1 - y) * row_size;
        int dest_index = y * row_size;
        memcpy(flipped_pixel_data.data() + dest_index, pixel_data.data() + source_index, row_size);
    }
    outfile.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    outfile.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));
    outfile.write(reinterpret_cast<const char*>(flipped_pixel_data.data()), pixel_data.size());
 
    // 确保文件已正确关闭
    outfile.close();
}

void readRgbDataAndCreateImage(const std::string& inputBinFile, const std::string& outputBmpFile, int width, int height) {
	std::ifstream binFile(inputBinFile, std::ios::binary);
	if (!binFile.is_open()) {
		std::cerr << "Error: The file '" << inputBinFile << "' could not be opened." << std::endl;
		return;
	}
 
	std::vector<uint8_t> bgrData(width * height * 3);
 
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

