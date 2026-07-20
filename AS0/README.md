# AS0

## 编译

```powershell
cd E:\Akie\Assignment\AS0
mingw32-make
```

## 参数说明

| 参数 | 说明 |
|------|------|
| `-input <file>` | IFS 场景描述文件 |
| `-points <n>` | 随机采样点数 |
| `-iters <k>` | 每个点的迭代次数 |
| `-size <n>` | 输出正方形图像边长（像素） |
| `-output <file.tga>` | 输出 TGA 路径 |

## 样例

### sierpinski_triangle — Sierpinski 三角形（对比迭代次数）

```powershell
.\ifs.exe -input Datafiles\sierpinski_triangle.txt -points 10000 -iters 0 -size 200 -output OutputFiles\sierpinski_triangle_0.tga
.\ifs.exe -input Datafiles\sierpinski_triangle.txt -points 10000 -iters 1 -size 200 -output OutputFiles\sierpinski_triangle_1.tga
.\ifs.exe -input Datafiles\sierpinski_triangle.txt -points 10000 -iters 2 -size 200 -output OutputFiles\sierpinski_triangle_2.tga
.\ifs.exe -input Datafiles\sierpinski_triangle.txt -points 10000 -iters 3 -size 200 -output OutputFiles\sierpinski_triangle_3.tga
.\ifs.exe -input Datafiles\sierpinski_triangle.txt -points 10000 -iters 4 -size 200 -output OutputFiles\sierpinski_triangle_4.tga
.\ifs.exe -input Datafiles\sierpinski_triangle.txt -points 10000 -iters 30 -size 200 -output OutputFiles\sierpinski_triangle.tga
```

### fern — Barnsley 蕨叶

```powershell
.\ifs.exe -input Datafiles\fern.txt -points 50000 -iters 30 -size 400 -output OutputFiles\fern.tga
```

### dragon — 龙形吸引子

```powershell
.\ifs.exe -input Datafiles\dragon.txt -points 50000 -iters 30 -size 400 -output OutputFiles\dragon.tga
```

### giant_x — Giant X

```powershell
.\ifs.exe -input Datafiles\giant_x.txt -points 50000 -iters 30 -size 400 -output OutputFiles\giant_x.tga
```
