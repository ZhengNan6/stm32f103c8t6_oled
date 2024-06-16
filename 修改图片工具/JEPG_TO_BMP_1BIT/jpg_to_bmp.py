from PIL import Image
import os  
import sys  
 
# 获取文件夹的目录  
script_dir = os.path.dirname(os.path.abspath(__file__))  
 
# 将工作目录更改为文件夹所在的目录  
os.chdir(script_dir)  

# 打印当前工作目录  
print("当前工作目录:", os.getcwd())  
  
# 打印当前工作目录下的文件列表  
files = os.listdir(os.getcwd())  
print("当前工作目录下的文件:")  
for file in files:  
    print(file)

print("转换图片名称请为 input.jpg (128像素宽度,64像素高度)")

try:
    input_num = int(input("选择图片反像? (1: 是; 0: 否): "))
    if input_num == 1:
        num_1 = 0
        num_2 = 255
    elif input_num == 0:
        num_1 = 255
        num_2 = 0
    else:
        raise ValueError("无效输入，请输入 1 或 0。")
except ValueError as e:
    print(e)
    exit(1)

try:
    input_binary = int(input("二值化阈值 (0-255): "))
    if input_binary < 0 or input_binary > 255:
        raise ValueError("阈值必须在 0 到 255 之间。")
except ValueError as e:
    print(e)
    exit(1)

# 打开JPEG图像
try:
    jpeg_image = Image.open('input.jpg')
except FileNotFoundError:
    print("文件 'input.jpg' 未找到。")
    exit(1)

# 将图像转换为灰度
gray_image = jpeg_image.convert('L')

# 二值化处理
binary_image = gray_image.point(lambda x: num_1 if x < input_binary else num_2, '1')

# 保存为1位BMP格式
binary_image.save('output.bmp')

print("转换完成，保存为 'output.bmp'")
