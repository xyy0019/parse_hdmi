#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
// BMP 文件头结构
#pragma pack(push, 1)
struct BMPFileHeader {
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
};
 
// BMP 信息头结构
struct BMPInfoHeader {
	uint32_t biSize;	   
	int32_t  biWidth;	   
	int32_t  biHeight;	   
	uint16_t biPlanes;	   
	uint16_t biBitCount;   
	uint32_t biCompression;
	uint32_t biSizeImage;  
	int32_t  biXPelsPerMeter;
	int32_t  biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};
#pragma pack(pop)
// 输入和输出文件路径
const std::string output_file_path = "1.bin";
const int depth = 4;

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

int main(int argc, char* argv[]) {
	// 打开输入文件
	int htotal;  // 输入文件的宽度（列数）
	int vtotal;  // 输入文件的高度（行数）
	int start_col;	// 起始列（横坐标）
	int num_cols;  // 每行要读取的列数（像素数）
	int start_row;	// 起始行（纵坐标）
	std::string outputImageFile = "output.bmp";
	std::string input_file_path = argv[1];
	std::ifstream input_file(input_file_path, std::ios::binary);
	if (!input_file) {
		std::cerr << "Failed to open input file." << std::endl;
		return 1;
	}

	// 打开输出文件
	std::ofstream output_file(output_file_path, std::ios::binary);
	if (!output_file) {
		std::cerr << "Failed to open output file." << std::endl;
		input_file.close();
		return 1;
	}
	htotal = atoi(argv[2]);
	vtotal = atoi(argv[3]);
	int hactive = atoi(argv[4]);
	int vactive = atoi(argv[5]);
	num_cols = hactive;
	start_col = htotal - hactive;
	start_row = vtotal - vactive;
	// 计算每行要读取的字节数
	const std::streamsize bytes_per_row = num_cols * depth;

	// 读取并写入数据
	for (int row = start_row; row < vtotal; ++row) {
		// 移动到当前行的起始位置（考虑起始列和每个像素的字节数）
		input_file.seekg((row * htotal + start_col) * depth, std::ios::beg);

		// 准备一个缓冲区来存储读取的数据
		std::vector<char> buffer(bytes_per_row);

		// 读取当前行的数据
		input_file.read(buffer.data(), bytes_per_row);

		// 检查是否读取成功（可能由于到达文件末尾而失败）
		if (input_file.gcount() != bytes_per_row) {
			// 如果读取的字节数少于预期的bytes_per_row，则可能是到达了文件末尾
			break;	// 跳出循环
		}

		// 将读取的数据写入输出文件
		output_file.write(buffer.data(), bytes_per_row);
	}

	// 关闭文件
	input_file.close();
	output_file.close();

	readRgbDataAndCreateImage(output_file_path, outputImageFile, hactive, vactive);
	return 0;
}
