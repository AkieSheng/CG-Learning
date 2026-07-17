# Assignment 9: Particle Systems

作者：王洵

## 1 实验原理

### 1.1 粒子状态与牛顿动力学

粒子系统（Particle System）将大量离散粒子视为质点集合。每个粒子在时刻 $t$ 由其位置 $\mathbf{p}(t)$、速度 $\mathbf{v}(t)$、质量 $m$ 及剩余寿命等属性刻画。在牛顿力学框架下，合外力 $\mathbf{F}$ 与加速度 $\mathbf{a}$ 满足

$$
\mathbf{F}=m\mathbf{a}.
$$

外场有两种常见给法：若直接给出加速度（例如近似均匀的地表重力 $\mathbf{g}$），则 $\mathbf{a}=\mathbf{g}$，质量不进入加速度计算；若给出力 $\mathbf{F}$，则

$$
\mathbf{a}=\frac{\mathbf{F}}{m},
$$

质量越大，同一力下加速度越小。粒子状态随时间的演化写成一阶常微分方程组

$$
\frac{\mathrm{d}\mathbf{p}}{\mathrm{d}t}=\mathbf{v},\qquad
\frac{\mathrm{d}\mathbf{v}}{\mathrm{d}t}=\mathbf{a}(\mathbf{p},m,t),
$$

其中 $\mathbf{a}$ 随位置与时间变化。数值仿真在离散步长 $\Delta t$ 上推进该系统，由初始状态生成轨迹。

### 1.2 力场模型

力场通过规定 $\mathbf{a}(\mathbf{p},m,t)$ 决定粒子动力学。本报告用到的几类力场如下。

重力场。 加速度恒为 $\mathbf{g}$，与 $\mathbf{p}$、$m$、$t$ 无关。

恒定力场。 力向量 $\mathbf{F}$ 恒定，加速度为 $\mathbf{F}/m$。

径向力场。 取

$$
\mathbf{a}(\mathbf{p})=-\kappa\mathbf{p},\qquad \kappa>0,
$$

其中 $\kappa$ 为强度系数。加速度始终指向原点，模长与到原点的距离成正比。在合适的初速与 $\kappa$ 下，质点近似沿圆周运动，用于比较不同积分格式的轨道误差。

垂直力场。 取

$$
\mathbf{a}=(0,-\kappa\,p_y,0),
$$

将粒子拉向平面 $y=0$；偏离平面越远，回复加速度越大，轨迹呈振荡或波动状。

时变风场。 水平力随时间 $t$ 与高度 $p_y$ 变化，例如

$$
\mathbf{F}=\mu\Bigl(\sin\bigl(2t+\tfrac12 p_y\bigr),\,0,\,\cos\bigl(\tfrac32 t+\tfrac{3}{10}p_y\bigr)\Bigr),
$$

再取 $\mathbf{a}=\mathbf{F}/m$。$\mu$ 控制风力强弱；相位中含 $p_y$ 后，喷流上下段受力不同，轨迹在水平方向弯曲。

### 1.3 数值积分

#### 1.3.Euler method

Euler method 在当前状态处取一阶近似：

$$
\mathbf{p}_{n+1}=\mathbf{p}_n+\mathbf{v}_n\Delta t,\qquad
\mathbf{v}_{n+1}=\mathbf{v}_n+\mathbf{a}(\mathbf{p}_n,t_n)\Delta t.
$$

其中 $\mathbf{p}_n$、$\mathbf{v}_n$ 为第 $n$ 步的位置与速度，$t_n$ 为当前时刻，$\Delta t$ 为步长。局部截断误差为 $O((\Delta t)^2)$；在曲率较大的力场中，能量易伪增，圆周轨道会逐渐外扩。

#### 1.3.2 Midpoint method

Midpoint method 先以半步构造中点状态

$$
\mathbf{p}_m=\mathbf{p}_n+\mathbf{v}_n\frac{\Delta t}{2},\qquad
\mathbf{v}_m=\mathbf{v}_n+\mathbf{a}(\mathbf{p}_n,t_n)\frac{\Delta t}{2},
$$

再用中点处的速度与加速度完成全步：

$$
\mathbf{p}_{n+1}=\mathbf{p}_n+\mathbf{v}_m\Delta t,\qquad
\mathbf{v}_{n+1}=\mathbf{v}_n+\mathbf{a}\bigl(\mathbf{p}_m,\,t_n+\tfrac{\Delta t}{2}\bigr)\Delta t.
$$

对光滑右端函数，该方法具有二阶精度；相同 $\Delta t$ 下，轨道通常比 Euler 更接近解析解。

#### 1.3.3 RK4

四阶 Runge–Kutta method（RK4）在一步内对状态导数作四点加权。对状态 $(\mathbf{p},\mathbf{v})$，令

$$
\begin{aligned}
\mathbf{k}_{1p}&=\mathbf{v}_n,&
\mathbf{k}_{1v}&=\mathbf{a}(\mathbf{p}_n,t_n),\\
\mathbf{k}_{2p}&=\mathbf{v}_n+\tfrac{\Delta t}{2}\mathbf{k}_{1v},&
\mathbf{k}_{2v}&=\mathbf{a}\bigl(\mathbf{p}_n+\tfrac{\Delta t}{2}\mathbf{k}_{1p},\,t_n+\tfrac{\Delta t}{2}\bigr),\\
\mathbf{k}_{3p}&=\mathbf{v}_n+\tfrac{\Delta t}{2}\mathbf{k}_{2v},&
\mathbf{k}_{3v}&=\mathbf{a}\bigl(\mathbf{p}_n+\tfrac{\Delta t}{2}\mathbf{k}_{2p},\,t_n+\tfrac{\Delta t}{2}\bigr),\\
\mathbf{k}_{4p}&=\mathbf{v}_n+\Delta t\,\mathbf{k}_{3v},&
\mathbf{k}_{4v}&=\mathbf{a}\bigl(\mathbf{p}_n+\Delta t\,\mathbf{k}_{3p},\,t_n+\Delta t\bigr),
\end{aligned}
$$

再合成

$$
\mathbf{p}_{n+1}=\mathbf{p}_n+\frac{\Delta t}{6}(\mathbf{k}_{1p}+2\mathbf{k}_{2p}+2\mathbf{k}_{3p}+\mathbf{k}_{4p}),
$$

速度更新形式相同。更高阶的局部误差使圆轨道与波形在较大步长下仍更接近解析解。

### 1.4 粒子发射

粒子由发射器按时间步产生。稳态情形下，若期望同时存活约 $N$ 个粒子、平均寿命为 $L$，则单位时间应发射约 $N/L$ 个粒子，故步长 $\Delta t$ 内新生数约为

$$
n\approx\Delta t\cdot\frac{N}{L},
$$

使生灭大致平衡。

软管发射器在空间点 $\mathbf{p}_0$ 附近采样位置，并沿标称速度 $\mathbf{v}_0$ 叠加随机扰动：

$$
\mathbf{p}=\mathbf{p}_0+r_p\,\boldsymbol{\xi},\qquad
\mathbf{v}=\mathbf{v}_0+r_v\,\boldsymbol{\xi},
$$

其中 $r_p$、$r_v$ 为位置与速度随机幅度，$\boldsymbol{\xi}$ 为伪随机向量。

环形发射器将粒子置于随时间扩张的圆环上：半径取当前仿真时间 $r=t$，角 $\theta$ 在 $[0,2\pi)$ 上均匀随机；每步新生数再乘以 $t$，使环上线密度近似恒定。颜色、质量与寿命在标称值上叠加有界随机扰动。固定种子的伪随机序列使相同参数下的轨迹复现，从而能比较不同积分格式与力场。

## 2 程序设计与实现

### 2.1 总体架构

仿真由场景解析、力场、积分器与粒子发射器组成。解析模块读入各场景描述，为每一套粒子系统装配发射器、积分器与力场。交互画布按刷新间隔周期推进仿真，并按命令行选项决定是否以积分器颜色绘制粒子、是否绘制速度与加速度向量、以及是否启用运动模糊。

每个时间步的流程为：先推进现有粒子，再按发射策略生成新粒子，最后移除寿命耗尽者并推进仿真时钟。

```cpp
void System::Update(float dt) {
  int num_particles = particles->getNumParticles();
  for (int i = 0; i < num_particles; i++) {
    integrator->Update(particles->Get(i), forcefield, current_time, dt);
  }
  int num_new = generator->numNewParticles(current_time, dt);
  for (int i = 0; i < num_new; i++) {
    Particle *p = generator->Generate(current_time, i);
    particles->Add(p);
  }
  particles->RemoveDead();
  current_time += dt;
}
```

重启时清空粒子集合、以固定种子重建发射器的伪随机流，并将时钟归零。

### 2.2 力场

力场以抽象接口给出：由当前位置、质量与时刻返回加速度向量。

```cpp
virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const = 0;
```

各具体力场的核心计算如下。

重力场直接返回重力加速度向量：

```cpp
Vec3f GravityForceField::getAcceleration(...) const {
  return gravity;
}
```

恒定力场按牛顿第二定律除以质量：

```cpp
return force * (1.0f / mass);
```

径向力场与垂直力场：

```cpp
return -magnitude * position;
// ——
return Vec3f(0, -magnitude * position.y(), 0);
```

时变风场先构造水平力，再除以质量：

```cpp
float wx = sinf(2.0f * t + 0.5f * position.y());
float wz = cosf(1.5f * t + 0.3f * position.y());
Vec3f force = magnitude * Vec3f(wx, 0, wz);
return force * (1.0f / mass);
```

### 2.3 积分器

积分器在给定力场、当前时刻与步长下更新单个粒子的位置与速度，并在更新末尾增加粒子年龄。可视化时，Euler 为红、中点法为绿、RK4 为蓝。

Euler 积分器：

```cpp
Vec3f a = forcefield->getAcceleration(p, mass, t);
particle->setPosition(p + dt * v);
particle->setVelocity(v + dt * a);
particle->increaseAge(dt);
```

中点积分器先取半步中点，再在中点加速度下完成全步：

```cpp
Vec3f a_n = forcefield->getAcceleration(pn, mass, t);
Vec3f pm = pn + (dt * 0.5f) * vn;
Vec3f vm = vn + (dt * 0.5f) * a_n;
Vec3f a_m = forcefield->getAcceleration(pm, mass, t + dt * 0.5f);
particle->setPosition(pn + dt * vm);
particle->setVelocity(vn + dt * a_m);
```

四阶 Runge–Kutta 积分器对 $(\mathbf{p},\mathbf{v})$ 做四点加权后合成：

```cpp
Vec3f k1p = vn;
Vec3f k1v = forcefield->getAcceleration(pn, mass, t);
// k2、k3、k4 同理……
float sixth = h / 6.0f;
particle->setPosition(pn + sixth * (k1p + 2.0f * k2p + 2.0f * k3p + k4p));
particle->setVelocity(vn + sixth * (k1v + 2.0f * k2v + 2.0f * k3v + k4v));
```

### 2.4 粒子发射器

发射器记录颜色与死亡色、质量、寿命、期望同时存活的粒子数 $N$，以及伪随机流。稳态下每步新生数为

```cpp
int n = (int)(dt * desired_num_particles / lifespan);
if (n == 0 && dt > 0 && desired_num_particles > 0)
  n = 1;
```

当 $\Delta t$ 较小使整型截断为零时，仍生成 $1$ 个粒子，使发射连续。颜色与质量、寿命在标称值上叠加随机扰动。

软管发射器在标称位置与速度上叠加随机向量：

```cpp
Vec3f p = position + position_randomness * rng->randomVector();
Vec3f v = velocity + velocity_randomness * rng->randomVector();
return new Particle(p, v, c, dead_color, m, life);
```

环形发射器以当前时间为半径，在 $y=-4$ 平面上采样圆环，并使新生数随时间增大：

```cpp
int n = (int)(dt * desired_num_particles / lifespan * current_time);
// ——
float radius = current_time;
float theta = float(2.0 * M_PI) * rng->next();
Vec3f p(radius * cosf(theta), RING_GROUND_Y, radius * sinf(theta));
```

绘制时额外画出地面参考四边形。重启时以固定种子重建伪随机流。

## 3 实验结果

交互式运行示例：

```text
# 软管喷射；刷新间隔、仿真步长、运动模糊
particle_system -input <软管喷射场景> -refresh 0.05 -dt 0.05 -motion_blur
# 径向场圆周 + 显式欧拉；按积分器着色
particle_system -input <径向场圆周·欧拉场景> -refresh 0.01 -dt 0.01 -integrator_color -motion_blur
# 径向场圆周 + 中点法
particle_system -input <径向场圆周·中点法场景> -refresh 0.01 -dt 0.01 -integrator_color -motion_blur
# 环形火焰
particle_system -input <环形火焰场景> -refresh 0.05 -dt 0.05 -motion_blur
```

软管场景观察位置与速度随机性，以及重力、恒力下质量对轨迹的影响；径向场下 Euler 轨道外扩，中点法与 RK4 更接近圆；垂直场波浪场景中三种积分器在相同 $\Delta t$ 下精度差异清晰；环形火焰与时变风场分别对应变密度发射与水平弯曲喷流。

### 3.1 软管喷射

| 较大 $\Delta t$ | 较小 $\Delta t$ | 较大 $\Delta t$ + 运动模糊 | 较小 $\Delta t$ + 运动模糊 |
|:---:|:---:|:---:|:---:|
| ![output9_01a](OutputFiles/output9_01a.png) | ![output9_01b](OutputFiles/output9_01b.png) | ![output9_01c](OutputFiles/output9_01c.png) | ![output9_01d](OutputFiles/output9_01d.png) |
| 三组软管直线喷射 | 粒子分布更连续 | 相邻位置连线成拖尾 | 拖尾更细密 |

三组软管在零恒力下沿标称速度方向喷射。位置与速度随机幅度不同，束状扩散程度不同。减小 $\Delta t$ 后粒子分布更连续；运动模糊在相邻位置间连线，形成拖尾。

### 3.2 软管 + 重力

| 重力下落 | 运动模糊 | 速度／加速度向量 |
|:---:|:---:|:---:|
| ![output9_02a](OutputFiles/output9_02a.png) | ![output9_02b](OutputFiles/output9_02b.png) | ![output9_02c](OutputFiles/output9_02c.png) |
| 抛物线轨迹 | 拖尾可见 | 速度（粒子色）与加速度（白） |

重力加速度与质量无关，不同质量粒子在相同初速下呈相近抛物线下落。绘制速度与加速度向量时，速度沿粒子色线段显示，加速度以白色线段显示并按给定比例缩放。

### 3.3 软管 + 恒力

| 恒力下落 | 运动模糊 | 向量可视化 |
|:---:|:---:|:---:|
| ![output9_03a](OutputFiles/output9_03a.png) | ![output9_03b](OutputFiles/output9_03b.png) | ![output9_03c](OutputFiles/output9_03c.png) |
| $\mathbf{F}=(0,-9.8,0)$ | 拖尾 | 质量大则下落更缓 |

恒力 $\mathbf{F}=(0,-9.8,0)$ 下加速度为 $\mathbf{F}/m$，质量大的粒子下落更缓，与重力场场景形成对比，说明力场与加速度场在物理语义上的差别。

### 3.4 径向场 + 显式 Euler

| $\Delta t=0.1$ | $\Delta t=0.05$ | $\Delta t=0.01$ |
|:---:|:---:|:---:|
| ![output9_04a](OutputFiles/output9_04a.png) | ![output9_04b](OutputFiles/output9_04b.png) | ![output9_04c](OutputFiles/output9_04c.png) |
| 轨道螺旋外扩明显 | 外扩减缓 | 外扩进一步减弱 |

单位圆附近初值在 $\kappa=25$ 的径向场中，Euler 使能量伪增，轨道螺旋外扩。$\Delta t$ 越小，外扩越慢；同一场景下高阶格式的轨道闭合程度更高。

### 3.5 径向场 + 中点法

| $\Delta t=0.1$ | $\Delta t=0.05$ | $\Delta t=0.01$ |
|:---:|:---:|:---:|
| ![output9_05a](OutputFiles/output9_05a.png) | ![output9_05b](OutputFiles/output9_05b.png) | ![output9_05c](OutputFiles/output9_05c.png) |
| 绿色，接近单位圆 | 轨迹近似闭合 | 运动模糊闭合良好 |

相同径向场下，中点法（绿色）在较大 $\Delta t$ 时仍接近单位圆，运动模糊轨迹近似闭合。

### 3.6 径向场 + Runge–Kutta

| $\Delta t=0.1$ | $\Delta t=0.05$ | $\Delta t=0.01$ |
|:---:|:---:|:---:|
| ![output9_06a](OutputFiles/output9_06a.png) | ![output9_06b](OutputFiles/output9_06b.png) | ![output9_06c](OutputFiles/output9_06c.png) |
| 蓝色，最接近理想圆周 | 精度高于中点法 | 与解析轨道几乎重合 |

RK4（蓝色）在相同步长下轨道最接近理想圆周。Euler、中点法、RK4 的闭合程度依次提高，和数值积分阶数提高后局部误差减小的预期相符。

### 3.7 垂直场波浪对比

| $\Delta t=0.2$ | $\Delta t$ 中等 | $\Delta t=0.01$ |
|:---:|:---:|:---:|
| ![output9_07a](OutputFiles/output9_07a.png) | ![output9_07b](OutputFiles/output9_07b.png) | ![output9_07c](OutputFiles/output9_07c.png) |
| 红／绿／蓝三者分离明显 | 低阶误差收敛中 | 三者逐渐靠拢 |

同一垂直场中并列 Euler（红）、中点法（绿）、RK4（蓝）。$\Delta t$ 从 $0.2$ 减至 $0.01$ 时，低阶格式的波形误差收敛，三条曲线逐渐靠拢。

### 3.8 环形火焰

| 火焰环 a | 火焰环 b | 火焰环 c |
|:---:|:---:|:---:|
| ![output9_08a](OutputFiles/output9_08a.png) | ![output9_08b](OutputFiles/output9_08b.png) | ![output9_08c](OutputFiles/output9_08c.png) |
| 扩张环上发射 | 黄→红寿命着色 | 环半径随时间增大 |

环形发射器在扩张环上发射粒子，配合向下恒力与黄→红的寿命着色，形成火焰环。环半径与每步新生数随时间增大，环上线密度大致稳定。

### 3.9 时变风场

| 风场 a | 风场 b | 风场 c |
|:---:|:---:|:---:|
| ![output9_09a](OutputFiles/output9_09a.png) | ![output9_09b](OutputFiles/output9_09b.png) | ![output9_09c](OutputFiles/output9_09c.png) |
| 向上喷流受风弯曲 | 不同高度风向差异 | 轨迹呈摆动状 |

向上软管喷流受水平时变风力作用，不同时刻与高度处的风向差异使轨迹呈摆动状弯曲。

## 4 问题记录

径向场圆轨道场景在 $\Delta t=0.01$ 且开启运动模糊时一度整屏无粒子，起初怀疑是 RK4 在小步长下数值发散。单独对 RK4 做测试后，发现积分本身没有问题。而对照发射逻辑后发现，每步新生数为 $\lfloor\Delta t\cdot N/L\rfloor$，场景默认值 $N=1000$、$L=10$，当 $\Delta t<0.01$ 时整体截断为 $0$，重启后粒子集为空且未进行补充。叠加运动模糊后稀疏粒子更难察觉。修复方案：在截断结果为 $0$ 但 $\Delta t>0$ 且期望粒子数大于 $0$ 时，强制至少生成 $1$ 个粒子，小步长的场景出现，问题解决。

交互预览中左键旋转可用，右键拖动缩放几乎无效。多次检查后未找到原有，最终由 Agent 发现原因如下：Windows 上 freeglut 默认占用右键弹出菜单，拖动事件常被拦截；原缩放用 $\mathrm{pow}(10,0.001\Delta x)$，系数过小且仅依赖水平位移，体感不明显。解决方法：创建窗口后调用 `glutSetMenu(0)` 禁用右键菜单，并将缩放改为沿视线方向按拖动位移线性推拉相机距离（限制在合理区间内）。按此方案修复后问题解决。

## 5 总结

将粒子状态置于牛顿力学 ODE 框架中，由力场给出位置与时间相关的加速度，再用 Euler、中点法与 RK4 等显式积分格式按步长推进位置与速度，并从软管或扩张环发射器按稳态或变密度策略补充新生粒子。径向场与垂直场使不同积分阶数的轨道误差得以直观对比；重力与恒力分别对应加速度场与 $\mathbf{F}/m$ 两种物理语义；环形火焰与时变风场则体现发射几何与时变外力对视觉效果的影响。固定种子的伪随机流使重启后轨迹复现，各测试场景的运动形态和数值积分及力场模型的预期行为相符。

## 附录（TODO list 记录）

- [x] 实现重力场、恒定力场、径向力场与垂直力场，并由位置、质量与时刻计算加速度（重力直接取 $\mathbf{g}$，恒力取 $\mathbf{F}/m$，径向与垂直场强度分别正比于到原点或到 $y=0$ 平面的距离）
- [x] 实现显式 Euler 积分器与中点积分器，按力场与步长更新位置、速度并增加粒子年龄；可视化时分别着红色与绿色
- [x] 实现软管发射器与环形发射器，设置颜色、寿命、质量及随机幅度，按 $\Delta t\cdot N/L$ 计算每步新生数并叠加随机扰动；以固定种子重启伪随机流；环形发射器半径与新生数随仿真时间增大，并绘制参考地面
- [x] 实现四阶 Runge–Kutta 积分器（可视化为蓝色）
- [x] 实现时变风场（水平风力随时间与高度变化，$\mathbf{a}=\mathbf{F}/m$）
- [ ] 设计并实现新型粒子生成器测试场景
- [ ] 实现粒子与外部物体（球面、平面等）的碰撞
- [ ] 增加阻尼力
- [ ] 增加粒子间相互作用（如 n-body、Lennard-Jones 等）
- [ ] 燃烧行星、喷泉、烟花等更多效果
