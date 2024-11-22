# parse_hdmi
parse a hdmi frame
这个仓库介绍如何解析hdmi的一帧数据，截取hdmi 1.4编码部分协议如下:
![image](https://github.com/user-attachments/assets/3a37b824-7c88-411b-8000-1e3ecf07dda3)

如上图所示，hdmi分别有3个tmds通道和1个时钟通道

![image](https://github.com/user-attachments/assets/95f6e18d-2bae-4749-8439-458cef09c8ac)

上图是hdmi一帧数据的构成
右下角为有效图像区域，其余为blanking区域，blanking区域中嵌入data island来传递各种包的信息

现有dump出的部分数据如下图所示
![avi byte](https://github.com/user-attachments/assets/20a141a4-e137-4454-98c9-85468e4a47d4)

图中四个byte一循环，分别是clk r g b,我们不用关心clk，开头G B通道为01 01，组合为1010，为1个ctrl,协议中
如下图所示:

![image](https://github.com/user-attachments/assets/403d7344-2237-4756-8aac-660ceaa17343)

1010表示后面的数据是data island,接着 R G通道为33 33,表示data island的leading guard band

![image](https://github.com/user-attachments/assets/bedbcc57-1148-4266-97b6-1eac1de47f38)

后面跟着的是64个4byte,表示两个packet，解析规则如下:
![image](https://github.com/user-attachments/assets/d020f4c4-d548-4500-9778-7e64be692a40)
![image](https://github.com/user-attachments/assets/4a8cb006-d12f-4b3e-932a-e19d87cb5161)

B通道的32byte的d2 bit组成pkt的包头

![597c7cb2f264ce93235ea69b2ff900a](https://github.com/user-attachments/assets/a9e7d6dc-1662-4b34-9d77-bd57aad0b4f3)

如上数据B通道依次取d2，数据如下：
01000001 01000000 00001011 00100111
对应16进制 0x82 0x02 0xd    ecc校验
是avi的包头
解析body数据，如下:

![11b29b98d2fa80597b165afd138a9d3](https://github.com/user-attachments/assets/2490b5ae-fb24-4ae8-85c4-d3cdb19dc8c7)

01001000 00001010 00010101 00100000 10000110 00000000 00000000 10111111
0x12     0x50       0xa8       0x04     0x61    0x0       0x0


以上是data island解包的介绍，接着如何提取有效图像的部分
计算有效图像的原点为(htotal-hactive, vtotal - vactive),提取长为hactive,宽为vactive的图像即可，RGB 8bit是最简单的

![image](https://github.com/user-attachments/assets/8cba43a0-76df-4d80-81d9-887526bd89d5)
解析出的图像如下:

![image](https://github.com/user-attachments/assets/e92a60a6-2834-4943-9a1b-798fcb335e98)

目前只支持rgb 8bit解析，其它还未开发,参考writemif仓库应该不难实现。

