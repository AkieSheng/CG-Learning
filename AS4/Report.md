# Assignment 4: Shadows, Reflection & Refraction

作者：王洵

## 1 实验原理

### 1.1 递归光线追踪与辐射亮度

沿视线方向追踪的射线在场景中与表面相交后，该点处的出射辐射亮度（radiance）由局部直接光照与经二次射线（secondary ray）传播的间接贡献共同组成。局部项沿用 Phong 模型：设交点处单位法线为 $\mathbf{N}$，指向第 $i$ 个光源的单位方向为 $\mathbf{L}_i$，视线方向为 $\mathbf{V}$，半角向量

$$
\mathbf{H}_i=\mathrm{normalize}(\mathbf{L}_i+\mathbf{V}),
$$

则漫反射强度为 $\max(\mathbf{N}\cdot\mathbf{L}_i,0)$，镜面高光为 $\max(\mathbf{N}\cdot\mathbf{H}_i,0)^n$。环境光提供与视角无关的基底项。

对具有反射系数 $\mathbf{k}_r$ 或透射系数 $\mathbf{k}_t$ 的材质，还需沿镜面方向或折射方向继续追踪，将远端辐射亮度以 $\mathbf{k}_r\odot\mathbf{c}_{\mathrm{remote}}$ 或 $\mathbf{k}_t\odot\mathbf{c}_{\mathrm{remote}}$ 叠加，其中 $\odot$ 为逐分量相乘。无限次反射与折射在闭合光路下会使能量发散，故以最大反弹次数 $N_{\max}$ 与射线权重阈值 $w_{\min}$ 截断递归：当当前深度 $b>N_{\max}$ 或权重 $w$ 低于阈值时，以背景色终止，使能量贡献随路径深度衰减。

### 1.2 投射阴影与半透明衰减

阴影由光源到着色点的可见性决定。对交点 $\mathbf{p}$ 与指向光源的单位方向 $\mathbf{L}$，自 $\mathbf{p}$ 沿 $\mathbf{L}$ 发射阴影射线（shadow ray），起点取

$$
\mathbf{o}_s=\mathbf{p}+\varepsilon\mathbf{n},\qquad \mathbf{d}_s=\mathbf{L},
$$

其中 $\mathbf{n}$ 为几何法线、$\varepsilon$ 为微小偏移量，用于将起点推出表面，以免浮点误差引起自相交。若在区间 $t\in[\varepsilon,\,t_{\mathrm{light}}-\varepsilon]$ 内存在遮挡物（点光源时 $t_{\mathrm{light}}$ 为到着色点到光源的距离，方向光取无穷大），则该光源的直接贡献为零；否则按 Phong 公式累加。

半透明遮挡物可用 Beer–Lambert 定律描述介质内衰减。透射率

$$
T(d)=\exp(-\boldsymbol{\kappa}\,d),\qquad \boldsymbol{\kappa}=\mathbf{1}-\mathbf{k}_t,
$$

其中 $\boldsymbol{\kappa}$ 为吸收系数，$d$ 为光程长度。沿阴影射线逐段穿过透明体时，将各段透射率逐分量相乘得到总衰减 $\mathbf{a}_{\mathrm{shadow}}$，再以 $\mathbf{c}_{\mathrm{light}}\odot\mathbf{a}_{\mathrm{shadow}}$ 调制该光源颜色。

### 1.3 镜面反射

理想镜面反射遵循入射角等于反射角。设入射方向 $\mathbf{I}$（指向交点）与单位法线 $\mathbf{N}$，反射方向为

$$
\mathbf{R}=\mathbf{I}-2(\mathbf{N}\cdot\mathbf{I})\mathbf{N}.
$$

该式由入射方向在法向分量上的反向叠加得到，使 $\mathbf{R}$ 与 $\mathbf{N}$ 位于同一侧且 $|\mathbf{R}|=|\mathbf{I}|$。反射贡献为 $\mathbf{k}_r\odot\mathbf{c}_{\mathrm{reflect}}$，其中 $\mathbf{c}_{\mathrm{reflect}}$ 为沿 $\mathbf{R}$ 递归求得的辐射亮度，权重按 $\|\mathbf{k}_r\|$ 缩放。

### 1.4 折射与折射率栈

透明介质中的折射由 Snell 定律 $\eta_i\sin\theta_i=\eta_t\sin\theta_t$ 描述，$\eta_i$、$\eta_t$ 分别为入射侧与出射侧折射率（index of refraction）。设入射方向 $\mathbf{I}$、法线 $\mathbf{N}$（指向入射侧），令 $\cos\theta_i=-\mathbf{N}\cdot\mathbf{I}$，$\eta=\eta_i/\eta_t$，则折射方向

$$
\mathbf{T}=\eta\mathbf{I}-\Bigl(\eta\cos\theta_i+\sqrt{1-\eta^2(1-\cos^2\theta_i)}\Bigr)\mathbf{N}.
$$

根号内为负时发生全反射，无透射路径。射线由外入内时 $\mathbf{N}\cdot\mathbf{I}<0$，$\eta_i$ 取当前外围介质折射率（真空为 $1$），$\eta_t$ 取物体折射率；由内出外时法线相对入射方向翻转，$\eta_i$ 为物体内折射率，$\eta_t$ 恢复为进入该层之前的介质折射率。嵌套透明体用栈记录各层外围折射率，使每次穿越界面时 $\eta_i$、$\eta_t$ 与物理情境对应。

透射贡献以 $\mathbf{k}_t\odot\mathbf{c}_{\mathrm{transmit}}$ 累加。当视线方向 $\mathbf{d}$ 与几何法线满足 $\mathbf{d}\cdot\mathbf{N}>0$ 时，射线从背面入射，可将法线取反 $\mathbf{N}\leftarrow-\mathbf{N}$，使内侧表面获得正确照明。

### 1.5 点光源距离衰减

点光源在距离 $d$ 处的照度随几何扩散衰减：

$$
I(d)=\frac{I_0}{a_1+a_2 d+a_3 d^2},
$$

其中 $a_1,a_2,a_3$ 分别控制常数、线性及平方反比分量。$a_3>0$ 时远处亮度下降更快；$a_2>0$ 时介于线性与平方之间；$a_1>0$ 且其余为零时近似无距离衰减。方向 $\mathbf{L}$ 由 $\mathbf{p}$ 指向光源位置后归一化，$d$ 取该向量长度，再与阴影可见性、Phong 着色联立得到最终像素颜色。

## 2 程序设计与实现

### 2.1 总体架构

系统在场景配置解析、几何图元与 Phong 着色之上，增加递归光线追踪模块，将沿射线计算辐射亮度的过程集中封装。Phong 材质扩展反射系数、透射系数与折射率。主程序解析投射阴影、最大反弹次数、射线权重阈值及半透明阴影等选项后读入场景，逐像素调用光线追踪；亦可启用 OpenGL 预览，并对单像素射线树做可视化调试。各图元另提供阴影射线求交接口，以便在可见性判定时加速遮挡测试。

```cpp
globalRayTracer = new RayTracer(globalParser, globalArgs.max_bounces,
                                globalArgs.cutoff_weight,
                                globalArgs.shadows,
                                globalArgs.shade_back,
                                globalArgs.transparent_shadows);
if (globalArgs.use_gui) {
  glutInit(&argc, argv);
  GLCanvas canvas;
  canvas.initialize(globalParser, renderScene, traceRayAtScreen);
}
renderScene();
```

逐像素渲染时，将像素中心映射为屏幕坐标，生成相机射线并追踪：

```cpp
for (int y = 0; y < globalArgs.height; y++) {
  for (int x = 0; x < globalArgs.width; x++) {
    float u = (x + 0.5f) / globalArgs.width;
    float v = (y + 0.5f) / globalArgs.height;
    Ray ray = generateCameraRay(u, v);
    Hit hit(max_t, NULL, Vec3f(0, 0, 0));
    Vec3f color = globalRayTracer->traceRay(
        ray, camera->getTMin(), 0, 1.0f, 1.0f, hit);
    if (hit.getMaterial() != NULL)
      image.SetPixel(x, y, color);
  }
}
```

### 2.2 材质扩展

Phong 材质在漫反射色、高光色与高光指数之外，增加反射色 $\mathbf{k}_r$、透明色 $\mathbf{k}_t$ 与折射率：

```cpp
class PhongMaterial : public Material {
  Vec3f diffuseColor;
  Vec3f specularColor;
  float exponent;
  Vec3f reflectiveColor;
  Vec3f transparentColor;
  float indexOfRefraction;
};
```

场景配置解析在读取材质时解析上述字段，供反射与折射分支查询。

### 2.3 递归追踪与终止条件

光线追踪模块保存场景访问入口，以及最大反弹次数、权重阈值、是否投射阴影、是否背面着色、是否半透明阴影等配置。主过程先对主射线与场景物体组求最近交点，再计算局部着色，并按材质系数决定是否发射反射或折射二次射线：

```cpp
if (bounces > maxBounces || weight < cutoffWeight)
  return background;
if (!group->intersect(ray, bestHit, tmin))
  return background;
Vec3f color = computeLocalShading(ray, hit, normal);
// 随后按反射 / 透射系数发射二次射线并累加
```

进入反射或折射分支时，反弹深度加一，权重分别乘以反射系数或透射系数的模长，使间接贡献随路径衰减。

### 2.4 局部着色与投射阴影

局部着色先累加环境光与漫反射系数的逐分量乘积，再对每个光源查询方向、颜色与距离。开启投射阴影时，自交点沿光源方向发射阴影射线；起点沿几何法线做微小偏置；对点光源将求交上界限制为到着色点到光源的距离：

```cpp
Vec3f color = componentMultiply(parser->getAmbientLight(),
                                material->getDiffuseColor());
for (int i = 0; i < parser->getNumLights(); i++) {
  parser->getLight(i)->getIllumination(
      hit.getIntersectionPoint(), lightDir, lightColor, distanceToLight);
  if (castShadows) {
    Vec3f shadowAtten = getShadowAttenuation(
        hit.getIntersectionPoint(), hit.getNormal(),
        lightDir, distanceToLight);
    if (isFullyBlocked(shadowAtten))
      continue;
    lightColor = componentMultiply(lightColor, shadowAtten);
  }
  color += material->Shade(ray, shadedHit, lightDir, lightColor);
}
```

默认路径下任意遮挡即丢弃该光源贡献。开启半透明阴影时，对透明遮挡物按 Beer–Lambert 定律计算段内透射率并沿光路迭代，直至到达光源或完全遮挡。

### 2.5 镜面反射与折射

镜面反射方向按 $\mathbf{R}=\mathbf{I}-2(\mathbf{N}\cdot\mathbf{I})\mathbf{N}$ 计算：

```cpp
Vec3f RayTracer::mirrorDirection(const Vec3f &normal,
                                 const Vec3f &incoming) const {
  float nDotI = normal.Dot3(incoming);
  return incoming - normal * (2.0f * nDotI);
}
```

反射系数任一分量大于零时，自交点沿 $\mathbf{R}$ 递归追踪，结果以反射系数与远端辐射亮度的逐分量乘积叠加。

折射方向按 Snell 公式求得，并根据几何法线与入射方向点积的符号区分由外入内或由内出外：

```cpp
float eta = index_i / index_t;
float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
if (k < 0.0f)
  return false;  // 全反射
transmitted = incoming * eta - n * (eta * cosi + sqrtf(k));
transmitted.Normalize();
```

进入物体时压入外围折射率并将当前介质折射率更新为物体值；离开时弹出栈顶恢复上一层折射率。开启背面着色且射线从背面入射时翻转法线，否则背面着黑色。

### 2.6 点光源衰减

点光源按 $1/(a_1+a_2 d+a_3 d^2)$ 计算衰减后的光色，与阴影测试及 Phong 着色联立：

```cpp
dir = position - p;
distanceToLight = dir.Length();
dir.Normalize();
float attenuation = 1 / (attenuation_1 +
                         attenuation_2 * distanceToLight +
                         attenuation_3 * distanceToLight * distanceToLight);
col = color * attenuation;
```

### 2.7 射线树调试与阴影加速

命中表面后记录主射线段；阴影、反射、折射路径分别记录对应线段，供 OpenGL 预览中按键可视化单像素射线树。屏幕坐标到相机射线的生成与主渲染循环共用同一套逻辑。

阴影专用求交在可见性判定时可只返回是否遮挡及最近参数 $t$，物体组遇到首个交点即可提前返回。需要半透明阴影时再查询遮挡物的透射系数以计算衰减。

## 3 实验结果

测试命令形如：

```text
# 球体投射阴影；-shadows 开启投射阴影
raytracer -input <球体投射阴影场景> -size 200 200 -output output4_01.tga -shadows
# 反射球体；-bounces 最大反弹次数，-weight 射线权重阈值
raytracer -input <反射球体场景> -size 200 200 -output output4_04d.tga -shadows -bounces 3 -weight 0.01
# 多根透明柱；-shade_back 背面着色，-transparent_shadows 半透明阴影
raytracer -input <多根透明柱场景> -size 200 200 -output output4_06f.tga -shadows -bounces 5 -weight 0.01 -shade_back -transparent_shadows
```

全部场景在 $200\times 200$ 分辨率下渲染。

### 3.1 投射阴影

| 球体投射阴影 | 彩色遮挡物有色阴影 |
|:---:|:---:|
| ![output4_01](OutputFiles/output4_01.png) | ![output4_02](OutputFiles/output4_02.png) |
| 不透明球在地面投下清晰暗区 | 半透明彩色遮挡使阴影带对应色调 |

球体阴影边界清晰；彩色半透明遮挡物使阴影呈现对应色调，说明光源可见性测试与局部着色正确联立。

### 3.2 镜面地板与反射深度

| 镜面地板（$N_{\max}=1$） |
|:---:|
| ![output4_03](OutputFiles/output4_03.png) |
| 地面映出上方物体的一次镜像 |

| 反射球体　$N_{\max}=0$ | $N_{\max}=1$ | $N_{\max}=2$ | $N_{\max}=3$ |
|:---:|:---:|:---:|:---:|
| ![output4_04a](OutputFiles/output4_04a.png) | ![output4_04b](OutputFiles/output4_04b.png) | ![output4_04c](OutputFiles/output4_04c.png) | ![output4_04d](OutputFiles/output4_04d.png) |
| 仅局部 Phong 着色 | 出现一次环境反射 | 二次反射细节增加 | 多次互反射更完整 |

反弹次数为 $0$ 时球面仅有局部着色；随深度增加，周围环境与多次反射细节逐步出现在球面中，说明递归终止条件与反射权重缩放工作正常。

### 3.3 透明折射与半透明阴影

| 单根透明柱（背面着色开启） |
|:---:|
| ![output4_05](OutputFiles/output4_05.png) |
| 折射路径穿过柱体并扭曲背景 |

| 多根透明柱　$N_{\max}=0$ | $N_{\max}=2$ | $N_{\max}=5$ | $N_{\max}=5$＋半透明阴影 |
|:---:|:---:|:---:|:---:|
| ![output4_06a](OutputFiles/output4_06a.png) | ![output4_06c](OutputFiles/output4_06c.png) | ![output4_06f](OutputFiles/output4_06f.png) | ![output4_06f_trans](OutputFiles/output4_06f_trans.png) |
| 仅局部着色 | 杆间折射初现 | 多次折射较完整 | 阴影区保留部分透光 |

背面着色开启后，折射路径穿过透明柱并扭曲背景；反弹次数增加时，杆间多次折射逐渐完整。半透明阴影使阴影区域保留部分透光，与不透明遮挡形成对比。

### 3.4 折射率对比

| $\eta=1.0$ | $\eta=1.1$ | $\eta=2.0$ | $\eta=1.1$＋半透明阴影 |
|:---:|:---:|:---:|:---:|
| ![output4_07](OutputFiles/output4_07.png) | ![output4_08](OutputFiles/output4_08.png) | ![output4_09](OutputFiles/output4_09.png) | ![output4_08_trans](OutputFiles/output4_08_trans.png) |
| 背景几乎不弯折 | 轻微扭曲 | 扭曲明显加剧 | 投影边缘更柔和 |

$\eta=1.0$ 时背景几乎不弯折；$\eta$ 增大后背景扭曲加剧，符合 Snell 定律。半透明阴影使球体投影边缘更为柔和。

### 3.5 点光源与距离衰减

| 点光源距离测试 | 常数衰减 | $1/d$ 衰减 | $1/d^2$ 衰减 |
|:---:|:---:|:---:|:---:|
| ![output4_10](OutputFiles/output4_10.png) | ![output4_11](OutputFiles/output4_11.png) | ![output4_12](OutputFiles/output4_12.png) | ![output4_13](OutputFiles/output4_13.png) |
| 点光源下的场景明暗 | 近远亮度差较小 | 中等对比 | 远处明显变暗 |

点光源照明下物体亮度随距离变化；$1/d^2$ 衰减对比度最高，远处球体明显变暗，与公式 $I(d)=I_0/(a_1+a_2 d+a_3 d^2)$ 的预期相符。

### 3.6 多面体宝石

| 三角网格宝石　$N_{\max}=0$ | $N_{\max}=2$ | $N_{\max}=5$ |
|:---:|:---:|:---:|
| ![output4_14a](OutputFiles/output4_14a.png) | ![output4_14c](OutputFiles/output4_14c.png) | ![output4_14f](OutputFiles/output4_14f.png) |
| 仅局部着色 | 内部折射初现 | 多次反射／折射亮斑丰富 |

反弹次数增加后，内部多次反射与折射形成更多亮斑与色块，说明折射系数栈与递归追踪在复杂三角网格上正常工作。

## 4 问题记录

最初实现的透明柱与透明球场景中，折射看像半透明叠色，背景几乎不弯折。对照 Snell 矢量公式后发现折射方向写成 $\mathbf{T}=\eta\mathbf{I}+N(\eta\cos\theta_i-\sqrt{k})$，原因是在已将 $\cos\theta_i$ 规范为非正之后，切向分量的符号与标准式 $\mathbf{T}=\eta\mathbf{I}-N(\eta\cos\theta_i+\sqrt{k})$ 相反，透射射线未按正确角度偏折。更正算式形式后，背景出现扭曲，问题解决。

开启背面着色后，透明柱交叠处地面出现突兀的死黑断层。阴影射线起点误用了经 `shade_back` 翻转后的着色法线：背面时法线指向物体内侧，起点被推入体内，阴影射线在极小 $t$ 处再次命中自身表面，被判为完全遮挡。改为用几何法线 `hit.getNormal()` 做偏置，并根据射线方向与法线的点积选择 $\pm\varepsilon$ 侧后，假阴影消失。

折射与反射二次射线的起点也曾按 $\mathbf{N}\cdot\mathbf{d}$ 的符号固定沿法线外推：由外入内时起点被推到物体外侧，透射射线向内发射时再次自相交，多次反弹后内部折射路径断断续续。改为沿实际发射方向（折射或反射方向）相对几何法线偏置后，透明柱与宝石 mesh 上的递归路径修复，成像清晰。

实现基础阴影后，透明柱在地面投下与不透明物相同的硬阴影，与样例中透光阴影不符。发现课程样例默认把半透明体当作不透明遮挡；Extra Credit 中说明了需沿 shadow ray 逐段穿过透明体。为此在 $[\varepsilon,\,t_{\mathrm{light}}]$ 内迭代求交，对每段光程用 Beer–Lambert 累积衰减。另外阴影实现过程中还出现一次着色错误，即直接把透射色 $\mathbf{k}_t$ 当作吸收系数，青色 $\mathbf{k}_t\approx(0,1,1)$ 时红光几乎不衰减、绿蓝反而衰减，地面阴影偏淡粉且几乎看不见。将吸收系数改为 $\boldsymbol{\kappa}=\mathbf{1}-\mathbf{k}_t$，并调节衰减强度后，彩色投影的色相与层次才终于与透射材质样例相同。

多根透明柱交叠时，若物外介质恒为真空，$\eta_t\in\{1,\eta_{\mathrm{obj}}\}$，离开内层柱体时 $\eta_i$ 会被错误设为 $1$，穿过外层柱壁时 Snell 比例错误，杆间折射出现错位亮斑。参照作业对嵌套介质的说明，在光追的递归过程中中维护了一个 IOR 栈，由外入内时压入当前外围折射率，由内出外时弹出栈顶恢复上一层介质，折射递归传入更新后的下一层 IOR 与栈深度后，嵌套场景正常显示，问题解决。

调试反射深度时，`-bounces 0` 与 `-bounces 1` 的对比图有偏差：原因是对终止条件的理解出现了偏差，`bounces` 从 $0$ 起计，当 `bounces < maxBounces` 时才发射二次射线，修正参数对应值后问题解决。

课程提供文件的老问题：场景解析器中 `new (Light*)[n]` 在当前 MinGW 的 g++ 下无法通过编译，改为 `new Light*[n]` 后解决；`rayTree` 直接包含 `<GL/gl.h>` 会与标准库的 `byte` 冲突，改为经 `gl_headers.h` 先包含 Windows 头文件后解决。

## 5 总结

在射线–场景求交与 Phong 局部着色的基础上，向各光源发射阴影射线判断可见性，并沿镜面方向与 Snell 折射方向递归追踪二次射线，从而将射线投射扩展为支持阴影、反射与折射的光线追踪器。以反弹次数与射线权重截断无限递归；背面着色处理透明体内外法线朝向；折射率栈支持嵌套透明介质。Beer–Lambert 半透明阴影与阴影射线快速求交改善了透明场景下的视觉效果与性能。点光源的距离衰减与方向光、环境光一并纳入局部着色，使各测试场景的结果与预期物理行为相符。

## 附录（TODO list 记录）

- [x] 扩展 Phong 材质以存储反射系数、透射系数与折射率，并在场景解析中读取
- [x] 实现递归光线追踪模块，沿射线计算辐射亮度，并以反弹深度与权重截断递归
- [x] 在主程序中逐像素调用光线追踪，并解析投射阴影、最大反弹次数与权重阈值
- [x] 实现投射阴影，自交点沿光源方向发射阴影射线并做微小偏置
- [x] 实现镜面反射方向计算，并按反射系数递归累加反射贡献
- [x] 按 Snell 定律实现折射，根据法线与入射方向切换折射率，并支持背面着色
- [x] 集成射线树可视化，记录主射线及阴影、反射、折射线段
- [x] 整合点光源距离衰减，并与局部着色、阴影测试联立

### Extra Credit

- [x] 渲染半透明阴影
- [x] 支持嵌套折射材质
- [x] 阴影射线加速
- [ ] Fresnel 反射项
- [ ] 其他 BRDF 模型（如 Cook-Torrance 或 Ward）
- [ ] 各向异性 BRDF
