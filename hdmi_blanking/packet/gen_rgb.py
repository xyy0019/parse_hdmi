from PIL import Image

def extract_rgb_interleaved(image_path, output_bin_path):
    # 打开JPEG图像
    with Image.open(image_path) as img:
        # 确保图像是RGB模式
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        # 获取图像的宽度和高度
        width, height = img.size
        
        # 创建一个空列表来存储交织的RGB数据
        interleaved_data = []
        
        # 遍历图像的每个像素
        for y in range(height):
            for x in range(width):
                # 获取当前像素的RGB值
                r, g, b = img.getpixel((x, y))
                # 将RGB值交织添加到列表中
                interleaved_data.append(r)
                interleaved_data.append(g)
                interleaved_data.append(b)
        
        # 将交织的数据转换为字节数组
        interleaved_bytes = bytearray(interleaved_data)
        
        # 将字节数组写入二进制文件
        with open(output_bin_path, 'wb') as bin_file:
            bin_file.write(interleaved_bytes)
        
        print(f"RGB交织数据已写入 {output_bin_path}")

# 示例用法
image_path = 'test1.jpg'  # 输入的JPEG图像路径
output_bin_path = 'output.bin'  # 输出的二进制文件路径
extract_rgb_interleaved(image_path, output_bin_path)
