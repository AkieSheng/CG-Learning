# AS5

## 编译

```powershell
cd E:\Akie\Assignment\AS5
mingw32-make
```

## 参数说明

| 参数 | 说明 |
|------|------|
| `-input <file>` | 场景描述文件 |
| `-size <w> <h>` | 输出图像宽高（常用 200 200） |
| `-output <file.tga>` | 渲染输出；`-gui` 下按 `r` 写入 |
| `-grid <nx> <ny> <nz>` | 均匀网格分辨率 |
| `-visualize_grid` | 可视化网格单元占用（非几何着色） |
| `-gui` | OpenGL 预览 |
| `-normals <file.tga>` | 法线图 |
| `-tessellation <theta> <phi>` | Sphere 细分（OpenGL） |
| `-gouraud` | Sphere Gouraud（OpenGL） |

含 mesh 的场景在 `Meshes\` 下运行。

## 样例

### scene5_01 — 单球网格分辨率对比

```powershell
.\raytracer.exe -input InputFiles\scene5_01_sphere.txt -size 200 200 -output OutputFiles\output5_01a.tga -grid 1 1 1 -visualize_grid
.\raytracer.exe -input InputFiles\scene5_01_sphere.txt -size 200 200 -output OutputFiles\output5_01b.tga -grid 5 5 5 -visualize_grid
.\raytracer.exe -input InputFiles\scene5_01_sphere.txt -size 200 200 -output OutputFiles\output5_01c.tga -grid 15 15 15 -visualize_grid
.\raytracer.exe -input InputFiles\scene5_01_sphere.txt -size 200 200 -output OutputFiles\gui5_01c.tga -grid 15 15 15 -visualize_grid -gui
```

### scene5_02 — 多球（各向同性 / 扁平网格）

```powershell
.\raytracer.exe -input InputFiles\scene5_02_spheres.txt -size 200 200 -output OutputFiles\output5_02a.tga -grid 15 15 15 -visualize_grid
.\raytracer.exe -input InputFiles\scene5_02_spheres.txt -size 200 200 -output OutputFiles\output5_02b.tga -grid 15 15 3 -visualize_grid
```

### scene5_03 — 偏心球

```powershell
.\raytracer.exe -input InputFiles\scene5_03_offcenter_spheres.txt -size 200 200 -output OutputFiles\output5_03.tga -grid 20 20 20 -visualize_grid
```

### scene5_04 — 平面测试

```powershell
.\raytracer.exe -input InputFiles\scene5_04_plane_test.txt -size 200 200 -output OutputFiles\output5_04.tga -grid 15 15 15 -visualize_grid
.\raytracer.exe -input InputFiles\scene5_04_plane_test.txt -size 200 200 -output OutputFiles\gui5_04_norm.tga -normals OutputFiles\norm5_04.tga -tessellation 30 15 -gouraud -gui
```

### scene5_05 — 球与三角形

```powershell
.\raytracer.exe -input InputFiles\scene5_05_sphere_triangles.txt -size 200 200 -output OutputFiles\output5_05.tga -grid 20 20 10 -visualize_grid
```

### scene5_06 — bunny_200

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene5_06_bunny_mesh_200.txt -size 200 200 -output ..\OutputFiles\output5_06.tga -grid 10 10 7 -visualize_grid
..\raytracer.exe -input ..\InputFiles\scene5_06_bunny_mesh_200.txt -size 200 200 -output ..\OutputFiles\gui5_06.tga -grid 10 10 7 -visualize_grid -gui
```

### scene5_07 — bunny_1k

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene5_07_bunny_mesh_1k.txt -size 200 200 -output ..\OutputFiles\output5_07.tga -grid 15 15 12 -visualize_grid
```

### scene5_08 — bunny_5k

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene5_08_bunny_mesh_5k.txt -size 200 200 -output ..\OutputFiles\output5_08.tga -grid 20 20 15 -visualize_grid
```

### scene5_09 — bunny_40k

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene5_09_bunny_mesh_40k.txt -size 200 200 -output ..\OutputFiles\output5_09.tga -grid 40 40 33 -visualize_grid
```

### scene5_10 — 缩放平移

```powershell
.\raytracer.exe -input InputFiles\scene5_10_scale_translate.txt -size 200 200 -output OutputFiles\output5_10.tga -grid 15 15 15 -visualize_grid
.\raytracer.exe -input InputFiles\scene5_10_scale_translate.txt -size 200 200 -output OutputFiles\gui5_10_norm.tga -normals OutputFiles\norm5_10.tga -tessellation 30 15 -gouraud -gui
```

### scene5_11 — 旋转三角形

```powershell
.\raytracer.exe -input InputFiles\scene5_11_rotated_triangles.txt -size 200 200 -output OutputFiles\output5_11.tga -grid 15 15 9 -visualize_grid
```

### scene5_12 — 嵌套变换（mesh）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene5_12_nested_transformations.txt -size 200 200 -output ..\OutputFiles\output5_12.tga -grid 30 30 30 -visualize_grid
..\raytracer.exe -input ..\InputFiles\scene5_12_nested_transformations.txt -size 200 200 -output ..\OutputFiles\gui5_12.tga -grid 30 30 30 -visualize_grid -gui
```
