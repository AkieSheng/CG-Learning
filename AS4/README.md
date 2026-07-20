# AS4

## 编译

```powershell
cd E:\Akie\Assignment\AS4
mingw32-make
```

## 参数说明

| 参数 | 说明 |
|------|------|
| `-input <file>` | 场景描述文件 |
| `-size <w> <h>` | 输出图像宽高（Makefile 默认常用 200 200） |
| `-output <file.tga>` | 递归光线追踪输出；`-gui` 下按 `r` 写入 |
| `-shadows` | 启用阴影光线 |
| `-transparent_shadows` | 半透明阴影（光路穿过透明体衰减） |
| `-bounces <n>` | 最大反射/折射递归深度（0=仅局部着色） |
| `-weight <w>` | 递归权重截止阈值 |
| `-shade_back` | 背面着色（透明场景通常需要） |
| `-gui` | OpenGL 预览；`t` 可视化 Ray Tree |
| `-tessellation <theta> <phi>` | Sphere 细分（OpenGL） |
| `-gouraud` | Sphere Gouraud（OpenGL） |
| `-specular_fix` | 高光乘以 N·L |

含 mesh 的场景在 `Meshes\` 下运行。

## 样例

### scene4_01 — 球体硬阴影

```powershell
.\raytracer.exe -input InputFiles\scene4_01_sphere_shadow.txt -size 200 200 -output OutputFiles\output4_01.tga -shadows
.\raytracer.exe -input InputFiles\scene4_01_sphere_shadow.txt -size 200 200 -output OutputFiles\gui4_01.tga -shadows -gui
```

### scene4_02 — 彩色阴影

```powershell
.\raytracer.exe -input InputFiles\scene4_02_colored_shadows.txt -size 200 200 -output OutputFiles\output4_02.tga -shadows
.\raytracer.exe -input InputFiles\scene4_02_colored_shadows.txt -size 200 200 -output OutputFiles\gui4_02.tga -shadows -gui -tessellation 50 25 -gouraud
```

### scene4_03 — 镜面地板（mesh）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene4_03_mirrored_floor.txt -size 200 200 -output ..\OutputFiles\output4_03.tga -shadows -bounces 1 -weight 0.01
..\raytracer.exe -input ..\InputFiles\scene4_03_mirrored_floor.txt -size 200 200 -output ..\OutputFiles\gui4_03.tga -shadows -bounces 1 -weight 0.01 -gui
```

### scene4_04 — 反射球（对比 bounces）

```powershell
.\raytracer.exe -input InputFiles\scene4_04_reflective_sphere.txt -size 200 200 -output OutputFiles\output4_04a.tga -shadows -bounces 0 -weight 0.01
.\raytracer.exe -input InputFiles\scene4_04_reflective_sphere.txt -size 200 200 -output OutputFiles\output4_04b.tga -shadows -bounces 1 -weight 0.01
.\raytracer.exe -input InputFiles\scene4_04_reflective_sphere.txt -size 200 200 -output OutputFiles\output4_04c.tga -shadows -bounces 2 -weight 0.01
.\raytracer.exe -input InputFiles\scene4_04_reflective_sphere.txt -size 200 200 -output OutputFiles\output4_04d.tga -shadows -bounces 3 -weight 0.01
.\raytracer.exe -input InputFiles\scene4_04_reflective_sphere.txt -size 200 200 -output OutputFiles\gui4_04.tga -shadows -bounces 2 -weight 0.01 -gui
```

### scene4_05 — 透明 bar

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene4_05_transparent_bar.txt -size 200 200 -output ..\OutputFiles\output4_05.tga -shadows -bounces 10 -weight 0.01 -shade_back
..\raytracer.exe -input ..\InputFiles\scene4_05_transparent_bar.txt -size 200 200 -output ..\OutputFiles\gui4_05.tga -shadows -bounces 10 -weight 0.01 -shade_back -gui
```

### scene4_06 — 多透明 bar（对比 bounces / 半透明阴影）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene4_06_transparent_bars.txt -size 200 200 -output ..\OutputFiles\output4_06a.tga -shadows -bounces 0 -weight 0.01 -shade_back
..\raytracer.exe -input ..\InputFiles\scene4_06_transparent_bars.txt -size 200 200 -output ..\OutputFiles\output4_06b.tga -shadows -bounces 1 -weight 0.01 -shade_back
..\raytracer.exe -input ..\InputFiles\scene4_06_transparent_bars.txt -size 200 200 -output ..\OutputFiles\output4_06c.tga -shadows -bounces 2 -weight 0.01 -shade_back
..\raytracer.exe -input ..\InputFiles\scene4_06_transparent_bars.txt -size 200 200 -output ..\OutputFiles\output4_06d.tga -shadows -bounces 3 -weight 0.01 -shade_back
..\raytracer.exe -input ..\InputFiles\scene4_06_transparent_bars.txt -size 200 200 -output ..\OutputFiles\output4_06e.tga -shadows -bounces 4 -weight 0.01 -shade_back
..\raytracer.exe -input ..\InputFiles\scene4_06_transparent_bars.txt -size 200 200 -output ..\OutputFiles\output4_06f.tga -shadows -bounces 5 -weight 0.01 -shade_back
..\raytracer.exe -input ..\InputFiles\scene4_06_transparent_bars.txt -size 200 200 -output ..\OutputFiles\output4_06_trans.tga -shadows -transparent_shadows -bounces 3 -weight 0.01 -shade_back
```

### scene4_07 / 08 / 09 — 透明球 IOR 1.0 / 1.1 / 2.0

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene4_07_transparent_sphere_1.0.txt -size 200 200 -output ..\OutputFiles\output4_07.tga -shadows -bounces 5 -weight 0.01 -shade_back
..\raytracer.exe -input ..\InputFiles\scene4_08_transparent_sphere_1.1.txt -size 200 200 -output ..\OutputFiles\output4_08.tga -shadows -bounces 5 -weight 0.01 -shade_back
..\raytracer.exe -input ..\InputFiles\scene4_09_transparent_sphere_2.0.txt -size 200 200 -output ..\OutputFiles\output4_09.tga -shadows -bounces 5 -weight 0.01 -shade_back
..\raytracer.exe -input ..\InputFiles\scene4_08_transparent_sphere_1.1.txt -size 200 200 -output ..\OutputFiles\gui4_08.tga -shadows -bounces 5 -weight 0.01 -shade_back -gui -tessellation 30 15
```

### scene4_10 — 点光源距离衰减

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene4_10_point_light_distance.txt -size 200 200 -output ..\OutputFiles\output4_10.tga -shadows
..\raytracer.exe -input ..\InputFiles\scene4_10_point_light_distance.txt -size 200 200 -output ..\OutputFiles\gui4_10.tga -shadows -gui
```

### scene4_11 / 12 / 13 — 圆环点光源衰减

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene4_11_point_light_circle.txt -size 200 200 -output ..\OutputFiles\output4_11.tga -shadows
..\raytracer.exe -input ..\InputFiles\scene4_12_point_light_circle_d_attenuation.txt -size 200 200 -output ..\OutputFiles\output4_12.tga -shadows
..\raytracer.exe -input ..\InputFiles\scene4_13_point_light_circle_d2_attenuation.txt -size 200 200 -output ..\OutputFiles\output4_13.tga -shadows
```

### scene4_14 — 钻石 mesh（对比 bounces）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene4_14_faceted_gem.txt -size 200 200 -output ..\OutputFiles\output4_14a.tga -shadows -shade_back -bounces 0 -weight 0.01
..\raytracer.exe -input ..\InputFiles\scene4_14_faceted_gem.txt -size 200 200 -output ..\OutputFiles\output4_14b.tga -shadows -shade_back -bounces 1 -weight 0.01
..\raytracer.exe -input ..\InputFiles\scene4_14_faceted_gem.txt -size 200 200 -output ..\OutputFiles\output4_14c.tga -shadows -shade_back -bounces 2 -weight 0.01
..\raytracer.exe -input ..\InputFiles\scene4_14_faceted_gem.txt -size 200 200 -output ..\OutputFiles\output4_14d.tga -shadows -shade_back -bounces 3 -weight 0.01
..\raytracer.exe -input ..\InputFiles\scene4_14_faceted_gem.txt -size 200 200 -output ..\OutputFiles\output4_14e.tga -shadows -shade_back -bounces 4 -weight 0.01
..\raytracer.exe -input ..\InputFiles\scene4_14_faceted_gem.txt -size 200 200 -output ..\OutputFiles\output4_14f.tga -shadows -shade_back -bounces 5 -weight 0.01
..\raytracer.exe -input ..\InputFiles\scene4_14_faceted_gem.txt -size 200 200 -output ..\OutputFiles\gui4_14.tga -shadows -shade_back -bounces 2 -weight 0.01 -gui
```
