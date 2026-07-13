Assignment 9: Particle Systems

Implemented:
  Phase 1: Gravity/Constant + Euler + Hose
  Phase 2: Radial + Midpoint
  Phase 3: Vertical + RingGenerator
  Extra Credit: RungeKuttaIntegrator (RK4), WindForceField

Build:
  mingw32-make

GUI 快捷键：p 暂停/继续 | s 单步 | r 重启 | q 退出
鼠标操作：左键拖动旋转 | 右键拖动缩放

----------------------------------------------------------------------
Common command-line parameters
----------------------------------------------------------------------
  -input <file>              场景描述文件（InputFiles/system9_*.txt）
  -refresh <seconds>         GUI 重绘 / System::Update 调用间隔（秒）
  -dt <seconds>              每次 Update 的仿真时间步长（秒）
  -motion_blur               在 last_position 与 position 之间画运动模糊线
  -draw_vectors <scale>      绘制速度向量（粒子颜色）与加速度向量（白色）
  -integrator_color          按积分器着色（Euler=红，Midpoint=绿，RK4=蓝）

----------------------------------------------------------------------
Sample Results
----------------------------------------------------------------------

--- system9_01_hose：基础软管，无力场 ---
# 较大时间步；三组不同随机性的软管直线喷射
./particle_system -input InputFiles/system9_01_hose.txt -refresh 0.1 -dt 0.1

# 较小时间步；粒子发射更平滑
./particle_system -input InputFiles/system9_01_hose.txt -refresh 0.05 -dt 0.05

# 较大时间步 + 运动模糊拖尾
./particle_system -input InputFiles/system9_01_hose.txt -refresh 0.1 -dt 0.1 -motion_blur

# 较小时间步 + 运动模糊拖尾
./particle_system -input InputFiles/system9_01_hose.txt -refresh 0.05 -dt 0.05 -motion_blur


--- system9_02_hose_gravity：软管 + 重力---
# 绘制速度/加速度向量；质量不同但在重力下下落相同
./particle_system -input InputFiles/system9_02_hose_gravity.txt -refresh 0.05 -dt 0.05 -draw_vectors 0.1

# 同一场景 + 运动模糊
./particle_system -input InputFiles/system9_02_hose_gravity.txt -refresh 0.05 -dt 0.05 -motion_blur


--- system9_03_hose_force：软管 + 恒定力（a = F/m）---
# 绘制向量；质量大的粒子加速度更小
./particle_system -input InputFiles/system9_03_hose_force.txt -refresh 0.05 -dt 0.05 -draw_vectors 0.1

# 同一场景 + 运动模糊
./particle_system -input InputFiles/system9_03_hose_force.txt -refresh 0.05 -dt 0.05 -motion_blur


--- system9_04_circle_euler：径向力场 + Euler（轨道发散）---
# 较大 dt；积分器着色 + 加速度向量，可见螺旋外扩
./particle_system -input InputFiles/system9_04_circle_euler.txt -refresh 0.1 -dt 0.1 -integrator_color -draw_vectors 0.02

# 中等 dt + 运动模糊
./particle_system -input InputFiles/system9_04_circle_euler.txt -refresh 0.05 -dt 0.05 -integrator_color -motion_blur

# 较小 dt + 运动模糊（不如 Midpoint / RK4 精确）
./particle_system -input InputFiles/system9_04_circle_euler.txt -refresh 0.01 -dt 0.01 -integrator_color -motion_blur


--- system9_05_circle_midpoint：径向力场 + Midpoint（接近圆轨）---
# 较大 dt；绿色粒子比 Euler 更接近单位圆
./particle_system -input InputFiles/system9_05_circle_midpoint.txt -refresh 0.1 -dt 0.1 -integrator_color -draw_vectors 0.02

# 中等 dt + 运动模糊
./particle_system -input InputFiles/system9_05_circle_midpoint.txt -refresh 0.05 -dt 0.05 -integrator_color -motion_blur

# 较小 dt + 运动模糊
./particle_system -input InputFiles/system9_05_circle_midpoint.txt -refresh 0.01 -dt 0.01 -integrator_color -motion_blur


--- system9_06_circle_rungekutta：径向力场 + RK4---
# 较大 dt；蓝色轨道最接近理想圆
./particle_system -input InputFiles/system9_06_circle_rungekutta.txt -refresh 0.1 -dt 0.1 -integrator_color -draw_vectors 0.02

# 中等 dt + 运动模糊
./particle_system -input InputFiles/system9_06_circle_rungekutta.txt -refresh 0.05 -dt 0.05 -integrator_color -motion_blur

# 较小 dt + 运动模糊
./particle_system -input InputFiles/system9_06_circle_rungekutta.txt -refresh 0.01 -dt 0.01 -integrator_color -motion_blur


--- system9_07_wave：垂直力场；Euler / Midpoint / RK4 对比 ---
# 较大 dt；对比同一波浪场景下三种积分器精度
./particle_system -input InputFiles/system9_07_wave.txt -refresh 0.01 -dt 0.2 -integrator_color -motion_blur

# 中等 dt
./particle_system -input InputFiles/system9_07_wave.txt -refresh 0.01 -dt 0.05 -integrator_color -motion_blur

# 较小 dt
./particle_system -input InputFiles/system9_07_wave.txt -refresh 0.01 -dt 0.03 -integrator_color -motion_blur
./particle_system -input InputFiles/system9_07_wave.txt -refresh 0.01 -dt 0.02 -integrator_color -motion_blur
./particle_system -input InputFiles/system9_07_wave.txt -refresh 0.01 -dt 0.01 -integrator_color -motion_blur


--- system9_08_fire：扩张环生成器 + 向下恒定力 ---
# 带地面的火焰环；运动模糊
./particle_system -input InputFiles/system9_08_fire.txt -refresh 0.05 -dt 0.05 -motion_blur


--- system9_09_wind：时变风场---
# 向上喷射的软管被水平风弯曲；运动模糊
./particle_system -input InputFiles/system9_09_wind.txt -motion_blur -dt 0.05 -refresh 0.05
