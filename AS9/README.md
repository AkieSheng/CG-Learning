# AS9

## 编译

```powershell
cd E:\Akie\Assignment\AS9
mingw32-make
```

## 参数说明

| 参数 | 说明 |
|------|------|
| `-input <file>` | 场景描述文件（`InputFiles\system9_*.txt`） |
| `-refresh <seconds>` | GUI 重绘 / `System::Update` 调用间隔（秒） |
| `-dt <seconds>` | 每次 Update 的仿真时间步长（秒） |
| `-motion_blur` | 在 `last_position` 与 `position` 之间画运动模糊线 |
| `-draw_vectors <scale>` | 绘制速度向量（粒子色）与加速度向量（白色），scale 为长度缩放 |
| `-integrator_color` | 按积分器着色（Euler=红，Midpoint=绿，RK4=蓝） |

GUI：`p` 暂停/继续；`s` 单步；`r` 重启；`q` 退出。左键旋转；右键缩放。

## 样例

### system9_01 — 基础软管（无力场）

```powershell
.\particle_system.exe -input InputFiles\system9_01_hose.txt -refresh 0.1 -dt 0.1
.\particle_system.exe -input InputFiles\system9_01_hose.txt -refresh 0.05 -dt 0.05
.\particle_system.exe -input InputFiles\system9_01_hose.txt -refresh 0.1 -dt 0.1 -motion_blur
.\particle_system.exe -input InputFiles\system9_01_hose.txt -refresh 0.05 -dt 0.05 -motion_blur
```

### system9_02 — 软管 + 重力

```powershell
.\particle_system.exe -input InputFiles\system9_02_hose_gravity.txt -refresh 0.05 -dt 0.05 -draw_vectors 0.1
.\particle_system.exe -input InputFiles\system9_02_hose_gravity.txt -refresh 0.05 -dt 0.05 -motion_blur
```

### system9_03 — 软管 + 恒定力（a = F/m）

```powershell
.\particle_system.exe -input InputFiles\system9_03_hose_force.txt -refresh 0.05 -dt 0.05 -draw_vectors 0.1
.\particle_system.exe -input InputFiles\system9_03_hose_force.txt -refresh 0.05 -dt 0.05 -motion_blur
```

### system9_04 — 径向力场 + Euler

```powershell
.\particle_system.exe -input InputFiles\system9_04_circle_euler.txt -refresh 0.1 -dt 0.1 -integrator_color -draw_vectors 0.02
.\particle_system.exe -input InputFiles\system9_04_circle_euler.txt -refresh 0.05 -dt 0.05 -integrator_color -motion_blur
.\particle_system.exe -input InputFiles\system9_04_circle_euler.txt -refresh 0.01 -dt 0.01 -integrator_color -motion_blur
```

### system9_05 — 径向力场 + Midpoint

```powershell
.\particle_system.exe -input InputFiles\system9_05_circle_midpoint.txt -refresh 0.1 -dt 0.1 -integrator_color -draw_vectors 0.02
.\particle_system.exe -input InputFiles\system9_05_circle_midpoint.txt -refresh 0.05 -dt 0.05 -integrator_color -motion_blur
.\particle_system.exe -input InputFiles\system9_05_circle_midpoint.txt -refresh 0.01 -dt 0.01 -integrator_color -motion_blur
```

### system9_06 — 径向力场 + RK4

```powershell
.\particle_system.exe -input InputFiles\system9_06_circle_rungekutta.txt -refresh 0.1 -dt 0.1 -integrator_color -draw_vectors 0.02
.\particle_system.exe -input InputFiles\system9_06_circle_rungekutta.txt -refresh 0.05 -dt 0.05 -integrator_color -motion_blur
.\particle_system.exe -input InputFiles\system9_06_circle_rungekutta.txt -refresh 0.01 -dt 0.01 -integrator_color -motion_blur
```

### system9_07 — 垂直力场波浪（三种积分器对比）

```powershell
.\particle_system.exe -input InputFiles\system9_07_wave.txt -refresh 0.01 -dt 0.2 -integrator_color -motion_blur
.\particle_system.exe -input InputFiles\system9_07_wave.txt -refresh 0.01 -dt 0.05 -integrator_color -motion_blur
.\particle_system.exe -input InputFiles\system9_07_wave.txt -refresh 0.01 -dt 0.03 -integrator_color -motion_blur
.\particle_system.exe -input InputFiles\system9_07_wave.txt -refresh 0.01 -dt 0.02 -integrator_color -motion_blur
.\particle_system.exe -input InputFiles\system9_07_wave.txt -refresh 0.01 -dt 0.01 -integrator_color -motion_blur
```

### system9_08 — 火焰环（扩张环生成器 + 向下恒定力）

```powershell
.\particle_system.exe -input InputFiles\system9_08_fire.txt -refresh 0.05 -dt 0.05 -motion_blur
```

### system9_09 — 时变风场

```powershell
.\particle_system.exe -input InputFiles\system9_09_wind.txt -motion_blur -dt 0.05 -refresh 0.05
```
