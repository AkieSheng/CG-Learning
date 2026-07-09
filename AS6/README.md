6.837 Assignment 6: Grid Acceleration & Solid Textures
======================================================

阶段一 — RayTracingStats + RayCast（无 grid 加速）
--------------------------------------------------

实现：
- 命令行参数 `-stats`；渲染前调用 `Initialize`，结束后调用 `PrintStatistics`
- `RayTracer::rayCast()` 封装暴力遍历 `group->intersect`
- 计数项：non-shadow rays（`traceRay`）、shadow rays（`getShadowAttenuation`）、
  图元 `intersect` 调用（仅 Sphere / Triangle / Plane）

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
  cell 边界拒绝（cell-boundary rejection）、early exit
- `Grid::infiniteObjects` 处理 Plane；`GridTransform` 包装展平后的 transform；
  `MarchingInfo::nextCell()` → `IncrementNumGridCellsTraversed()`

实验验证：
- scene6_01 grid 开/关：图像一致；intersections/ray 1.0 → 0.3
- bunny_200：grid 10×10×7 下 intersections/ray 201.0 → 9.8
- scene6_03 plane + sphere：有无 grid 图像均一致

阶段三 — Procedural solid textures
----------------------------------

实现：
- `Material` accessor 接受世界空间 `point`（diffuse、specular、reflective、
  transparent、IOR、exponent）
- `Checkerboard`：3D 单位棋盘格，`odd(floor(x)) ^ odd(floor(y)) ^ odd(floor(z))`
- `Noise`：多 octave Perlin 混合；`Shade` 对子材质结果插值
- `Marble`：`sin(freq*x + amp*N)` 条纹 + noise 扰动
- `Wood`：texture space 中环形图案 `sin(freq*radius + amp*N)`
- 公共辅助函数见 `Materials/procedural_utils.h`
- `rayTracer` 对 ambient、reflection、refraction、transparent shadows
  使用空间变化的 material 属性

验证结果：

| 场景 | 尺寸 | 说明 |
|------|------|------|
| scene6_13_checkerboard | 200² | sphere + checkerboard plane，shadows |
| scene6_14_glass_sphere | 200² | recursive checkerboard + glass，bounces=5 |
| scene6_15_marble_cubes | 300² | 4 个 cube 使用 Noise + Marble |
| scene6_16_wood_cubes | 300² | 4 个 cube 使用 Wood 环纹 |
| scene6_17_marble_vase | 300² | Marble vase + Noise 底座，grid + shadows |

附加 — Grid intersection marking
----------------------------------

出现的问题：栅格化到多个 grid cell 的图元，同一条 ray 可能在每个 cell 各求交一次。
Marking 在同一次 ray cast 内跳过已测试过的 object。

实现：
- `Object3D::intersectionMark` + 每条 ray 缓存 `markedHit`
- `RayTracer::intersectionMarkCounter`；每次 `rayCastFast` / `rayCastShadow`
  开始时调用 `beginIntersectionMarking()`
- 首次进入 cell：求交一次并缓存；后续 cell 复用缓存的 t 做 `hitInCell`，
  不再调用 `intersect`

验证结果（200×200，启用 grid）：

| 场景 | Grid | intersections/ray（marking 前） | intersections/ray（marking 后） |
|------|------|--------------------------------|-------------------------------|
| scene6_01_sphere | 10³ | 0.3 | 0.1 |
| scene6_04_bunny_200 | 10×10×7 | 9.8 | 6.1 |

图像与 marking 前输出一致。

完成：stats、grid 加速、solid textures、marking 及其测试验证。
未完成： special case for transformations of triangle primitives（找不到合适解决方案）
