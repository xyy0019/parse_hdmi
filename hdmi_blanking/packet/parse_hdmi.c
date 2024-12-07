#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "pktinfo.h"
#include "div.h"
#include "bmp.h"

#define GUARD_BAND_SIZE 8
#define PACKET_SIZE (32 * 4)
#define SEARCH_PATTERN {0x40, 0x01, 0x01, 0x00}
#define VIDEO_SEARCH_PATTERN {0xc0}

 
bool compare_bytes(const uint8_t *bytes1, const uint8_t *bytes2, size_t size) {
	for (size_t i = 0; i < size; i++) {
		if (bytes1[i] != bytes2[i]) {
			return false;
		}
	}
	return true;
}


uint8_t reverse_byte(uint8_t byte) {
	uint8_t reversed = 0;
	for (int i = 0; i < 8; i++) {
		reversed <<= 1;
		reversed |= (byte & 1);
		byte >>= 1;
	}
	return reversed;
}

uint32_t reverse_bytes_in_32bit(uint32_t num) {
	uint32_t reversed = 0;
	for (int i = 0; i < 4; i++) {
		uint8_t byte = (num >> (i * 8)) & 0xFF;
		uint8_t reversed_byte = reverse_byte(byte);
		reversed |= reversed_byte << (i * 8);
	}
	return reversed;
}

long getFileSize(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return -1;
    }
 
    // 移动文件指针到文件末尾
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Failed to seek to end of file");
        fclose(file);
        return -1;
    }
 
    // 获取当前文件指针的位置，即文件大小
    long size = ftell(file);
    if (size == -1L) {
        perror("Failed to get file size");
    }
 
    fclose(file);
    return size;
}



void hdmi_parse_packet(uint8_t (*data)[4])
{
	uint32_t head = 0;
	uint32_t body_low32 = 0;
	uint32_t body_high32 = 0;
	for (int i = 0; i < 32; i++) {
		uint8_t fourth_byte = data[i][3];
		uint8_t bit2 = (fourth_byte >> 2) & 1;
		head |= bit2 << (31 - i);
	}

	char binary_str[33];
	char strQ[1000];
	char strR[1000];

	head = reverse_bytes_in_32bit(head);
	int_to_binary_string(head >> 8, binary_str);
	strm2div(binary_str, "111000001", strQ, strR);
	printf("ecc %s\n", strR);
	printf("head 32-bit result: 0x%08X\n", head);
	int bit_position = 0;
	for (int i = 0; i < 16; i++) {
		uint8_t byte2_bit0 = (data[i][2] & 1);
		body_low32 |= (byte2_bit0 << (31 - bit_position));
		bit_position++; 
		uint8_t byte3_bit0 = (data[i][1] & 1);
		body_low32 |= (byte3_bit0 << (31 - bit_position));
		bit_position++; 
	}
	body_low32 = reverse_bytes_in_32bit(body_low32);
	printf("body_low 32-bit result: 0x%08X\n", body_low32);
	
	bit_position = 0;
 
	for (int i = 16; i < 32; i++) {
		uint8_t byte2_bit0 = (data[i][2] & 1);
		body_high32 |= (byte2_bit0 << (31 - bit_position));
		bit_position++; 
		uint8_t byte3_bit0 = (data[i][1] & 1);
		body_high32 |= (byte3_bit0 << (31 - bit_position));
		bit_position++;
	}
	body_high32 = reverse_bytes_in_32bit(body_high32);
	printf("body_high 32-bit result: 0x%08X\n", body_high32);
	hdmi_parse_pkt_info(&head, &body_low32, &body_high32);
}

int hdmi_parse_blank_packet_and_fdet(char *input_file, int *htotal, int *hactive, int *vtotal, int *vactive)
{
	FILE *file = fopen(input_file, "rb");
	if (!file) {
		perror("Failed to open file");
		return -1;
	}
	int h_cnt;
	int h_b_cnt;
	int v_cnt = 0;
	int hactive_detect = 0;
	int hblank_detect = 0;
	long frame_pixel = getFileSize(input_file);
	uint8_t pkt_data[32][4];
	unsigned char buffer[4 + GUARD_BAND_SIZE + PACKET_SIZE];
	unsigned char search_pattern[4] = SEARCH_PATTERN;
	unsigned char guard_band[GUARD_BAND_SIZE] = {0x05, 0x33, 0x33, 0x0c, 0x05, 0x33, 0x33, 0x0c};
	unsigned char video_search_pattern[1] = VIDEO_SEARCH_PATTERN;
	int pattern_count = 0; // Counter for the search pattern occurrences
	int hactive_count = 0;
	int hblank_count = 0;
	long offset;
	bool first = true;
	while (1) {
		size_t bytes_read = fread(buffer, 1, 4, file);
		if (bytes_read != 4) {
			if (feof(file)) {
				break; // End of file reached
			} else {
				perror("Failed to read data");
				fclose(file);
				return -1;
			}
		}
 
		if (memcmp(buffer, search_pattern, 3) == 0) {
			pattern_count++;
			// Check if this is the 8th (or any other specific) occurrence
			// For simplicity, let's assume we're looking for the 8th non-consecutive occurrence
			// In a real-world scenario, you might have a more complex logic for this
			if (pattern_count == 8) { // Adjust the condition as needed
				// Read guard band
				size_t guard_bytes_read = fread(buffer, 1, GUARD_BAND_SIZE, file);
				if (guard_bytes_read != GUARD_BAND_SIZE) {
					fprintf(stderr, "Guard band mismatch\n");
					fclose(file);
					return -1;
				}
 
				// Read packet
				size_t packet_bytes_read = fread(buffer, 1, PACKET_SIZE, file);
				for (int i = 0; i < 32; i++) {
					for (int j = 0; j < 4; j++)
						pkt_data[i][j] = buffer[i * 4 + j];
				}
				hdmi_parse_packet(pkt_data);
				if (packet_bytes_read != PACKET_SIZE) {
					fprintf(stderr, "Failed to read full packet data\n");
					fclose(file);
					return -1;
				}
 
				// Here you can process the packet data stored in `buffer`
				// For demonstration, we'll just print a message
				printf("Packet read successfully\n\n");
				printf("************************\n");
				// Reset the pattern counter since we've already read the packet
				pattern_count = 0; // Optionally, you might want to set a different logic here
 
				// If you only need one packet, you can break the loop here
				// break;
			}
		} else {
			pattern_count = 0; // Reset the counter if the pattern doesn't match
			//memset(buffer, 0, sizeof(buffer));
		}
		if (memcmp(buffer, video_search_pattern, 1) == 0) {
			if (first) {
				offset = ftell(file);
				first = false;
			}
			hactive_detect = 1;
			hactive_count++;
			// 如果当前是消隐期，则退出消隐期
			if (hblank_detect) {
				hblank_detect = 0;
				h_b_cnt = hblank_count;
				hblank_count = 0;  // 可选：如果需要在进入活动期时重置消隐计数器
			}
		} else {
			// 如果之前处于活动期，则现在进入消隐期
			if (hactive_detect) {
				h_cnt = hactive_count;
				hactive_detect = 0;
				hblank_detect = 1;
			}
			// 在消隐期内，增加消隐计数器
			if (hblank_detect) {
				hblank_count++;
			}
			// 重置活动计数器
			hactive_count = 0;
		}

	}
	*htotal = h_b_cnt + h_cnt;
	*vtotal = frame_pixel / *htotal / 4;
	*hactive = h_cnt;
	int vblank = offset / 4 / *htotal;
	*vactive = *vtotal - vblank;
	printf("frame_pixel = %ld\n", frame_pixel);
 	printf("hactive = %d\n", h_cnt);
	printf("hblank = %d\n", h_b_cnt);
	printf("vactive = %d\n", *vactive);
	printf("vblank = %d\n", vblank);
	fclose(file);
	return 0;
}

int hdmi_generate_bmpfile(char *filename, int *htotal, int *vtotal, int *hactive, int *vactive)
{
	int start_col;
	int num_cols;
	int start_row;
	std::string outputImageFile = "output.bmp";
	std::string input_file_path = filename;
	std::ifstream input_file(input_file_path, std::ios::binary);
	if (!input_file) {
		std::cerr << "Failed to open input file." << std::endl;
		return 1;
	}

	// 输入和输出文件路径
	const std::string output_file_path = "1.bin";
	// 打开输出文件
	std::ofstream output_file(output_file_path, std::ios::binary);
	if (!output_file) {
		std::cerr << "Failed to open output file." << std::endl;
		input_file.close();
		return -1;
	}
	num_cols = *hactive;
	start_col = *htotal - *hactive;
	start_row = *vtotal - *vactive;
	const std::streamsize bytes_per_row = num_cols * 4;

	for (int row = start_row; row < *vtotal; ++row) {
		input_file.seekg((row * *htotal + start_col) * 4, std::ios::beg);

		std::vector<char> buffer(bytes_per_row);

		input_file.read(buffer.data(), bytes_per_row);

		if (input_file.gcount() != bytes_per_row) {
			break;
		}

		output_file.write(buffer.data(), bytes_per_row);
	}

	input_file.close();
	output_file.close();

	readRgbDataAndCreateImage(output_file_path, outputImageFile, *hactive, *vactive);
	return 0;
}

int hdmi_parse(char *input_file) {
	int htotal;
	int hactive;
	int vtotal;
	int vactive;
	int ret;

	ret = hdmi_parse_blank_packet_and_fdet(input_file, &htotal, &hactive, &vtotal, &vactive);
	if (ret == -1) {
		std::cerr << "Failed to parse blank." << std::endl;
		return -1;
	}
	ret = hdmi_generate_bmpfile(input_file, &htotal, &vtotal, &hactive, &vactive);
	if (ret == -1) {
		std::cerr << "Failed to gen bmpfile." << std::endl;
		return -1;
	}
	return 0;
}
