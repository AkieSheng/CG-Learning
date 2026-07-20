# AS1

## 编译

```powershell
cd E:\Akie\Assignment\AS1
mingw32-make
```

## 参数说明

| 参数 | 说明 |
|------|------|
| `-input <file>` | 场景描述文件 |
| `-size <w> <h>` | 输出图像宽高（像素） |
| `-output <file.tga>` | 颜色渲染输出 |
| `-depth <min> <max> <file.tga>` | 深度可视化：将交点 t 映射到 `[min,max]` 灰度并写 TGA |

## 样例

### scene1_01

```powershell
.\raytracer.exe -input InputFiles\scene1_01.txt -size 200 200 -output OutputFiles\output1_01.tga -depth 9 10 OutputFiles\depth1_01.tga
```

### scene1_02

```powershell
.\raytracer.exe -input InputFiles\scene1_02.txt -size 200 200 -output OutputFiles\output1_02.tga -depth 8 12 OutputFiles\depth1_02.tga
```

### scene1_03

```powershell
.\raytracer.exe -input InputFiles\scene1_03.txt -size 200 200 -output OutputFiles\output1_03.tga -depth 8 12 OutputFiles\depth1_03.tga
```

### scene1_04

```powershell
.\raytracer.exe -input InputFiles\scene1_04.txt -size 200 200 -output OutputFiles\output1_04.tga -depth 12 17 OutputFiles\depth1_04.tga
```

### scene1_05

```powershell
.\raytracer.exe -input InputFiles\scene1_05.txt -size 200 200 -output OutputFiles\output1_05.tga -depth 14.5 19.5 OutputFiles\depth1_05.tga
```

### scene1_06

```powershell
.\raytracer.exe -input InputFiles\scene1_06.txt -size 200 200 -output OutputFiles\output1_06.tga -depth 3 7 OutputFiles\depth1_06.tga
```

### scene1_07

```powershell
.\raytracer.exe -input InputFiles\scene1_07.txt -size 200 200 -output OutputFiles\output1_07.tga -depth -2 2 OutputFiles\depth1_07.tga
```
