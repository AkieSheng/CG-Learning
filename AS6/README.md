# AS6

## 编译

```powershell
cd E:\Akie\Assignment\AS6
mingw32-make
```

## 参数说明

| 参数 | 说明 |
|------|------|
| `-input <file>` | 场景描述文件 |
| `-size <w> <h>` | 输出图像宽高（Makefile 默认常用 200 200） |
| `-output <file.tga>` | 渲染输出；`-gui` 下按 `r` 写入 |
| `-grid <nx> <ny> <nz>` | 均匀网格加速分辨率 |
| `-visualize_grid` | 可视化网格单元占用 |
| `-stats` | 打印光线追踪统计（射线数、求交次数等） |
| `-shadows` | 启用阴影光线 |
| `-bounces <n>` | 最大反射/折射递归深度 |
| `-weight <w>` | 递归权重截止阈值 |
| `-shade_back` | 背面着色 |
| `-gui` | OpenGL 预览 |
| `-normals <file.tga>` | 法线图 |
| `-depth <min> <max> <file.tga>` | 深度图 |
| `-tessellation <theta> <phi>` | Sphere 细分（OpenGL） |
| `-gouraud` | Sphere Gouraud（OpenGL） |

含 mesh / 过程纹理贴图的场景在 `Meshes\` 下运行。

## 样例

### scene6_01 — 单球（暴力 / 网格 / 网格可视化）

```powershell
.\raytracer.exe -input InputFiles\scene6_01_sphere.txt -size 200 200 -output OutputFiles\output6_01a.tga -stats
.\raytracer.exe -input InputFiles\scene6_01_sphere.txt -size 200 200 -output OutputFiles\output6_01b.tga -grid 10 10 10 -stats
.\raytracer.exe -input InputFiles\scene6_01_sphere.txt -size 200 200 -output OutputFiles\output6_01c.tga -grid 10 10 10 -visualize_grid
```

### scene6_02 — 球与三角形（阴影对比）

```powershell
.\raytracer.exe -input InputFiles\scene6_02_sphere_triangles.txt -size 200 200 -output OutputFiles\output6_02a.tga -stats
.\raytracer.exe -input InputFiles\scene6_02_sphere_triangles.txt -size 200 200 -output OutputFiles\output6_02b.tga -grid 10 10 10 -stats
.\raytracer.exe -input InputFiles\scene6_02_sphere_triangles.txt -size 200 200 -output OutputFiles\output6_02c.tga -stats -shadows
.\raytracer.exe -input InputFiles\scene6_02_sphere_triangles.txt -size 200 200 -output OutputFiles\output6_02d.tga -grid 10 10 10 -stats -shadows
.\raytracer.exe -input InputFiles\scene6_02_sphere_triangles.txt -size 200 200 -output OutputFiles\output6_02e.tga -grid 10 10 10 -visualize_grid
```

### scene6_03 — 球与平面（阴影对比）

```powershell
.\raytracer.exe -input InputFiles\scene6_03_sphere_plane.txt -size 200 200 -output OutputFiles\output6_03a.tga -stats
.\raytracer.exe -input InputFiles\scene6_03_sphere_plane.txt -size 200 200 -output OutputFiles\output6_03b.tga -grid 10 10 10 -stats
.\raytracer.exe -input InputFiles\scene6_03_sphere_plane.txt -size 200 200 -output OutputFiles\output6_03c.tga -stats -shadows
.\raytracer.exe -input InputFiles\scene6_03_sphere_plane.txt -size 200 200 -output OutputFiles\output6_03d.tga -grid 10 10 10 -stats -shadows
.\raytracer.exe -input InputFiles\scene6_03_sphere_plane.txt -size 200 200 -output OutputFiles\output6_03e.tga -grid 10 10 10 -visualize_grid
```

### scene6_04 — bunny_200

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene6_04_bunny_mesh_200.txt -size 200 200 -output ..\OutputFiles\output6_04a.tga -stats
..\raytracer.exe -input ..\InputFiles\scene6_04_bunny_mesh_200.txt -size 200 200 -output ..\OutputFiles\output6_04b.tga -grid 10 10 7 -stats
..\raytracer.exe -input ..\InputFiles\scene6_04_bunny_mesh_200.txt -size 200 200 -output ..\OutputFiles\output6_04c.tga -stats -shadows
..\raytracer.exe -input ..\InputFiles\scene6_04_bunny_mesh_200.txt -size 200 200 -output ..\OutputFiles\output6_04d.tga -grid 10 10 7 -stats -shadows
..\raytracer.exe -input ..\InputFiles\scene6_04_bunny_mesh_200.txt -size 200 200 -output ..\OutputFiles\output6_04e.tga -grid 10 10 7 -visualize_grid
```

### scene6_05 / 06 / 07 — bunny_1k / 5k / 40k

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene6_05_bunny_mesh_1k.txt -size 200 200 -output ..\OutputFiles\output6_05.tga -grid 15 15 12 -stats -shadows
..\raytracer.exe -input ..\InputFiles\scene6_06_bunny_mesh_5k.txt -size 200 200 -output ..\OutputFiles\output6_06.tga -grid 20 20 15 -stats -shadows
..\raytracer.exe -input ..\InputFiles\scene6_07_bunny_mesh_40k.txt -size 200 200 -output ..\OutputFiles\output6_07.tga -grid 40 40 33 -stats -shadows
```

### scene6_08 — 缩放平移

```powershell
.\raytracer.exe -input InputFiles\scene6_08_scale_translate.txt -size 200 200 -output OutputFiles\output6_08a.tga
.\raytracer.exe -input InputFiles\scene6_08_scale_translate.txt -size 200 200 -output OutputFiles\output6_08b.tga -grid 15 15 15
.\raytracer.exe -input InputFiles\scene6_08_scale_translate.txt -size 200 200 -output OutputFiles\output6_08c.tga -grid 15 15 15 -visualize_grid
```

### scene6_09 — 旋转三角形

```powershell
.\raytracer.exe -input InputFiles\scene6_09_rotated_triangles.txt -size 200 200 -output OutputFiles\output6_09a.tga
.\raytracer.exe -input InputFiles\scene6_09_rotated_triangles.txt -size 200 200 -output OutputFiles\output6_09b.tga -grid 15 15 9
.\raytracer.exe -input InputFiles\scene6_09_rotated_triangles.txt -size 200 200 -output OutputFiles\output6_09c.tga -grid 15 15 9 -visualize_grid
```

### scene6_10 — 嵌套变换（mesh）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene6_10_nested_transformations.txt -size 200 200 -output ..\OutputFiles\output6_10a.tga
..\raytracer.exe -input ..\InputFiles\scene6_10_nested_transformations.txt -size 200 200 -output ..\OutputFiles\output6_10b.tga -grid 30 30 30
..\raytracer.exe -input ..\InputFiles\scene6_10_nested_transformations.txt -size 200 200 -output ..\OutputFiles\output6_10c.tga -grid 30 30 30 -visualize_grid
```

### scene6_11 — 镜面地板（mesh）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene6_11_mirrored_floor.txt -size 200 200 -output ..\OutputFiles\output6_11a.tga -shadows -bounces 1 -weight 0.01 -stats
..\raytracer.exe -input ..\InputFiles\scene6_11_mirrored_floor.txt -size 200 200 -output ..\OutputFiles\output6_11b.tga -shadows -bounces 1 -weight 0.01 -grid 40 10 40 -stats
..\raytracer.exe -input ..\InputFiles\scene6_11_mirrored_floor.txt -size 200 200 -output ..\OutputFiles\output6_11c.tga -grid 40 10 40 -visualize_grid
```

### scene6_12 — 钻石 mesh

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene6_12_faceted_gem.txt -size 200 200 -output ..\OutputFiles\output6_12a.tga -shadows -shade_back -bounces 5 -weight 0.01 -stats
..\raytracer.exe -input ..\InputFiles\scene6_12_faceted_gem.txt -size 200 200 -output ..\OutputFiles\output6_12b.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20 -stats
..\raytracer.exe -input ..\InputFiles\scene6_12_faceted_gem.txt -size 200 200 -output ..\OutputFiles\output6_12c.tga -grid 20 20 20 -visualize_grid
```

### scene6_13 — 棋盘过程纹理

```powershell
.\raytracer.exe -input InputFiles\scene6_13_checkerboard.txt -size 200 200 -output OutputFiles\output6_13.tga -shadows
```

### scene6_14 — 玻璃球（mesh）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene6_14_glass_sphere.txt -size 200 200 -output ..\OutputFiles\output6_14.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20
```

### scene6_15 — 大理石立方体（mesh，300×300）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene6_15_marble_cubes.txt -size 300 300 -output ..\OutputFiles\output6_15.tga
```

### scene6_16 — 木材立方体（mesh，300×300）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene6_16_wood_cubes.txt -size 300 300 -output ..\OutputFiles\output6_16.tga
```

### scene6_17 — 大理石花瓶（mesh，300×300）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene6_17_marble_vase.txt -size 300 300 -output ..\OutputFiles\output6_17a.tga -grid 15 30 15 -bounces 1 -shadows
..\raytracer.exe -input ..\InputFiles\scene6_17_marble_vase.txt -size 300 300 -output ..\OutputFiles\output6_17b.tga -grid 15 30 15 -visualize_grid
```

### scene6_18 — 6.837 logo（mesh，400×200）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene6_18_6.837_logo.txt -size 400 200 -output ..\OutputFiles\output6_18a.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 80 30 3
..\raytracer.exe -input ..\InputFiles\scene6_18_6.837_logo.txt -size 400 200 -output ..\OutputFiles\output6_18b.tga -grid 80 30 3 -visualize_grid
```
