6.837 Assignment 6: Grid Acceleration & Solid Textures
======================================================

阶段一 — RayTracingStats + RayCast（无 grid 加速）
--------------------------------------------------

实现：
- 命令行参数 `-stats`；渲染前调用 `Initialize`，结束后调用 `PrintStatistics`
- `RayTracer::rayCast()` 封装暴力遍历 `group->intersect`
- 计数项：
  - non-shadow rays：主射线 `traceRay`，以及反射/折射 secondary rays
  - shadow rays：`getShadowAttenuation`
  - 图元 `intersect`：仅 Sphere / Triangle / Plane
  - grid cells：`MarchingInfo::nextCell`

验证结果（200×200，无 grid，bounces=0）：

| 场景 | Shadows | non-shadow rays | shadow rays | intersections/ray |
|------|---------|-----------------|-------------|-------------------|
| scene6_01_sphere | off | 40000 | 0 | 1.0 |
| scene6_02_sphere_triangles | off | 40000 | 0 | 3.0 |
| scene6_02_sphere_triangles | on | 40000 | 16250 | 2.1 |

阶段二 — Grid 加速（rayCastFast）
---------------------------------

实现：
- `rayCastFast()` / `rayCastShadow()`：DDA marching、逐 cell 求交、
  cell 边界拒绝、early exit
- `Grid::infiniteObjects` 处理 Plane；`GridTransform` 包装展平后的 transform
- 阴影射线也作了 grid 加速

实验验证：
- scene6_01 grid 开/关：图像一致；intersections/ray 约 1.0 → 0.1（含 marking）
- bunny_200：grid 10×10×7 下 intersections/ray 约 201 → 6.1（含 marking）
- scene6_03 plane + sphere：有无 grid 图像均一致

阶段三 — Procedural solid textures
----------------------------------

实现：
- `Material` accessor 接受世界空间 `point`（diffuse、specular、reflective、
  transparent、IOR、exponent）
- `Checkerboard`：3D 单位棋盘格，`odd(floor(x)) ^ odd(floor(y)) ^ odd(floor(z))`
- `Noise`：`N = noise(p) + noise(2p)/2 + ...`（`fractalNoise`），混合权重用 `N+0.5`
- `Marble`：`sin(freq*x + amp*N)`；`Wood`：`sin(freq*radius + amp*N)`
- 修复BUG：`computeLocalShading` 传给 `Shade` Hit
  `intersectionPoint`
- ambient / reflection / refraction / transparent shadows 均使用空间变化属性

验证结果：

| 场景 | 尺寸 | 说明 |
|------|------|------|
| scene6_13_checkerboard | 200² | sphere + checkerboard plane，shadows |
| scene6_14_glass_sphere | 200² | recursive checkerboard + glass，bounces=5 |
| scene6_15_marble_cubes | 300² | 4 个 cube 使用 Noise + Marble |
| scene6_16_wood_cubes | 300² | 4 个 cube 使用 Wood 环纹 |
| scene6_17_marble_vase | 300² | Marble vase + Noise 底座，grid + shadows |

Plus — Grid intersection marking
----------------------------------

栅格化到多个 cell 的图元，同一条 ray 可能重复求交。Marking 在同一次
ray cast 内对每个 object 只求交一次并缓存结果。

实现：
- 首次求交使用无穷远 / `tmax` 上界缓存真实交点，避免被当前 `bestHit` 误拒后跳过
- 后续 cell 复用缓存的 t，仅做 `hitInCell` 与最近交点比较

验证（200×200，启用 grid）：

| 场景 | Grid | intersections/ray（marking 前） | intersections/ray（marking 后） |
|------|------|--------------------------------|-------------------------------|
| scene6_01_sphere | 10³ | 0.3 | 0.1 |
| scene6_04_bunny_200 | 10×10×7 | 9.8 | 6.1 |

图像与 marking 前输出一致。

性能分析摘要
------------

- 简单场景（单球）：grid 可能因 marching 开销略慢，但 intersections/ray 下降。
- 复杂网格（bunny 1k–40k）：grid 显著降低求交次数与总时间；分辨率过细会增加
  cells traversed 与内存，过粗则 cell 内物体过多。
- 阴影与递归射线会放大加速收益

完成：stats、grid 加速、solid textures、marking、变换三角形的 special case。
