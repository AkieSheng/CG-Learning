# AS3

## 编译

```powershell
cd E:\Akie\Assignment\AS3
mingw32-make
```

## 参数说明

| 参数 | 说明 |
|------|------|
| `-input <file>` | 场景描述文件 |
| `-size <w> <h>` | 输出图像宽高 |
| `-output <file.tga>` | Phong 着色输出；`-gui` 下按 `r` 也写入该文件 |
| `-gui` | 启动 OpenGL 预览 |
| `-tessellation <theta> <phi>` | Sphere 多边形细分（OpenGL） |
| `-gouraud` | Sphere Gouraud 插值（OpenGL） |
| `-specular_fix` | 光线追踪高光乘以 N·L |
| `-normals <file.tga>` | 法线图（若实现/启用） |
| `-depth <min> <max> <file.tga>` | 深度图（若命令行支持） |

GUI：`r` 用当前相机光线追踪；`q` 退出；左键旋转、中键平移、右键推拉。

含 mesh 的场景在 `TriangleMeshes\` 下运行。

## 样例

### scene3_01 — 正交相机 + 立方体

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene3_01_cube_orthographic.txt -size 200 200 -output ..\OutputFiles\output3_01.tga
..\raytracer.exe -input ..\InputFiles\scene3_01_cube_orthographic.txt -size 200 200 -output ..\OutputFiles\gui3_01.tga -gui
```

### scene3_02 — 透视相机 + 立方体

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene3_02_cube_perspective.txt -size 200 200 -output ..\OutputFiles\output3_02.tga
..\raytracer.exe -input ..\InputFiles\scene3_02_cube_perspective.txt -size 200 200 -output ..\OutputFiles\gui3_02.tga -gui
```

### scene3_03 — bunny_200

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene3_03_bunny_mesh_200.txt -size 200 200 -output ..\OutputFiles\output3_03.tga
..\raytracer.exe -input ..\InputFiles\scene3_03_bunny_mesh_200.txt -size 200 200 -output ..\OutputFiles\gui3_03.tga -gui
```

### scene3_04 — bunny_1k

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene3_04_bunny_mesh_1k.txt -size 200 200 -output ..\OutputFiles\output3_04.tga
..\raytracer.exe -input ..\InputFiles\scene3_04_bunny_mesh_1k.txt -size 200 200 -output ..\OutputFiles\gui3_04.tga -gui
```

### scene3_05 — 坐标轴立方体

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene3_05_axes_cube.txt -size 200 200 -output ..\OutputFiles\output3_05.tga
..\raytracer.exe -input ..\InputFiles\scene3_05_axes_cube.txt -size 200 200 -output ..\OutputFiles\gui3_05.tga -gui
```

### scene3_06 — 复杂变换

```powershell
cd TriangleMeshes
..\raytracer.exe -input ..\InputFiles\scene3_06_crazy_transforms.txt -size 200 200 -output ..\OutputFiles\output3_06.tga
..\raytracer.exe -input ..\InputFiles\scene3_06_crazy_transforms.txt -size 200 200 -output ..\OutputFiles\gui3_06.tga -gui
```

### scene3_07 — 平面 + 多球

```powershell
.\raytracer.exe -input InputFiles\scene3_07_plane.txt -size 200 200 -output OutputFiles\output3_07.tga -tessellation 10 5
.\raytracer.exe -input InputFiles\scene3_07_plane.txt -size 200 200 -output OutputFiles\gui3_07.tga -gui -tessellation 10 5
```

### scene3_08 — 单球（细分 / Gouraud 变体）

```powershell
.\raytracer.exe -input InputFiles\scene3_08_sphere.txt -size 200 200 -output OutputFiles\output3_08.tga -tessellation 10 5
.\raytracer.exe -input InputFiles\scene3_08_sphere.txt -size 200 200 -output OutputFiles\output3_08_t10_p5.tga -tessellation 10 5
.\raytracer.exe -input InputFiles\scene3_08_sphere.txt -size 200 200 -output OutputFiles\output3_08_t20_p10.tga -tessellation 20 10
.\raytracer.exe -input InputFiles\scene3_08_sphere.txt -size 200 200 -output OutputFiles\output3_08_t10_p5_gouraud.tga -tessellation 10 5 -gouraud
.\raytracer.exe -input InputFiles\scene3_08_sphere.txt -size 200 200 -output OutputFiles\output3_08_t20_p10_gouraud.tga -tessellation 20 10 -gouraud
.\raytracer.exe -input InputFiles\scene3_08_sphere.txt -size 200 200 -output OutputFiles\gui3_08.tga -gui -tessellation 10 5
```

### scene3_09 — exponent 高光变化

```powershell
.\raytracer.exe -input InputFiles\scene3_09_exponent_variations.txt -size 300 300 -output OutputFiles\output3_09.tga -tessellation 100 50 -gouraud
.\raytracer.exe -input InputFiles\scene3_09_exponent_variations.txt -size 300 300 -output OutputFiles\output3_09_specfix.tga -tessellation 100 50 -gouraud -specular_fix
.\raytracer.exe -input InputFiles\scene3_09_exponent_variations.txt -size 300 300 -output OutputFiles\gui3_09.tga -gui -tessellation 100 50 -gouraud
```

### scene3_10 — 背面 exponent

```powershell
.\raytracer.exe -input InputFiles\scene3_10_exponent_variations_back.txt -size 300 300 -output OutputFiles\output3_10.tga -tessellation 100 50 -gouraud
.\raytracer.exe -input InputFiles\scene3_10_exponent_variations_back.txt -size 300 300 -output OutputFiles\output3_10_specfix.tga -tessellation 100 50 -gouraud -specular_fix
.\raytracer.exe -input InputFiles\scene3_10_exponent_variations_back.txt -size 300 300 -output OutputFiles\gui3_10.tga -gui -tessellation 100 50 -gouraud
```

### scene3_11 — 特殊漫反射光照

```powershell
.\raytracer.exe -input InputFiles\scene3_11_weird_lighting_diffuse.txt -size 200 200 -output OutputFiles\output3_11.tga -tessellation 100 50 -gouraud
.\raytracer.exe -input InputFiles\scene3_11_weird_lighting_diffuse.txt -size 200 200 -output OutputFiles\gui3_11.tga -gui -tessellation 100 50 -gouraud
```

### scene3_12 — 特殊高光光照

```powershell
.\raytracer.exe -input InputFiles\scene3_12_weird_lighting_specular.txt -size 200 200 -output OutputFiles\output3_12.tga -tessellation 100 50 -gouraud
.\raytracer.exe -input InputFiles\scene3_12_weird_lighting_specular.txt -size 200 200 -output OutputFiles\output3_12_specfix.tga -tessellation 100 50 -gouraud -specular_fix
.\raytracer.exe -input InputFiles\scene3_12_weird_lighting_specular.txt -size 200 200 -output OutputFiles\gui3_12.tga -gui -tessellation 100 50 -gouraud
```
