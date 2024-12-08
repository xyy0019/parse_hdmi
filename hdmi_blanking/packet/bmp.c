#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include "bmp.h"

uint8_t reverse_byte(uint8_t byte) {
	uint8_t reversed = 0;
	for (int i = 0; i < 8; i++) {
		reversed <<= 1;
		reversed |= (byte & 1);
		byte >>= 1;
	}
	return reversed;
}

uint64_t byte_64(uint8_t byte) {
	return byte;
}

// 转换20字节缓冲区为4个像素的函数
std::vector<uint8_t> process_buffer_10(const uint8_t* buffer) {
    std::vector<uint8_t> pixels(12); // 每个像素3个字节（R, G, B），共4个像素

    uint64_t r = byte_64(buffer[1]) << 0 |
    byte_64(buffer[5]) << 8 |
    byte_64(buffer[9]) << 16 |
    byte_64(buffer[13]) << 24 |
    byte_64(buffer[17]) << 32;
	uint16_t r1 = r >> 30 >> 2;
	uint16_t r2 = ((r >> 20) & 0x3ff) >> 2;
	uint16_t r3 = ((r >> 10) & 0x3ff) >> 2;
	uint16_t r4 = (r & 0x3ff) >> 2;

	uint64_t g = byte_64(buffer[2]) << 0 |
    byte_64(buffer[6]) << 8 |
    byte_64(buffer[10]) << 16 |
    byte_64(buffer[14]) << 24 |
    byte_64(buffer[18]) << 32;
	uint16_t g1 = g >> 30 >> 2;
	uint16_t g2 = ((g >> 20) & 0x3ff) >> 2;
	uint16_t g3 = ((g >> 10) & 0x3ff) >> 2;
	uint16_t g4 = (g & 0x3ff) >> 2;

	uint64_t b = byte_64(buffer[3]) << 0 |
    byte_64(buffer[7]) << 8 |
    byte_64(buffer[11]) << 16 |
    byte_64(buffer[15]) << 24 |
    byte_64(buffer[19]) << 32;
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

// 转换20字节缓冲区为4个像素的函数
std::vector<uint8_t> process_buffer_12(const uint8_t* buffer) {
    std::vector<uint8_t> pixels(6);
    // 提取R、G、B分量
        // 注意：这里我们实际上只使用了每个颜色通道的前8位，忽略了低2位
    uint8_t byte1 = reverse_byte(buffer[1]);
    uint8_t byte2 = reverse_byte(buffer[5]);
	uint8_t byte3 = reverse_byte(buffer[9]);
	uint16_t r1 = (byte1 << 4) | (byte2 >> 4);
	r1 = uint8_t(r1 & 0xff);
	r1 = reverse_byte(r1);
	uint16_t r2 = ((byte2 & 0xff) << 4) | byte3;
	r2 = uint8_t(r2 & 0xff);
	r2 = reverse_byte(r2);

	byte1 = reverse_byte(buffer[2]);
    byte2 = reverse_byte(buffer[6]);
	byte3 = reverse_byte(buffer[10]);
	uint16_t g1 = (byte1 << 4) | (byte2 >> 4);
	g1 = uint8_t(g1 & 0xff);
	g1 = reverse_byte(g1);
	uint16_t g2 = ((byte2 & 0xff) << 4) | byte3;
	g2 = uint8_t(g2 & 0xff);
	g2 = reverse_byte(g2);

	byte1 = reverse_byte(buffer[3]);
    byte2 = reverse_byte(buffer[7]);
	byte3 = reverse_byte(buffer[11]);
	uint16_t b1 = (byte1 << 4) | (byte2 >> 4);
	b1 = uint8_t(b1 & 0xff);
	b1 = reverse_byte(b1);
	uint16_t b2 = ((byte2 & 0xff) << 4) | byte3;
	b2 = uint8_t(b2 & 0xff);
	b2 = reverse_byte(b2);
    // 将R、G、B值放入像素数组中
    pixels[0] = b1;
    pixels[1] = g1;
    pixels[2] = r1;

	pixels[3] = b2;
    pixels[4] = g2;
    pixels[5] = r2;

    return pixels;
}

// 创建BMP文件的函数
void hdmi_create_bmp_from_12bit_data(const std::string& inputBinFile, const std::string& outputBmpFile, int hactive, int vactive) {
    std::ifstream infile(inputBinFile, std::ios::binary);
    if (!infile) {
        std::cerr << "Error opening input file!" << std::endl;
        return;
    }
 
    std::vector<uint8_t> pixel_data;
    uint8_t buffer[12];
 
    // 读取文件并处理缓冲区
    while (infile.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
        std::vector<uint8_t> pixels = process_buffer_12(buffer);
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

// 创建BMP文件的函数
void hdmi_create_bmp_from_10bit_data(const std::string& inputBinFile, const std::string& outputBmpFile, int hactive, int vactive) {
    std::ifstream infile(inputBinFile, std::ios::binary);
    if (!infile) {
        std::cerr << "Error opening input file!" << std::endl;
        return;
    }
 
    std::vector<uint8_t> pixel_data;
    uint8_t buffer[20];
 
    // 读取文件并处理缓冲区
    while (infile.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
        std::vector<uint8_t> pixels = process_buffer_10(buffer);
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

void hdmi_create_bmp_from_8bit_data(const std::string& inputBinFile, const std::string& outputBmpFile, int width, int height) {
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

