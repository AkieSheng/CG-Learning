# AS — OpenGL PBR glTF 渲染器

基于 OpenGL 3.3 Shader 的 **glTF 2.0 Metallic-Roughness PBR** 查看器。当前实现包含程序化 IBL、法线贴图、透明/玻璃、清漆、线性 HDR 场景缓冲，以及 ACES 色调映射；目标是接近 Sketchfab 的基础材质表现，但尚未实现真实 HDRI、反射探针或顺序无关透明。

## 项目结构

```
AS/
├── main.C                      # 程序入口
├── Makefile
├── README.md
├── Models/                     # 本地 glTF 资源（已从 Git 排除，需自行准备）
├── LinearAlgebraLibrary/       # 来自 AS0（Vec3f, Matrix）
├── Core/
│   ├── application.h/C         # GLUT 主循环、输入
│   ├── orbit_camera.h/C        # 轨道相机（类 Sketchfab 交互）
│   └── scene.h/C               # 场景管理
├── Renderer/
│   ├── renderer.h/C            # HDR FBO、透明排序、折射与最终合成
│   └── ibl.h/C                 # 程序化 IBL、GGX 预滤波与 BRDF LUT
├── Shader/
│   ├── shader_program.h/C      # 着色器编译/链接
│   ├── pbr.vert / pbr.frag     # Cook-Torrance PBR + IBL
│   ├── skybox.vert / skybox.frag  # IBL 天空盒背景
│   ├── tonemap.vert / tonemap.frag # ACES + 线性到 sRGB
│   └── brdf_lut.vert / brdf_lut.frag  # BRDF 查找表生成
├── GLTF/
│   ├── gltf_loader.h/C         # glTF 加载器（后端：tinygltf）
├── Mesh/
│   ├── vertex.h                # 顶点布局
│   └── mesh.h/C                # VAO/VBO/EBO
├── Material/
│   ├── texture.h/C             # 纹理加载（stb_image）
│   └── pbr_material.h/C        # glTF 材质参数
├── GL/
│   └── gl_headers.h            # GLUT + 轻量 GL 3.3 加载器
└── ThirdParty/
    ├── tinygltf/               # glTF 解析库源码
    ├── tiny_gltf_impl.C        # tinygltf 编译单元
    ├── stb_image.h             # PNG/JPEG 纹理加载
    ├── gl_loader.h/C           # wglGetProcAddress 函数加载
    └── README.md
```

## 架构数据流

```
scene.gltf + scene.bin + textures/
        │
        ▼
   tinygltf::TinyGLTF ──► tinygltf::Model
        │
        ▼
   GltfLoader (转换层)
        │
        ├──► PBRMaterial[] (贴图 + 因子 + 扩展)
        └──► Mesh[] (VAO + 索引 + modelMatrix)
                │
                ▼
         Renderer::render()
                │
                ├── 程序化 IBL（辐照度 + GGX 预滤波 + BRDF LUT）
                ├── Skybox + 不透明物体 → RGBA16F 场景 FBO
                ├── 场景颜色拷贝 → 透明物体屏幕空间折射
                ├── 透明物体按距离从远到近绘制
                ├── ACES 色调映射 + 线性到 sRGB → 默认帧缓冲
                ├── OrbitCamera (view / projection)
                └── Shader/pbr.frag (Cook-Torrance + Split-Sum IBL)
```

## 与 AS0–AS9 的关系

| 模块 | 来源 | 用途 |
|------|------|------|
| `LinearAlgebraLibrary` | AS0 | 向量、矩阵、变换 |
| 相机交互思路 | AS6 `Camera` | 轨道旋转/平移/缩放 |
| 材质概念 | AS6/AS7 `Material` | 漫反射/镜面/折射扩展为 PBR |
| OpenGL 窗口 | AS6 `GLCanvas` | 改为现代 Shader 管线 + freeglut + 自研轻量 GL 3.3 函数加载器 |
| 网格 | AS5/AS8 `Triangle` | 改为 VAO + glTF 属性 |

## 模型资源说明

`AS/Models/` **不纳入 Git**（根目录 `.gitignore` 已忽略）。文件体积过大（大厅玻璃 `scene.bin` 约 157MB），超过 GitHub 100MB 限制。请将 Sketchfab 等来源的 glTF（`scene.gltf` / `scene.bin` / 贴图）放到本地 `AS/Models/<name>/` 后运行。

推荐本地调试模型与特性：

| 模型 | PBR 贴图 | 扩展 / 说明 |
|------|----------|-------------|
| macbook | baseColor, metallicRoughness, normal | — |
| chess_set | baseColor, metallicRoughness | `KHR_materials_clearcoat` |
| low_poly_chess_set | 无（仅因子） | 基础 Metallic-Roughness |
| aviator_sunglasses | — | `KHR_materials_transmission`, BLEND |
| glass_pack / lmu_glass | transmission 贴图 | `KHR_materials_transmission` |
| crystal_stone_rock | + emissive | `alphaMode: BLEND` |
| cosmetic_serum_bottle | baseColor | 产品瓶、因子金属度/粗糙度 |
| ship_in_a_bottle | baseColor, MR, normal | `transmission` + `clearcoat` + BLEND |
| primogem…genshin | 无（仅因子） | Mora 带 emissive |
| bouquet…tropicales | diffuse / specularGlossiness / normal | 加载时自动 SpecGloss→MR 烘焙 |

## 当前实现状态

### 基础 PBR
- [x] Cook-Torrance GGX 镜面 + Lambert 漫反射
- [x] baseColor / metallicRoughness / normal / occlusion / emissive 贴图接口
- [x] 单方向光 + 环境光

### glTF 加载（tinygltf 后端）
- [x] 集成 [tinygltf](https://github.com/syoyo/tinygltf) 替代自写 JSON 解析器
- [x] `GltfLoader` 转换层：`tinygltf::Model` → `Mesh` + `PBRMaterial`
- [x] 场景图遍历、accessor 解码、切线自动生成
- [x] `KHR_materials_transmission` / `ior` / `volume` / `clearcoat` 扩展解析
- [x] `KHR_materials_pbrSpecularGlossiness` → Metallic-Roughness 加载时烘焙
- [x] 节点 `matrix` 与 TRS 变换、逆转置法线矩阵、负行列式绕序处理

### IBL 与输出
- [x] 程序化环境 cubemap（渐变天空 + 增强镜面亮斑）
- [x] Split-Sum 近似（RGB16F 辐照度卷积 + CPU GGX 重要性采样预滤波 + BRDF LUT）
- [x] `skybox.frag` 背景
- [x] 替换平坦 `uAmbientColor` 为漫反射/镜面 IBL
- [x] 镜面偏置能量配比（抬 specularEnv、压 diffuse/hemi），改善金属/木材光泽
- [x] 固定工作室主光（模型上方、初始相机侧偏左）+ 背光轮廓；环境漫反射底保留
- [x] RGBA16F 线性 HDR 场景缓冲
- [x] 最终 ACES 色调映射与线性到 sRGB 编码
- [x] 场景 FBO 8x MSAA（失败时自动降到 4x/2x/单采样）
- [x] 1.0× / 1.5× / 2.0× 内部超采样可选（默认 1.5×，`S` 切换）；FXAA 默认关（`F` 切换）
- [x] 材质贴图各向异性过滤（上限 8x）
- [ ] 从文件加载真实 HDRI / EXR 环境
- [ ] 反射探针、SSR 或平面反射

### 玻璃 / 透明
- [x] `KHR_materials_transmission` 基础实现（环境反射 + 屏幕空间场景折射）
- [x] 透射仅替换漫反射能量并保留镜面；染色玻璃按底色恢复 alpha 与吸收，避免镜片消失或掠射发白
- [x] 透明物体两遍渲染（不透明 → 透明，按相机距离从远到近排序）
- [x] 双面/透明背面法线按 `gl_FrontFacing` 翻转
- [ ] OIT（顺序无关透明）；当前按 Mesh 中心排序，交叠表面仍可能错误
- [ ] 真正的体积多次折射与场景几何射线追踪

### 清漆层
- [x] `KHR_materials_clearcoat` 双层 BRDF 基础实现（chess_set）
- [x] clearcoat 强度/粗糙度/法线贴图

## 编译与运行

**当前构建环境**：Windows、MinGW-w64 g++（C++11）、freeglut、OpenGL 3.3+ 驱动，以及 `ThirdParty/tinygltf`（缺失时见文末 clone 命令）。OpenGL 3.3 函数由 `ThirdParty/gl_loader` 通过 WGL 加载。场景可按窗口分辨率的 1.0× / 1.5× / 2.0× 绘制（默认 1.5×，`S` 循环切换），再线性降采样到窗口；FXAA 默认关闭，按 `F` 开关。

Makefile 使用 Windows 库（`opengl32`、`glu32`）及 `del` 清理命令，不能直接用于 Linux/macOS。在 `AS/` 目录下执行：

```bash
cd AS

# 编译
mingw32-make

# 默认模型（MacBook）
mingw32-make run
# 等价于：
./pbr_viewer -model Models/macbook_air_notebook_pbr/scene.gltf
```

Windows PowerShell 运行示例：`.\pbr_viewer.exe -model Models/chess_set/scene.gltf`

### 全部模型调试命令

| 模型 | 用途 | 命令 |
|------|------|------|
| MacBook | 默认 PBR + 法线贴图 | `./pbr_viewer -model Models/macbook_air_notebook_pbr/scene.gltf` |
| 国际象棋 | 清漆 `clearcoat` | `./pbr_viewer -model Models/chess_set/scene.gltf` |
| 低模象棋 | 无贴图、仅 MR 因子 | `./pbr_viewer -model Models/low_poly_chess_set/scene.gltf` |
| 飞行员墨镜 | 玻璃 `transmission` + BLEND | `./pbr_viewer -model Models/aviator_sunglasses/scene.gltf` |
| 玻璃杯瓶套装 | transmission 贴图 | `./pbr_viewer -model Models/the_ultimate_glass_pack_cups_and_bottles/scene.gltf` |
| 大厅玻璃天花板 | 大场景玻璃（~157MB bin） | `./pbr_viewer -model Models/lmu_main_hall_ceiling_glass_pbr_texture/scene.gltf` |
| 水晶石 | emissive + BLEND | `./pbr_viewer -model Models/crystal_stone_rock/scene.gltf` |
| 精华液瓶 | 产品级 baseColor + 粗糙玻璃感 | `./pbr_viewer -model Models/cosmetic_serum_bottle/scene.gltf` |
| 瓶中船 | transmission + clearcoat 综合验证 | `./pbr_viewer -model Models/ship_in_a_bottle/scene.gltf` |
| 原石/摩拉/星尘 | 因子金属 + emissive | `./pbr_viewer -model Models/primogemmorastardust_from_genshin_impact_free/scene.gltf` |
| 热带花束 | SpecGloss→MR 自动烘焙 | `./pbr_viewer -model Models/bouquet_de_fleurs_tropicales__new_version_pbr/scene.gltf` |

```bash
./pbr_viewer -model Models/macbook_air_notebook_pbr/scene.gltf
./pbr_viewer -model Models/chess_set/scene.gltf
./pbr_viewer -model Models/low_poly_chess_set/scene.gltf
./pbr_viewer -model Models/aviator_sunglasses/scene.gltf
./pbr_viewer -model Models/the_ultimate_glass_pack_cups_and_bottles/scene.gltf
./pbr_viewer -model Models/lmu_main_hall_ceiling_glass_pbr_texture/scene.gltf
./pbr_viewer -model Models/crystal_stone_rock/scene.gltf
./pbr_viewer -model Models/cosmetic_serum_bottle/scene.gltf
./pbr_viewer -model Models/ship_in_a_bottle/scene.gltf
./pbr_viewer -model Models/primogemmorastardust_from_genshin_impact_free/scene.gltf
./pbr_viewer -model Models/bouquet_de_fleurs_tropicales__new_version_pbr/scene.gltf
```

**操作**：左键旋转 · 右键/中键平移 · 滚轮或 `+`/`-` 缩放 · `S` 切换 1.0×/1.5×/2.0× 超采样 · `F` 开关 FXAA · `Q`/`Esc` 退出

控制台会输出加载信息，例如：`GltfLoader (tinygltf): N mesh(es), M material(s)` 与 `Scene: bounds [...] camera dist=...`，可用于排查相机/包围盒问题。

## GltfLoader 架构（tinygltf）

`GltfLoader` 是薄转换层，公开接口不变：

```cpp
bool load(const std::string& gltfPath);
const std::vector<Mesh*>& getMeshes() const;
void getBounds(Vec3f& bmin, Vec3f& bmax) const;
```

内部流程：
1. `tinygltf::TinyGLTF::LoadASCIIFromFile()` 解析 glTF
2. 递归遍历 `nodes`，累积 `Matrix` 世界变换
3. 每个 primitive → `Mesh::upload()` + `PBRMaterial`
4. 纹理仍由 `Texture::loadFromFile()` 上传（sRGB/线性空间分离）

当前加载器针对本地静态 ASCII `.gltf` 路径实现，支持外部 `.bin` 和 PNG/JPEG URI。顶点属性要求 `FLOAT`，索引支持 `UNSIGNED_BYTE`、`UNSIGNED_SHORT`、`UNSIGNED_INT`，图元按 `TRIANGLES` 绘制。尚未支持 `.glb`、data URI/内嵌图像、skin、animation、morph target、多套材质 UV 选择等通用 glTF 功能。`KHR_materials_pbrSpecularGlossiness` 在加载时按 Khronos 启发式烘焙为 Metallic-Roughness 因子/贴图（线性空间转换后写入 sRGB baseColor + 线性 MR）。

若 `ThirdParty/tinygltf` 缺失，执行：
```bash
git clone --depth 1 https://github.com/syoyo/tinygltf.git ThirdParty/tinygltf
```

## PBR Shader 说明

`Shader/pbr.frag` 实现 glTF 标准的 **Metallic-Roughness** 工作流：
- `F0 = mix(0.04, albedo, metallic)`
- GGX 法线分布 + Smith 几何项 + Schlick Fresnel
- Split-Sum IBL（辐照度 cubemap + 预滤波 cubemap + BRDF LUT）
- `KHR_materials_transmission` / `clearcoat` 基础支持
- AO 只作用于环境光；clearcoat 会扣除底层 Fresnel 能量
- 输出先写入线性 HDR FBO，再由 `tonemap.frag` 完成 ACES 与 sRGB 编码

注：当前 IBL 来自程序化工作室环境（压低漫反射底、主光+背光镜面峰 + RGB16F 辐照度），非真实 HDRI；主光固定在模型上方、初始相机侧偏左。金属和玻璃只能反射该环境及屏幕空间折射采样到的背景，不能完整反射场景中的其他模型。

## 许可证

若本地模型附带 `Models/*/license.txt`，请按其中 Sketchfab / CC 条款保留署名。
