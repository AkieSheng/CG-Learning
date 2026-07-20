# AS8

## 编译

```powershell
cd E:\Akie\Assignment\AS8
mingw32-make
```

## 参数说明

| 参数 | 说明 |
|------|------|
| `-input <file>` | 样条描述文件 |
| `-gui` | 启动 OpenGL 交互窗口 |
| `-curve_tessellation <n>` | 曲线 Q(t) 采样密度（默认 10）；亦影响旋转曲面沿轮廓方向分段 |
| `-revolution_tessellation <n>` | 旋转曲面绕 y 轴分段数（默认 10） |
| `-patch_tessellation <n>` | Bezier Patch 网格密度（默认 10） |
| `-output <file.obj>` | 导出三角网格 `.obj` |
| `-output_bezier <file.txt>` | 导出 Bezier 格式样条 |
| `-output_bspline <file.txt>` | 导出 BSpline 格式样条 |

GUI：`s` 保存；`q` 退出。左键拖拽控制点；中键在控制边插入点（仅 BSpline）；右键删除控制点（仅 BSpline）。

## 样例

### spline8_01 — 两段 4 点 Bezier

```powershell
.\curve_editor.exe -input InputFiles\spline8_01_bezier.txt -gui -curve_tessellation 30
.\curve_editor.exe -input InputFiles\spline8_01_bezier.txt -output_bezier OutputFiles\output8_01_bezier.txt -output_bspline OutputFiles\output8_01_bspline.txt
.\curve_editor.exe -input OutputFiles\output8_01_bspline.txt -gui -curve_tessellation 30
```

### spline8_02 — 两段 4 点 BSpline

```powershell
.\curve_editor.exe -input InputFiles\spline8_02_bspline.txt -gui -curve_tessellation 30
.\curve_editor.exe -input InputFiles\spline8_02_bspline.txt -output_bezier OutputFiles\output8_02_bezier.txt -output_bspline OutputFiles\output8_02_bspline.txt
.\curve_editor.exe -input OutputFiles\output8_02_bezier.txt -gui -curve_tessellation 30
```

### spline8_03 — 多段 Bezier（对比 curve_tessellation）

```powershell
.\curve_editor.exe -input InputFiles\spline8_03_bezier.txt -gui -curve_tessellation 5
.\curve_editor.exe -input InputFiles\spline8_03_bezier.txt -gui -curve_tessellation 30
.\curve_editor.exe -input InputFiles\spline8_03_bezier.txt -gui -curve_tessellation 100
```

### spline8_04 — 多段 BSpline

```powershell
.\curve_editor.exe -input InputFiles\spline8_04_bspline.txt -gui -curve_tessellation 30
```

### spline8_05 — 含重复控制点的 BSpline

```powershell
.\curve_editor.exe -input InputFiles\spline8_05_bspline_dups.txt -gui -curve_tessellation 30
```

### spline8_06 — 圆环旋转曲面

```powershell
.\curve_editor.exe -input InputFiles\spline8_06_torus.txt -curve_tessellation 4 -gui
.\curve_editor.exe -input InputFiles\spline8_06_torus.txt -curve_tessellation 4 -revolution_tessellation 10 -output OutputFiles\torus_low.obj
.\curve_editor.exe -input InputFiles\spline8_06_torus.txt -curve_tessellation 30 -revolution_tessellation 60 -output OutputFiles\torus_high.obj
```

### spline8_07 — 花瓶旋转曲面

```powershell
.\curve_editor.exe -input InputFiles\spline8_07_vase.txt -curve_tessellation 4 -output_bspline OutputFiles\output8_07_edit.txt -gui
.\curve_editor.exe -input OutputFiles\output8_07_edit.txt -curve_tessellation 4 -revolution_tessellation 10 -output OutputFiles\vase_low.obj
.\curve_editor.exe -input OutputFiles\output8_07_edit.txt -curve_tessellation 10 -revolution_tessellation 60 -output OutputFiles\vase_high.obj
```

### spline8_08 — 单块 4×4 Bezier Patch

```powershell
.\curve_editor.exe -input InputFiles\spline8_08_bezier_patch.txt -gui
.\curve_editor.exe -input InputFiles\spline8_08_bezier_patch.txt -patch_tessellation 4 -output OutputFiles\patch_low.obj
.\curve_editor.exe -input InputFiles\spline8_08_bezier_patch.txt -patch_tessellation 10 -output OutputFiles\patch_med.obj
.\curve_editor.exe -input InputFiles\spline8_08_bezier_patch.txt -patch_tessellation 40 -output OutputFiles\patch_high.obj
```

### spline8_09 — 茶壶（旋转体 + Patch）

```powershell
.\curve_editor.exe -input InputFiles\spline8_09_teapot.txt -curve_tessellation 4 -gui
.\curve_editor.exe -input InputFiles\spline8_09_teapot.txt -patch_tessellation 4 -curve_tessellation 4 -revolution_tessellation 10 -output OutputFiles\teapot_low.obj
.\curve_editor.exe -input InputFiles\spline8_09_teapot.txt -patch_tessellation 30 -curve_tessellation 30 -revolution_tessellation 100 -output OutputFiles\teapot_high.obj
```
