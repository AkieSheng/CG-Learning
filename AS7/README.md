# AS7

## 编译

```powershell
cd E:\Akie\Assignment\AS7
mingw32-make
```

输出按场景分目录：`OutputFiles\scene7_0N\{outputs,samples,filters}\`。

## 参数说明

| 参数 | 说明 |
|------|------|
| `-input <file>` | 场景描述文件 |
| `-size <w> <h>` | 输出图像宽高 |
| `-output <file.tga>` | 抗锯齿合成后的颜色输出 |
| `-random_samples <n>` | 每像素 n 个随机采样 |
| `-uniform_samples <n>` | 每像素 n 个均匀网格采样（n 为完全平方数） |
| `-jittered_samples <n>` | 每像素 n 个抖动采样（n 为完全平方数） |
| `-box_filter <r>` | 盒滤波半径 |
| `-tent_filter <r>` | 帐篷滤波半径 |
| `-gaussian_filter <sigma>` | 高斯滤波标准差 |
| `-render_samples <file.tga> <zoom>` | 将采样点可视化放大 zoom 倍写出 |
| `-render_filter <file.tga> <zoom>` | 将滤波核可视化放大 zoom 倍写出 |
| `-grid <nx> <ny> <nz>` | 均匀网格加速 |
| `-shadows` | 阴影光线 |
| `-bounces <n>` | 最大反射/折射深度 |
| `-weight <w>` | 递归权重截止 |
| `-shade_back` | 背面着色 |
| `-gui` | OpenGL 预览 |

含 mesh 的场景在 `Meshes\` 下运行。

## 样例

### scene7_01 — 球与三角形：采样 / 滤波调试 + 低分辨率合成

```powershell
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 180 180 -output OutputFiles\scene7_01\outputs\output7_01.tga

.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_samples OutputFiles\scene7_01\samples\samples7_01a.tga 20 -random_samples 4
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_samples OutputFiles\scene7_01\samples\samples7_01b.tga 20 -uniform_samples 4
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_samples OutputFiles\scene7_01\samples\samples7_01c.tga 20 -jittered_samples 4
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_samples OutputFiles\scene7_01\samples\samples7_01d.tga 20 -random_samples 9
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_samples OutputFiles\scene7_01\samples\samples7_01e.tga 20 -uniform_samples 9
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_samples OutputFiles\scene7_01\samples\samples7_01f.tga 20 -jittered_samples 9
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_samples OutputFiles\scene7_01\samples\samples7_01g.tga 20 -random_samples 36
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_samples OutputFiles\scene7_01\samples\samples7_01h.tga 20 -uniform_samples 36
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_samples OutputFiles\scene7_01\samples\samples7_01i.tga 20 -jittered_samples 36

.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_filter OutputFiles\scene7_01\filters\filter7_01a.tga 20 -box_filter 0.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_filter OutputFiles\scene7_01\filters\filter7_01b.tga 20 -tent_filter 0.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_filter OutputFiles\scene7_01\filters\filter7_01c.tga 20 -gaussian_filter 0.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_filter OutputFiles\scene7_01\filters\filter7_01d.tga 20 -box_filter 1.7
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_filter OutputFiles\scene7_01\filters\filter7_01e.tga 20 -tent_filter 1.7
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_filter OutputFiles\scene7_01\filters\filter7_01f.tga 20 -gaussian_filter 1.7
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_filter OutputFiles\scene7_01\filters\filter7_01g.tga 20 -box_filter 2.3
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_filter OutputFiles\scene7_01\filters\filter7_01h.tga 20 -tent_filter 2.3
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 9 9 -render_filter OutputFiles\scene7_01\filters\filter7_01i.tga 20 -gaussian_filter 2.3

.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -output OutputFiles\scene7_01\outputs\output7_01_low_res.tga
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -render_samples OutputFiles\scene7_01\samples\samples7_01a_low_res.tga 15 -random_samples 9
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -render_samples OutputFiles\scene7_01\samples\samples7_01b_low_res.tga 15 -uniform_samples 9
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -render_samples OutputFiles\scene7_01\samples\samples7_01c_low_res.tga 15 -jittered_samples 9
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -render_filter OutputFiles\scene7_01\filters\filter7_01a_low_res.tga 15 -box_filter 0.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -render_filter OutputFiles\scene7_01\filters\filter7_01b_low_res.tga 15 -tent_filter 1.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -render_filter OutputFiles\scene7_01\filters\filter7_01c_low_res.tga 15 -gaussian_filter 1.0

.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -output OutputFiles\scene7_01\outputs\output7_01a_low_res.tga -random_samples 9 -box_filter 0.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -output OutputFiles\scene7_01\outputs\output7_01b_low_res.tga -random_samples 9 -tent_filter 1.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -output OutputFiles\scene7_01\outputs\output7_01c_low_res.tga -random_samples 9 -gaussian_filter 1.0
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -output OutputFiles\scene7_01\outputs\output7_01d_low_res.tga -uniform_samples 9 -box_filter 0.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -output OutputFiles\scene7_01\outputs\output7_01e_low_res.tga -uniform_samples 9 -tent_filter 1.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -output OutputFiles\scene7_01\outputs\output7_01f_low_res.tga -uniform_samples 9 -gaussian_filter 1.0
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -output OutputFiles\scene7_01\outputs\output7_01g_low_res.tga -jittered_samples 9 -box_filter 0.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -output OutputFiles\scene7_01\outputs\output7_01h_low_res.tga -jittered_samples 9 -tent_filter 1.5
.\raytracer.exe -input InputFiles\scene7_01_sphere_triangle.txt -size 12 12 -output OutputFiles\scene7_01\outputs\output7_01i_low_res.tga -jittered_samples 9 -gaussian_filter 1.0
```

### scene7_02 — 棋盘：采样 / 滤波 / 全分辨率抗锯齿

```powershell
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 180 180 -output OutputFiles\scene7_02\outputs\output7_02.tga

.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 9 9 -render_samples OutputFiles\scene7_02\samples\samples7_02a.tga 20 -random_samples 16
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 9 9 -render_samples OutputFiles\scene7_02\samples\samples7_02b.tga 20 -uniform_samples 16
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 9 9 -render_samples OutputFiles\scene7_02\samples\samples7_02c.tga 20 -jittered_samples 16
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 9 9 -render_filter OutputFiles\scene7_02\filters\filter7_02a.tga 20 -box_filter 0.5
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 9 9 -render_filter OutputFiles\scene7_02\filters\filter7_02b.tga 20 -tent_filter 1.5
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 9 9 -render_filter OutputFiles\scene7_02\filters\filter7_02c.tga 20 -gaussian_filter 0.6

.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 180 180 -output OutputFiles\scene7_02\outputs\output7_02a.tga -random_samples 16 -box_filter 0.5
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 180 180 -output OutputFiles\scene7_02\outputs\output7_02b.tga -random_samples 16 -tent_filter 1.5
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 180 180 -output OutputFiles\scene7_02\outputs\output7_02c.tga -random_samples 16 -gaussian_filter 0.6
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 180 180 -output OutputFiles\scene7_02\outputs\output7_02d.tga -uniform_samples 16 -box_filter 0.5
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 180 180 -output OutputFiles\scene7_02\outputs\output7_02e.tga -uniform_samples 16 -tent_filter 1.5
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 180 180 -output OutputFiles\scene7_02\outputs\output7_02f.tga -uniform_samples 16 -gaussian_filter 0.6
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 180 180 -output OutputFiles\scene7_02\outputs\output7_02g.tga -jittered_samples 16 -box_filter 0.5
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 180 180 -output OutputFiles\scene7_02\outputs\output7_02h.tga -jittered_samples 16 -tent_filter 1.5
.\raytracer.exe -input InputFiles\scene7_02_checkerboard.txt -size 180 180 -output OutputFiles\scene7_02\outputs\output7_02i.tga -jittered_samples 16 -gaussian_filter 0.6
```

### scene7_03 — 大理石花瓶（mesh，对比采样数）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene7_03_marble_vase.txt -size 200 200 -output ..\OutputFiles\scene7_03\outputs\output7_03a.tga -grid 15 30 15 -shadows
..\raytracer.exe -input ..\InputFiles\scene7_03_marble_vase.txt -size 200 200 -output ..\OutputFiles\scene7_03\outputs\output7_03b.tga -grid 15 30 15 -shadows -jittered_samples 4 -gaussian_filter 0.4
..\raytracer.exe -input ..\InputFiles\scene7_03_marble_vase.txt -size 200 200 -output ..\OutputFiles\scene7_03\outputs\output7_03c.tga -grid 15 30 15 -shadows -jittered_samples 9 -gaussian_filter 0.4
..\raytracer.exe -input ..\InputFiles\scene7_03_marble_vase.txt -size 200 200 -output ..\OutputFiles\scene7_03\outputs\output7_03d.tga -grid 15 30 15 -shadows -jittered_samples 36 -gaussian_filter 0.4
```

### scene7_04 — 6.837 logo（mesh）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene7_04_6.837_logo.txt -size 400 200 -output ..\OutputFiles\scene7_04\outputs\output7_04a.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 80 30 3
..\raytracer.exe -input ..\InputFiles\scene7_04_6.837_logo.txt -size 400 200 -output ..\OutputFiles\scene7_04\outputs\output7_04b.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 80 30 3 -jittered_samples 9 -gaussian_filter 0.4
```

### scene7_05 — 玻璃球（mesh）

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene7_05_glass_sphere.txt -size 300 300 -output ..\OutputFiles\scene7_05\outputs\output7_05a.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20
..\raytracer.exe -input ..\InputFiles\scene7_05_glass_sphere.txt -size 300 300 -output ..\OutputFiles\scene7_05\outputs\output7_05b.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20 -jittered_samples 4 -gaussian_filter 0.4
..\raytracer.exe -input ..\InputFiles\scene7_05_glass_sphere.txt -size 300 300 -output ..\OutputFiles\scene7_05\outputs\output7_05c.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20 -jittered_samples 16 -gaussian_filter 0.4
```

### scene7_06 — 钻石 mesh

```powershell
cd Meshes
..\raytracer.exe -input ..\InputFiles\scene7_06_faceted_gem.txt -size 200 200 -output ..\OutputFiles\scene7_06\outputs\output7_06a.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20
..\raytracer.exe -input ..\InputFiles\scene7_06_faceted_gem.txt -size 200 200 -output ..\OutputFiles\scene7_06\outputs\output7_06b.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20 -jittered_samples 9 -gaussian_filter 0.4
```
