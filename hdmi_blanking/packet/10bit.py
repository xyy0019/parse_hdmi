from PIL import Image

def reverse_bytes(byte_list):
    """
    反转字节列表。
    """
    return byte_list[::-1]

def bits_to_byte(bits):
    """
    将 8 位二进制字符串转换为字节。
    """
    return int(bits, 2).to_bytes(1, byteorder='big')


def process_buffer(buffer):
    """
    处理 20 字节的缓冲区，生成 4 个像素的 R、G、B 分量。
    """
    if len(buffer) != 20:
        raise ValueError("Buffer must contain exactly 20 bytes.")

    # 提取 R、G、B 分量并反转（这里使用 bytes 而不是 bytearray）
    r_bytes = bytes(reversed(buffer[1::4]))  # R 分量（索引 1, 5, 9, 13）
    g_bytes = bytes(reversed(buffer[2::4]))  # G 分量（索引 2, 6, 10, 14）
    b_bytes = bytes(reversed(buffer[3::4]))  # B 分量（索引 3, 7, 11, 15）

    # 确保每个颜色通道都有 4 个字节（不足则补零）
    r_bytes = r_bytes.ljust(4, b'\x00')
    g_bytes = g_bytes.ljust(4, b'\x00')
    b_bytes = b_bytes.ljust(4, b'\x00')

    # 将每个颜色通道的组合字节转换为整数（假设是大端序）
    r_40bit_int = int.from_bytes(r_bytes, byteorder='big')  # 这里不需要左移，因为我们直接使用了完整的 32 位（或更少，但用零填充）
    print(r_40bit_int)
    g_40bit_int = int.from_bytes(g_bytes, byteorder='big')
    b_40bit_int = int.from_bytes(b_bytes, byteorder='big')

    # 注意：实际上我们并没有使用 40 位，而是使用了 32 位（4 字节 * 8 位/字节），
    # 但由于问题描述中提到了 40 位，我们在这里保持这种表述方式，
    # 并在后续步骤中丢弃低 2 位以模拟 10 位到 8 位的转换。

    # 从高位到低位分割成 4 个 10 位的值（但我们只关心低 8 位）
    pixels = []
    for i in range(4):
        r_10bit = (r_40bit_int >> (i * 10)) & 0x3FF  # 获取第 i 个 10 位值（实际上只使用了低 8 位有效数据）
        g_10bit = (g_40bit_int >> (i * 10)) & 0x3FF
        b_10bit = (b_40bit_int >> (i * 10)) & 0x3FF

        # 丢弃低 2 位，转换为 8 位值
        r_8bit = r_10bit >> 2
        g_8bit = g_10bit >> 2
        b_8bit = b_10bit >> 2

        pixels.append((r_8bit, g_8bit, b_8bit))

    return pixels
def create_bmp_from_file(input_file, output_file):
    """
    从输入文件中读取数据，生成 BMP 图像并保存到输出文件。
    """
    with open(input_file, 'rb') as f:
        buffer = bytearray()
        pixels = []
        
        while True:
            # 读取最多 20 个字节到缓冲区
            chunk = f.read(20)
            if not chunk:
                break  # 如果没有更多数据，则退出循环
            
            buffer.extend(chunk)
            
            # 如果缓冲区有 20 个字节，则处理它
            if len(buffer) == 20:
                pixel_group = process_buffer(buffer)
                pixels.extend(pixel_group)
                buffer.clear()  # 清空缓冲区以便下一次读取
        
        # 假设图像是宽度为图像像素数的矩形（这里简单处理为 1x(像素数/宽度)，如果像素数不是宽度整数倍则会有问题）
        # 为了简单起见，我们假设宽度为 1（即每个像素一行），但您可以根据需要调整
        width = 1280  # 或者使用 int(len(pixels) ** 0.5) 尝试自动计算正方形宽度（如果像素数是完全平方数）
        height = 720  # 计算高度（这里假设像素数能被宽度整除）
        
        # 如果像素数不能被宽度整除，需要添加额外的处理逻辑
        # 例如：通过填充空白像素或调整宽度和高度来适应所有像素
        
        # 创建 BMP 图像
        image = Image.new('RGB', (width, height))
        image.putdata(pixels)  # 将像素数据放入图像
        image.save(output_file, 'BMP')  # 保存为 BMP 文件

# 使用示例
input_filename = '1.bin'  # 替换为您的输入文件名
output_filename = 'output_image.bmp'  # 替换为您想要保存的 BMP 文件名
create_bmp_from_file(input_filename, output_filename)
