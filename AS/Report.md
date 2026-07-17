# Assignment AS: OpenGL PBR glTF 渲染器

作者：王洵

## 1 实验原理

### 1.1 金属度–粗糙度工作流

实验实现基于 OpenGL 的实时 PBR（Physically Based Rendering）查看器，按 glTF 2.0 的金属度–粗糙度（Metallic-Roughness）工作流描述表面光学属性。片元颜色由底色（albedo）$\mathbf{c}$、金属度 $m\in[0,1]$、粗糙度 $\rho\in[0,1]$ 及可选法线、遮蔽、自发光等贴图共同决定。

正入射反射率 $\mathbf{F}_0$ 在电介质与金属之间线性插值：

$$
\mathbf{F}_0=\mathrm{mix}\bigl((0.04,0.04,0.04),\,\mathbf{c},\,m\bigr).
$$

其中 $0.04$ 对应折射率约 $1.5$ 的常见塑料在正入射处的反射率；金属度升高时，$\mathbf{F}_0$ 逐渐取底色，漫反射能量相应减小。粗糙度控制镜面微表面的展宽：$\rho$ 越小，高光越集中。

出射辐射亮度由直接光照与基于图像的照明（Image-Based Lighting，IBL）叠加，并在线性 HDR 缓冲中累积，最后经色调映射与线性到 sRGB 变换写入默认帧缓冲。

### 1.2 Cook–Torrance 镜面 BRDF

镜面项采用 Cook–Torrance 微表面模型：

$$
f_s(\mathbf{l},\mathbf{v})=\frac{D(\mathbf{h})\,G(\mathbf{l},\mathbf{v})\,F(\mathbf{v},\mathbf{h})}{4\,(\mathbf{n}\!\cdot\!\mathbf{l})\,(\mathbf{n}\!\cdot\!\mathbf{v})},
$$

其中 $\mathbf{n}$ 为表面法线，$\mathbf{l}$ 为入射方向，$\mathbf{v}$ 为观察方向，$\mathbf{h}=\mathrm{normalize}(\mathbf{l}+\mathbf{v})$ 为半角向量。

法线分布函数取 GGX（Trowbridge–Reitz）：

$$
D(\mathbf{h})=\frac{\alpha^2}{\pi\bigl((\mathbf{n}\!\cdot\!\mathbf{h})^2(\alpha^2-1)+1\bigr)^2},\qquad \alpha=\rho^2.
$$

几何遮蔽–阴影项取 Smith 形式，单侧用 Schlick–GGX 近似：

$$
k=\frac{(\rho+1)^2}{8},\qquad
G_1(\mathbf{n},\mathbf{x})=\frac{\mathbf{n}\!\cdot\!\mathbf{x}}{(\mathbf{n}\!\cdot\!\mathbf{x})(1-k)+k},\qquad
G=G_1(\mathbf{n},\mathbf{v})\,G_1(\mathbf{n},\mathbf{l}).
$$

Fresnel 反射率用 Schlick 近似：

$$
F(\mathbf{v},\mathbf{h})=\mathbf{F}_0+(1-\mathbf{F}_0)\,(1-\mathbf{v}\!\cdot\!\mathbf{h})^5.
$$

漫反射采用 Lambert 形式。镜面与漫反射能量由 Fresnel 划分：令 $\mathbf{k}_S=F$，则

$$
\mathbf{k}_D=(1-\mathbf{k}_S)\,(1-m),\qquad
f_d=\mathbf{k}_D\cdot\frac{\mathbf{c}}{\pi}.
$$

金属度 $m$ 增大时漫反射权重下降，能量更多集中在镜面通道。

### 1.3 Split-Sum 图像照明

实时环境下用 Karis 的 Split-Sum 近似将镜面环境积分拆成两项：预滤波环境立方体贴图与二维 BRDF 查找表。镜面环境贡献写为

$$
L_{\mathrm{spec}}\approx L_{\mathrm{prefilter}}(\mathbf{R},\rho)\cdot\bigl(F(\mathbf{n}\!\cdot\!\mathbf{v})\,A(\mathbf{n}\!\cdot\!\mathbf{v},\rho)+B(\mathbf{n}\!\cdot\!\mathbf{v},\rho)\bigr),
$$

其中 $\mathbf{R}=\mathrm{reflect}(-\mathbf{v},\mathbf{n})$ 为反射方向；$L_{\mathrm{prefilter}}$ 按粗糙度选取 mip 层级；$A,B$ 由离线对 GGX 与 Schlick Fresnel 做重要性采样积分得到，并烘焙进查找表。

漫反射环境用辐照度立方体贴图逼近半球积分

$$
E(\mathbf{n})=\frac{1}{\pi}\int_{\Omega} L_i(\boldsymbol{\omega})\,\max(\mathbf{n}\!\cdot\!\boldsymbol{\omega},0)\,\mathrm{d}\boldsymbol{\omega},
$$

再乘以 $\mathbf{c}\cdot\mathbf{k}_D$。本实验的入射场 $L_i$ 由程序化工作室环境生成，使用了渐变天空底色并叠加一定的主光与背光，再分别卷积得到辐照度与预滤波贴图。

### 1.4 透射与屏幕空间折射

对透明 / 玻璃材质，透射因子 $\tau\in[0,1]$ 用折射路径替换部分漫反射能量，保留镜面高光。由 Snell 定律得出折射方向。设相对折射率 $\eta=\eta_i/\eta_t$，入射方向 $\mathbf{i}$，法线 $\mathbf{n}$，则

$$
k=1-\eta^2\bigl(1-(\mathbf{n}\!\cdot\!\mathbf{i})^2\bigr).
$$

当 $k<0$ 时发生全内反射；否则透射方向为

$$
\mathbf{t}=\eta\,\mathbf{i}-\mathbf{n}\bigl(\eta\,(\mathbf{n}\!\cdot\!\mathbf{i})+\sqrt{k}\bigr).
$$

电介质正入射反射率由折射率 $\eta_t$ 得到：

$$
F_0=\Bigl(\frac{\eta_t-1}{\eta_t+1}\Bigr)^2.
$$

玻璃颜色按 Fresnel 混合反射与折射：

$$
\mathbf{c}_{\mathrm{glass}}=F\cdot\mathbf{L}_{\mathrm{refl}}+(1-F)\cdot\mathbf{L}_{\mathrm{refr}}.
$$

折射采样在已绘制的不透明场景颜色缓冲上沿 $\mathbf{t}$ 做短程投影（屏幕空间折射）；越界时退回环境立方体贴图。体积吸收采用 Beer–Lambert 定律：

$$
\mathbf{L}'=\mathbf{L}\,\exp(-\boldsymbol{\sigma}\,t),\qquad
\boldsymbol{\sigma}=-\frac{\ln\mathbf{c}_{\mathrm{att}}}{d},
$$

其中 $t$ 为厚度，$\mathbf{c}_{\mathrm{att}}$ 为衰减色，$d$ 为衰减距离。

### 1.5 清漆双层 BRDF

清漆扩展在底层 BRDF 之上叠加一层低粗糙度电介质镜面。清漆层取 $\mathbf{F}_0^{\mathrm{cc}}=(0.04,0.04,0.04)$，强度为 $f_c$。底层颜色先按清漆 Fresnel 衰减：

$$
\mathbf{C}\leftarrow\mathbf{C}\bigl(1-f_c\,F_{\mathrm{cc}}\bigr),
$$

再累加清漆直接光与 IBL 镜面贡献，形成漆面盖住基底的外观。

### 1.6 HDR 色调映射

场景缓冲使用线性 HDR 颜色。最终输出采用 Narkowicz（2015）ACES 近似：

$$
\mathbf{y}=\mathrm{clamp}\!\left(\frac{\mathbf{x}(a\mathbf{x}+b)}{\mathbf{x}(c\mathbf{x}+d)+e},\,0,\,1\right),
$$

其中 $a=2.51$，$b=0.03$，$c=2.43$，$d=0.59$，$e=0.14$，输入 $\mathbf{x}$ 为曝光缩放后的线性色。再按 IEC 61966-2-1 将线性值变换到 sRGB，写入帧缓冲。

## 2 程序设计与实现

### 2.1 总体架构

系统由应用主循环、场景管理、glTF 加载、网格与材质、渲染器、图像照明预计算与着色器管线组成。向量与矩阵复用前期实验的线性代数库；窗口与输入基于 freeglut，OpenGL 3.3 函数由轻量加载器经 WGL 取得。

数据流概览：

1. 解析 ASCII 格式的 glTF 及外部二进制缓冲、贴图；
2. 遍历场景图，累积世界变换，将每个图元转为可绘制网格与 PBR 材质；
3. 根据包围盒摆放轨道相机；
4. 每帧依次完成方向光阴影深度、天空盒与不透明物体写入 HDR 多重采样缓冲、将场景颜色解析到单采样纹理供折射、透明物体由远及近绘制，以及 ACES / sRGB（可选 FXAA）输出到屏幕。

材质参数结构（因子、贴图与扩展字段）如下：

```cpp
struct PBRMaterial {
  float baseColorFactor[4];
  float metallicFactor;
  float roughnessFactor;
  float emissiveFactor[3];
  Texture *baseColorTexture;
  Texture *metallicRoughnessTexture;
  Texture *normalTexture;
  // 透射、体积吸收、清漆等扩展字段……
  bool hasTransmission;
  float transmissionFactor;
  bool hasClearcoat;
  float clearcoatFactor;
  AlphaMode alphaMode;
};
```

### 2.2 glTF 加载与材质转换

加载器调用第三方解析库读入模型，再递归遍历节点。局部变换取自 $4\times 4$ 矩阵，或由平移、旋转四元数、缩放相乘得到；世界矩阵为父节点与局部变换之积。每个三角形图元上传顶点缓冲并绑定对应材质。

对镜面–光泽度（Specular-Glossiness）扩展，在加载时按 Khronos 启发式烘焙为金属度–粗糙度。由镜面亮度解二次方程求金属度，再混合漫反射与镜面通道得到底色，粗糙度为 $1-\mathrm{glossiness}$：

```cpp
float metallic = 0.0f;
if (specularBrightness >= dielectricSpecular) {
  float a = dielectricSpecular;
  float b = diffuseBrightness * oneMinusSpecularStrength / (1.0f - dielectricSpecular)
            + specularBrightness - 2.0f * dielectricSpecular;
  float c = dielectricSpecular - specularBrightness;
  float D = std::max(b * b - 4.0f * a * c, 0.0f);
  metallic = clamp01((-b + sqrtf(D)) / (2.0f * a));
}
outRoughness = clamp01(1.0f - glossiness);
```

同时解析透射、折射率、体积吸收与清漆等扩展，写入材质字段供着色器使用。法线矩阵在绘制时由模型矩阵求逆转置得到；模型行列式为负时翻转正面绕序，使变换后的三角片绕序正确。

### 2.3 顶点布局与网格

顶点包含位置、法线、切线（含副切线手性）与第一组纹理坐标，与着色器属性槽一一对应：

```cpp
struct Vertex {
  Vec3f position;
  Vec3f normal;
  Vec4f tangent;   // xyz = 切线, w = 副切线手性
  Vec2f texCoord0;
};
```

缺失切线时，由位置与 UV 按三角形边向量生成，供法线贴图构造 TBN 矩阵。网格以顶点数组对象、顶点缓冲与索引缓冲上传，按三角形索引绘制。

### 2.4 轨道相机

相机绕观察目标点运动。由偏航角、俯仰角与距离得到眼点位置：

$$
\mathbf{e}=\mathbf{t}+d\bigl(\cos\phi\sin\theta,\,\sin\phi,\,\cos\phi\cos\theta\bigr),
$$

其中 $\mathbf{t}$ 为目标点，$d$ 为距离，$\theta$、$\phi$ 为偏航与俯仰（弧度）。观察矩阵由前向、右向、上向基构成；投影为垂直视场角 $45^\circ$ 的透视投影。加载后按包围盒半径 $R$ 与半视场角估计距离 $d\approx 1.8\,R/\tan(\mathrm{fov}/2)$，使模型落入视野。

```cpp
void OrbitCamera::updatePosition() {
  float yawRad = yaw * (float)M_PI / 180.0f;
  float pitchRad = pitch * (float)M_PI / 180.0f;
  float x = distance * cosf(pitchRad) * sinf(yawRad);
  float y = distance * sinf(pitchRad);
  float z = distance * cosf(pitchRad) * cosf(yawRad);
  position = target + Vec3f(x, y, z);
}
```

同时实现左键旋转，右键或中键平移，滚轮或加减键缩放的交互。

### 2.5 PBR 片元着色

片元着色器实现 GGX / Smith / Schlick 与直接光合成。镜面 BRDF 求值：

```glsl
vec3 evaluateSpecularBRDF(vec3 N, vec3 V, vec3 L, vec3 H, vec3 F0, float roughness) {
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    return (NDF * G * F) / denom;
}
```

主路径中构造 $\mathbf{F}_0$、划分 $\mathbf{k}_D$，累加方向光、工作室灯带采样、漫反射 / 镜面 IBL，并用遮蔽贴图调制环境项：

```glsl
vec3 F0 = mix(vec3(0.04), albedo, metallic);
vec3 kD = (vec3(1.0) - fresnelSchlick(max(dot(H, V), 0.0), F0)) * (1.0 - metallic);
vec3 diffuseDirect = kD * albedo / PI * uLightColor * NdotL * uDirectLightScale;
vec3 specularDirect = evaluateSpecularBRDF(N, V, L, H, F0, roughness)
                    * uLightColor * NdotL * uDirectLightScale;
diffuseIBL = texture(uIrradianceMap, N).rgb * albedo * kD_ibl * uDiffuseEnvScale;
specularIBL = sampleSpecularIBL(N, V, roughness, F0, NdotV) * uSpecularEnvScale;
```

Split-Sum 镜面采样：

```glsl
vec3 sampleSpecularIBL(vec3 N, vec3 V, float roughness, vec3 F0, float NdotV) {
    vec3 R = reflect(-V, N);
    vec3 prefiltered = textureLod(uPrefilterMap, R, roughness * uMaxReflectionLOD).rgb;
    vec2 brdf = texture(uBrdfLUT, vec2(NdotV, roughness)).rg;
    vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    return prefiltered * (F * brdf.x + brdf.y);
}
```

透射路径用 Snell 折射与屏幕空间采样得到折射色，再与环境反射按 Fresnel 混合，并按透射因子替换漫反射部分；清漆路径在最终颜色上做底层衰减后叠加清漆镜面。双面或透明关闭背面剔除时，背面片元翻转法线。输出在 HDR 路径下保持线性，色调映射留到后处理阶段。

### 2.6 图像照明预计算

初始化时生成程序化照明立方体贴图：漫反射底色之上叠加主光与背光功率峰，使金属与漆面能反射到清晰高光：

```cpp
float keyDot = dx * kx + dy * ky + dz * kz;
if (keyDot > 0.0f) {
  float keySharp = powf(keyDot, 140.0f) * 36.0f;
  *r += keySharp; *g += keySharp; *b += keySharp * 1.05f;
}
```

随后对半球做余弦加权卷积得到辐照度立方体贴图；对 GGX 做重要性采样，按粗糙度写入预滤波立方体贴图各 mip；再在 GPU 上对 $(\mathbf{n}\!\cdot\!\mathbf{v},\rho)$ 积分生成 BRDF 查找表：

```glsl
vec2 IntegrateBRDF(float NdotV, float roughness) {
    // Hammersley + GGX 重要性采样，累计 A、B
    A += (1.0 - Fc) * G_Vis;
    B += Fc * G_Vis;
    return vec2(A, B) / float(SAMPLE_COUNT);
}
```

天空盒使用略压暗的显示用立方体贴图，与照明立方体贴图分离，防止背景过度曝光。

### 2.7 帧渲染管线

每帧将网格分为不透明与透明两类。透明判定依据为 Alpha 混合模式或非零透射因子；透明列表按网格世界中心到相机距离由远到近排序。主流程：

```cpp
void Renderer::render(Scene &scene) {
  // 分类 opaque / transparent，透明按距离远→近排序
  renderShadowMap(opaque);
  // 绑定 MSAA 或单采样 HDR FBO，清屏
  ibl.renderSkybox(...);
  drawMeshes(scene, false, &opaque, NULL);
  if (!transparent.empty()) {
    resolveMsaaToSceneColor();
    captureSceneColorSample();   // 供屏幕空间折射
    drawMeshes(scene, true, NULL, &transparent);
  }
  resolveMsaaToSceneColor();
  blitTonemapToScreen();
}
```

方向光阴影以包围场景球的正交投影写入深度图，主通道用斜率相关偏移与 $3\times 3$ PCF 采样。场景缓冲目标为 $8\times$ 多重采样的 RGBA16F；若该配置无法完成帧缓冲创建，则依次尝试 $4\times$、$2\times$ 或单采样。内部绘制分辨率为窗口的 $1.0\times$、$1.5\times$ 或 $2.0\times$（默认 $1.5\times$，按 `S` 切换），最终线性降采样到窗口。

### 2.8 色调映射与抗锯齿

全屏通道读取 HDR 场景颜色，做 ACES 与线性到 sRGB：

```glsl
vec3 toneMapACES(vec3 x) {
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}
vec3 sampleTonemapped(vec2 uv) {
    vec3 hdr = texture(uHdrColor, uv).rgb;
    return linearToSRGB(toneMapACES(hdr * EXPOSURE));
}
```

另外，实现了 FXAA 开关，在 LDR sRGB 空间做边缘亮度检测与亚像素混合。材质贴图启用各向异性过滤（上限约 $8\times$）。

## 3 实验结果

运行示例：

```text
mingw32-make
./pbr_viewer -model Models/macbook_air_notebook_pbr/scene.gltf
./pbr_viewer -model Models/chess_set/scene.gltf
./pbr_viewer -model Models/aviator_sunglasses/scene.gltf
```

操作：左键旋转，右键 / 中键平移，滚轮缩放；`S` 切换内部超采样倍率；`F` 开关 FXAA。下列截图均在程序化工作室 IBL、方向光阴影与默认 $1.5\times$ 内部分辨率下拍摄。

### 3.1 基础金属度–粗糙度与法线贴图

| 笔记本电脑 | 低模象棋 |
|:---:|:---:|
| ![notebook](OutputFiles/notebook.png) | ![chess_low](OutputFiles/chess_low.png) |
| 底色 / 金属粗糙度 / 法线贴图 | 该模型无贴图，突出金属度与粗糙度因子 |

笔记本电脑机身金属高光随视角变化，法线贴图带来细部凹凸；低模象棋仅靠常数因子区分木质与棋子材质，仍呈现合理的漫反射 / 镜面分离，说明工作流在无贴图时也可成立。

### 3.2 清漆层

| 国际象棋（清漆） |
|:---:|
| ![chess](OutputFiles/chess.png) |
| 棋盘与棋子表面叠加清漆镜面 |

棋盘表面可见一层光滑漆面高光，底层木纹与金属棋子显示清晰。清漆 Fresnel 对底层做能量扣除后，掠射角处漆面反射更强，与双层 BRDF 预期相符。

### 3.3 透射与玻璃

| 飞行员墨镜 | 杯瓶套装 | 大厅玻璃天花 |
|:---:|:---:|:---:|
| ![sunglasses](OutputFiles/sunglasses.png) | ![bottle_cups](OutputFiles/bottle_cups.png) | ![ceiling](OutputFiles/ceiling.png) |
| 透射 + Alpha 混合 | 透射贴图调制 | 大场景玻璃 |

墨镜镜片透过屏幕空间折射看到背景，同时保留边缘 Fresnel 反射；杯瓶与天花在透射贴图或大面积玻璃上呈现折射扭曲与环境反射。染色区域按底色吸收，正面镜片保持一定不透明度，掠射处不致整片发白。

### 3.4 透射与清漆综合

| 瓶中船 | 液瓶 |
|:---:|:---:|
| ![ship_in_bottle](OutputFiles/ship_in_bottle.png) | ![posion](OutputFiles/posion.png) |
| 玻璃透射 + 清漆效果 | 瓶身玻璃感 |

瓶中船同时启用透射与清漆，瓶壁折射内部船体，外表面另有一层清漆高光。液瓶以底色贴图为主，由透射路径与金属度–粗糙度因子共同作用，瓶身呈现部分高光。

### 3.5 自发光、混合模式与材质烘焙

| 水晶石 | 道具类物品 | 热带花束 |
|:---:|:---:|:---:|
| ![stone](OutputFiles/stone.png) | ![genshen](OutputFiles/genshen.png) | ![flower](OutputFiles/flower.png) |
| 自发光 + Alpha 混合 | 因子金属 + 自发光 | 镜面–光泽度烘焙为 MR |

水晶石半透明区域与自发光叠加后展现出良好外观；摩拉等物体在无贴图时靠金属度因子与自发光区分材质。花束由镜面–光泽度扩展在加载时烘焙为金属度–粗糙度，花瓣与叶片的漫反射 / 阴影分布与预期相同。

## 4 问题记录

画面只在窗口左上角约 $512\times512$ 区域有效，其余为清屏色，滚轮缩放几乎无效。原有是生成 BRDF 查找表时把视口改成查找表分辨率且未恢复，后续绘制沿用了错误的视口。解决方法：在生成查找表时保存并恢复视口，在初始化与每帧入口显式设置全窗口视口，并补上滚轮事件驱动相机距离。

加载后模型常不在视野内。发现观察矩阵平移行列误写，且投影深度项符号与索引错误。修正观察与投影系数后，自动取景与旋转、平移、缩放恢复正常。

部分贴图上下颠倒，负缩放节点出现镜像或半黑半白错乱。原因是纹理加载做了垂直翻转，与 glTF 的 OpenGL 式 UV 冲突，且世界矩阵行列式为负时仍按逆时针剔除背面。改为不翻转加载，并在行列式为负时改用顺时针正面绕序后问题解决。

加载部分杯瓶贴图时进程崩溃。原因是双通道灰度–Alpha 贴图被当成三通道彩色上传，像素布局与 GPU 期望不符。将其解码为四通道再上传后，图像正常加载。

象棋与瓶盖等出现局部过暗或近黑。原因是未写出的金属度默认为 $1.0$，无金属粗糙度贴图的深色塑料被当成金属，漫反射被 Fresnel 划分殆尽，再叠加过强直接光与偏弱环境。降低直接光、提高辐照度与半球填充，并按包围盒加工作室灯带补光后，问题解决。

为接近工作室观感曾把漫反射环境压得过暗，物体整体发黑、只剩局部高光。原因是把背景柔和与镜面窄高光混在同一套环境能量里调节。解决方法：将程序化环境拆成平滑漫反射卷积，压暗天空显示与镜面照明，然后分别绑定，最后实现背景暗灰而物体反射高光清晰。

交叠玻璃混合顺序错误，折射背景也不正确。原因是透明与不透明未分流排序，且缺少可供屏幕空间折射采样的不透明场景颜色。解决方法：分为不透明与透明两类并由远及近绘制，经过不透明解析，然后沿 Snell 方向采样场景颜色，若多重采样失败，则回退档位，修复完成后混合与折射恢复正常。

墨镜等带轴向转换的模型镜片像画在背面，高光与折射方向出现翻转。原因是模型矩阵按列主序上传，法线矩阵却按行主序打包，旋转节点下法线几乎翻到相机背面。解决：把法线矩阵改为列主序，并对双面或透明的背面片元翻转法线，恢复其朝向。

粗糙金属的环境高光呈盒状模糊。原因是对照明立方体贴图直接生成多级纹理再按粗糙度取样，与 BRDF 查找表的 GGX 假设不一致。解决方法：按 Hammersley 与 GGX 重要性采样写入独立预滤波立方体贴图。

线性色直接写入默认帧缓冲时高光过曝、暗部发灰。原因是未在高动态范围缓冲中累积再做色调映射。解决：先写入半精度浮点场景缓冲，再经 ACES 与线性到 sRGB 全屏输出。

为折射挂深度纹理附件时初始化崩溃。原因是本机器驱动对深度纹理帧缓冲附件支持不完整。解决方法：改为深度渲染缓冲附着。

玻璃只能透出天空、看不到身后物体。原因是透射只采样了环境立方体贴图。解决方案：不透明绘制结束后拷贝场景颜色，再沿折射方向短程投影采样，最后场景折射恢复。

轮廓锯齿、斜视贴图发糊，起初以为是材质公式问题。但后来发现是因为窗口多重采样只作用默认帧缓冲，主场景离屏 HDR 缓冲没有实现真正的多重采样，况且贴图也为实现各向异性过滤。解决方案：在场景缓冲启用多重采样，同时配合内部超采样、近似抗锯齿与各向异性过滤，最终改善了边缘与斜视清晰度。

金属像发灰塑料，旋转时高光几乎不滑动。原因是方向光跟随相机，解析高光总贴在朝向观察者一面，且环境灰底与漫反射填充过强冲淡金属色相。解决方法为固定工作室主光、压低漫反射环境、略抬镜面对比参数并调整金属参数，最后增加了金属质感。

墨镜镜片正面偏亮、侧面却透明。原因是透射先按 $(1-\tau)$ 清零再叠加反射，导致掠射处只剩灰环境，且不透明度处理不当使混合后镜片消失。解决方法：透射只替换漫反射部分，同时加强染色吸收，增加不透明度，最终墨镜感与侧面观感正常。

金属表面出现多处杂乱亮斑。原因是程序化环境叠了主光、填充灯与背光多处镜面峰。去掉填充灯峰，并调整各光源亮度和位置后，观感接近单一工作室主灯。

方向光阴影时通时断。原因是深度纹理挂到仅深度附件的帧缓冲在本机驱动上非法或不完整。解决方法：用半精度颜色纹理写入窗口深度、附加深度渲染缓冲，主通道上做斜率偏移与 $3\times 3$ 的邻近过滤，并让可见性只与直接光相乘，最终实现阴影稳定。

热带花束几何能加载但着色错误。原因是资源使用镜面–光泽度工作流，不含金属度–粗糙度字段，现有片元只跑了后者的处理。解决方法：增加 Khronos 启发式烘焙，将原贴图其转换为底色与金属度–粗糙度贴图，使花束外观恢复正常。

大尺度道具模型整屏不可见，只剩天空盒。多次观察后发现是世界尺度极大，自动取景后相机距离超过固定远裁剪面，几何被深度测试全部丢弃。解决方法：常规距离保留原算法，当距离明显超出时，按比例放大裁剪面。

## 5 总结

本实验在 OpenGL 3.3 着色器管线上实现了 glTF 金属度–粗糙度 PBR 查看器：以 Cook–Torrance（GGX / Smith / Schlick）计算直接光与镜面，用 Split-Sum IBL 提供漫反射辐照度与镜面环境；透射路径用 Snell 折射与屏幕空间场景采样替换漫反射能量，清漆路径用双层 BRDF 叠加电介质镜面；线性 HDR 场景缓冲经 ACES 与 sRGB 输出。加载阶段完成场景图变换、贴图空间分离，以及镜面–光泽度到金属度–粗糙度的烘焙。多类模型的正常解析和渲染表明基础贴图、清漆、玻璃透射与自发光等路径能够协同工作，视觉表现和物理模型能与预期相符。

## 附录（TODO list 记录）

- [x] 在片元着色器中实现 Cook–Torrance GGX 镜面与 Lambert 漫反射，按金属度划分能量，并支持底色 / 金属粗糙度 / 法线 / 遮蔽 / 自发光贴图
- [x] 用第三方库解析 ASCII glTF，遍历场景图累积世界变换，将图元转为网格与 PBR 材质
- [x] 解析透射、折射率、体积与清漆扩展，并加载时将镜面–光泽度烘焙为金属度–粗糙度
- [x] 生成程序化工作室立方体贴图，卷积辐照度与 GGX 预滤波贴图，并积分 BRDF 查找表，供 Split-Sum IBL 采样
- [x] 在渲染器中实现方向光阴影、MSAA HDR 场景缓冲、透明物体远到近绘制、屏幕空间折射采样，以及 ACES / sRGB 全屏输出
- [x] 实现轨道相机（旋转、平移、缩放）并按包围盒自动取景
- [x] 内部超采样与 FXAA 选择。
- [ ] 增加反射探针、屏幕空间反射或平面反射
- [ ] 实现顺序无关透明，改进交叠玻璃的混合顺序
- [ ] 支持体积多次折射及更完整的场景几何折射
- [ ] 扩展加载器以支持二进制 glTF、内嵌图像、蒙皮与动画等
