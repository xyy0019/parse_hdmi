#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>

#include "parse_hdmi.h"
//#define SHOW

void saveRgbBin(const std::string& jpgPath, const std::string& binPath) {
    // 读取JPG文件
    cv::Mat image = cv::imread(jpgPath, cv::IMREAD_COLOR);
 
    // 检查图像是否成功加载
    if (image.empty()) {
        throw std::runtime_error("Failed to load image from " + jpgPath);
    }
 
    // OpenCV默认读取为BGR格式，需要转换为RGB格式
    cv::Mat rgbImage;
    cv::cvtColor(image, rgbImage, cv::COLOR_BGR2RGB);
 
    // 将Mat数据转换为字节向量
    std::vector<uchar> rgbData;
    rgbImage.reshape(1, 1).copyTo(rgbData); // reshape为1行，然后拷贝到向量中
 
    // 打开二进制文件用于写入
    std::ofstream binFile(binPath, std::ios::binary);
 
    // 检查文件是否成功打开
    if (!binFile.is_open()) {
        throw std::runtime_error("Failed to open binary file for writing: " + binPath);
    }
 
    // 将字节向量写入二进制文件
    binFile.write(reinterpret_cast<const char*>(rgbData.data()), rgbData.size());
 
    // 关闭文件
    binFile.close();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return -1;
	}
	int select = atoi(argv[1]);
	if (select == 0) {
		hdmi_parse(argv[2], argv[3]);
	} else if (select == 1) {
		hdmi_encode(argv[2], argv[3], argv[4]);
	} else if (select == 2) {
		const char *directory = argv[2];
		struct dirent *entry;
		DIR *dp = opendir(directory);
		std::string windowName = "Video Stream from VSB HDMI";
    	cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
		std::string filenamePrefix = "./output_file/frame";
    	std::string filenameSuffix = ".bmp";
    	int startFrame = 0;
		
		if (dp == NULL) {
			perror("opendir");
			exit(EXIT_FAILURE);
		}
 
		// 遍历目录
		int i = 1;
		while ((entry = readdir(dp))) {
			// 检查文件扩展名是否为.dat
			size_t len = strlen(entry->d_name);
			if (len > 4 && strcmp(entry->d_name + len - 4, ".dat") == 0) {
				// 构造完整的文件路径
				char filePath[1024];
				snprintf(filePath, sizeof(filePath), "%s/frame%05d.dat", directory, i);
 				char output_file[1024];
				
				snprintf(output_file, sizeof(output_file), "%s/frame%05d.bmp", directory, i);
				// 调用hdmi_parse函数
				hdmi_parse(filePath, output_file);
        		// 读取图片
        		cv::Mat frame = cv::imread(output_file, cv::IMREAD_COLOR);
 
        		// 检查图片是否成功加载
        		if (frame.empty()) {
            		break; // 如果图片加载失败，则退出循环
        		}
 
        		// 显示图片
        		cv::imshow(windowName, frame);
 
        		int delay = 42; // 例如，设置为30毫秒延迟以大约33fps的速度播放
        		char key = (char)cv::waitKey(delay);
 				i++;
        		// 检查是否按下了退出键（例如ESC键）
        		if (key == 27) {
            		break;
        		}
			}
		}
	}else {
		const char *directory = argv[2];
		char *model = argv[3];
		struct dirent *entry;
		DIR *dp = opendir(directory);
		int i = 1;
		if (dp == NULL) {
			perror("opendir");
			exit(EXIT_FAILURE);
		}
		// 遍历目录
		while ((entry = readdir(dp))) {
			size_t len = strlen(entry->d_name);
	        if (len > 4 && strcmp(entry->d_name + len - 4, ".jpg") == 0) {
	            // 构造完整的文件路径
	            char jpg_path[1024];
	            char dat_path[1024];
				char bin_path[1024];
				snprintf(jpg_path, sizeof(jpg_path), "%s/%05d.jpg", directory, i);
	            snprintf(bin_path, sizeof(bin_path), "%s/frame%05d.bin", directory, i);
	            // 提取文件名中的数字部分，并构造对应的 .dat 文件名
	            saveRgbBin(jpg_path, bin_path);
	            snprintf(dat_path, sizeof(dat_path), "%s/frame%05d.dat", "output_file", i);
	            // 调用处理函数
	            hdmi_encode(model, bin_path, dat_path);
				i++;
	        }
		}
	}
	
#ifdef SHOW
	if (select == 0)
		system("python3 show.py");
#endif
	return 0;
}

