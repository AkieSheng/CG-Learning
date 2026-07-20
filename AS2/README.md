# AS2

## 编译

```powershell
cd E:\Akie\Assignment\AS2
mingw32-make
```

## 参数说明

| 参数 | 说明 |
|------|------|
| `-input <file>` | 场景描述文件 |
| `-size <w> <h>` | 输出图像宽高 |
| `-output <file.tga>` | 漫反射着色输出 |
| `-depth <min> <max> <file.tga>` | 深度图输出 |
| `-normals <file.tga>` | 法线图输出 |
| `-shade_back` | 对背面也做漫反射着色 |

含 `obj_file` 的场景须在 `TriangleMeshes\` 目录下运行可执行文件。

## 样例

### scene2_01 — 漫反射

```powershell
.\raytracer.exe -input InputFiles\scene2_01_diffuse.txt -size 200 200 -output OutputFiles\output2_01.tga
```

### scene2_02 — 环境光

```powershell
.\raytracer.exe -input InputFiles\scene2_02_ambient.txt -size 200 200 -output OutputFiles\output2_02.tga
```

### scene2_03 — 彩色灯光 + 法线

```powershell
.\raytracer.exe -input InputFiles\scene2_03_colored_lights.txt -size 200 200 -output OutputFiles\output2_03.tga -normals OutputFiles\normals2_03.tga
```

### scene2_04 — 透视相机 + 法线

```powershell
.\raytracer.exe -input InputFiles\scene2_04_perspective.txt -size 200 200 -output OutputFiles\output2_04.tga -normals OutputFiles\normals2_04.tga
```

### scene2_05 — 球内视角（对比 shade_back）

```powershell
.\raytracer.exe -input InputFiles\scene2_05_inside_sphere.txt -size 200 200 -output OutputFiles\output2_05.tga -depth 9 11 OutputFiles\depth2_05.tga -normals OutputFiles\normals2_05.tga -shade_back
.\raytracer.exe -input InputFiles\scene2_05_inside_sphere.txt -size 200 200 -output OutputFiles\output2_05_no_back.tga
```

### scene2_06 — 平面

```powershell
.\raytracer.exe -input InputFiles\scene2_06_plane.txt -size 200 200 -output OutputFiles\output2_06.tga -depth 8 20 OutputFiles\depth2_06.tga -normals OutputFiles\normals2_06.tga
```

### scene2_07 — 球与三角形（对比 shade_back）

```powershell
.\raytracer.exe -input InputFiles\scene2_07_sphere_triangles.txt -size 200 200 -output OutputFiles\output2_07.tga -depth 9 11 OutputFiles\depth2_07.tga -normals OutputFiles\normals2_07.tga -shade_back
.\raytracer.exe -input InputFiles\scene2_07_sphere_triangles.txt -size 200 200 -output OutputFiles\output2_07_no_back.tga
```

### scene2_08 — 立方体 mesh

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene2_08_cube.txt -size 200 200 -output ..\OutputFiles\output2_08.tga
```

### scene2_09 — bunny_200

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene2_09_bunny_200.txt -size 200 200 -output ..\OutputFiles\output2_09.tga
```

### scene2_10 — bunny_1k

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene2_10_bunny_1k.txt -size 200 200 -output ..\OutputFiles\output2_10.tga
```

### scene2_11 — 压扁球 + 法线

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene2_11_squashed_sphere.txt -size 200 200 -output ..\OutputFiles\output2_11.tga -normals ..\OutputFiles\normals2_11.tga
```

### scene2_12 — 旋转球 + 法线

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene2_12_rotated_sphere.txt -size 200 200 -output ..\OutputFiles\output2_12.tga -normals ..\OutputFiles\normals2_12.tga
```

### scene2_13 — 旋转压扁球 + 法线

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene2_13_rotated_squashed_sphere.txt -size 200 200 -output ..\OutputFiles\output2_13.tga -normals ..\OutputFiles\normals2_13.tga
```

### scene2_14 — 坐标轴立方体

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene2_14_axes_cube.txt -size 200 200 -output ..\OutputFiles\output2_14.tga
```

### scene2_15 — 复杂变换

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene2_15_crazy_transforms.txt -size 200 200 -output ..\OutputFiles\output2_15.tga
```

### scene2_16 — T 尺度变换 + 深度

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene2_16_t_scale.txt -size 200 200 -output ..\OutputFiles\output2_16.tga -depth 2 7 ..\OutputFiles\depth2_16.tga
```
