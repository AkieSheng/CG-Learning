# Assignment 2: Transformations & Additional Primitives

作者：王洵

## 1 实验原理

### 1.1 射线投射与表面法线

射线投射（Ray Casting）将图像上每个像素对应到三维空间中的一条射线，并求该射线与场景几何体的交点。射线可参数化为

$$
\mathbf{p}(t)=\mathbf{o}+t\mathbf{d},
$$

其中 $\mathbf{o}$ 为射线原点，$\mathbf{d}$ 为方向向量，$t$ 为沿射线的标量参数。交点处的单位法线 $\mathbf{n}$ 刻画局部切平面的朝向，供后续着色使用。

- 球心为 $\mathbf{c}_s$、半径为 $r$ 时，交点 $\mathbf{p}$ 处的外法线取 $\mathbf{n}=\mathrm{normalize}(\mathbf{p}-\mathbf{c}_s)$。
- 无限平面 $\mathbf{x}\cdot\hat{\mathbf{n}}=d$ 中，$\hat{\mathbf{n}}$ 为单位法线，$d$ 为原点到平面的有符号距离，法线在整张平面上恒为 $\hat{\mathbf{n}}$。
- 顶点 $\mathbf{a},\mathbf{b},\mathbf{c}$ 按逆时针顺序给出时，三角形面法线取 $\mathbf{n}=\mathrm{normalize}\bigl((\mathbf{b}-\mathbf{a})\times(\mathbf{c}-\mathbf{a})\bigr)$，从而固定朝外一侧。

沿每条射线在所有图元间保留最小 $t$（且 $t$ 不小于下界 $t_{\min}$）的交点，即得到该像素所见的最近表面及其法线。

### 1.2 法线可视化与漫反射着色

法线可视化（Normal Visualization）将 $\mathbf{n}=(n_x,n_y,n_z)$ 映射为颜色 $(|n_x|,|n_y|,|n_z|)$，使轴向法线在 RGB 通道上呈现纯色；未命中表面的像素以黑色表示。

漫反射着色（Diffuse Shading）基于 Lambert 余弦定律（Lambert's cosine law）。设指向光源的单位方向为 $\mathbf{L}$、单位法线为 $\mathbf{N}$，漫反射强度为 $d=\max(\mathbf{L}\cdot\mathbf{N},\,0)$。物体反照率为 $\mathbf{c}_{\mathrm{object}}$，第 $i$ 个光源颜色为 $\mathbf{c}_{\mathrm{light},i}$，环境光为 $\mathbf{c}_{\mathrm{ambient}}$，则

$$
\mathbf{c}_{\mathrm{pixel}}
=
\mathbf{c}_{\mathrm{ambient}}\odot\mathbf{c}_{\mathrm{object}}
+
\sum_i
\max(\mathbf{L}_i\cdot\mathbf{N},\,0)\,
\mathbf{c}_{\mathrm{light},i}\odot\mathbf{c}_{\mathrm{object}},
$$

其中 $\odot$ 表示逐分量相乘。当 $\mathbf{L}\cdot\mathbf{N}\le 0$ 时该光源贡献为零；环境光项为背光侧提供基底亮度。若环境光为白色且光源颜色为零，则退化为常数着色。

### 1.3 表面正反面

当视线方向 $\mathbf{d}$ 与几何法线 $\mathbf{N}$ 满足 $\mathbf{d}\cdot\mathbf{N}>0$ 时，射线从法线所指“背面”进入表面。相机位于物体内部时，为使内侧仍能正确受光，在着色前将法线取反：$\mathbf{N}'=-\mathbf{N}$。关闭该处理时，背面处 $\mathbf{L}\cdot\mathbf{N}\le 0$，漫反射贡献为零，画面呈暗色，便于检查面朝向。

### 1.4 透视相机

透视相机（Perspective Camera）中，各像素射线方向随屏幕位置变化。设相机中心为 $\mathbf{c}$，单位视线方向 $\hat{\mathbf{d}}$，正交化后的竖直轴 $\hat{\mathbf{u}}$ 与水平轴 $\hat{\mathbf{h}}=\hat{\mathbf{d}}\times\hat{\mathbf{u}}$ 构成右手标准正交基（orthonormal basis），视场角为 $\theta$。在距相机沿 $\hat{\mathbf{d}}$ 单位距离的虚拟成像平面上，半高为 $\tan(\theta/2)$。屏幕坐标 $(u,v)\in[0,1]^2$ 对应

$$
\mathbf{s}(u,v)
=
\mathbf{c}+\hat{\mathbf{d}}
+\bigl(v-\tfrac{1}{2}\bigr)\,2\tan(\tfrac{\theta}{2})\,\hat{\mathbf{u}}
+\bigl(u-\tfrac{1}{2}\bigr)\,2\tan(\tfrac{\theta}{2})\,\hat{\mathbf{h}},
$$

射线方向为 $\mathbf{d}=\mathrm{normalize}(\mathbf{s}-\mathbf{c})$。方向已归一化时，沿 $\hat{\mathbf{d}}$ 平移成像平面不改变射线方向，故成像平面距离与绝对尺寸只需满足同一视场角下的比例关系。透视相机射线从有限视点发出，$t_{\min}$ 取小正数，以免起点落在物体内部时产生数值自相交。非正方形分辨率下对 $u$ 或 $v$ 做宽高比裁剪，使成像平面宽高比与图像相符。

### 1.5 平面与三角形求交

平面求交将 $\mathbf{p}(t)=\mathbf{o}+t\mathbf{d}$ 代入 $\mathbf{x}\cdot\hat{\mathbf{n}}=d$，得

$$
t=\frac{d-\mathbf{o}\cdot\hat{\mathbf{n}}}{\mathbf{d}\cdot\hat{\mathbf{n}}}.
$$

当 $|\mathbf{d}\cdot\hat{\mathbf{n}}|$ 接近零时，射线与平面平行，无唯一交点。

三角形求交采用 Möller–Trumbore 算法。令 $\mathbf{e}_1=\mathbf{b}-\mathbf{a}$，$\mathbf{e}_2=\mathbf{c}-\mathbf{a}$，$\mathbf{p}=\mathbf{d}\times\mathbf{e}_2$，$\det=\mathbf{e}_1\cdot\mathbf{p}$。若 $|\det|$ 过小，则射线与三角形平面平行。记 $\mathbf{t}=\mathbf{o}-\mathbf{a}$，重心坐标

$$
u=\frac{\mathbf{t}\cdot\mathbf{p}}{\det},\qquad
v=\frac{\mathbf{d}\cdot(\mathbf{t}\times\mathbf{e}_1)}{\det}.
$$

当 $u\ge 0$、$v\ge 0$ 且 $u+v\le 1$ 时交点落在三角形内，交点参数为

$$
t=\frac{\mathbf{e}_2\cdot(\mathbf{t}\times\mathbf{e}_1)}{\det}.
$$

### 1.6 仿射变换下的射线与法线

仿射变换（Affine Transformation）将物体从局部坐标系映射到世界坐标系。点变换为 $\mathbf{x}'=M\mathbf{x}$（齐次坐标）；射线逆变换为

$$
\mathbf{o}_{\mathrm{loc}}=M^{-1}\mathbf{o},\qquad
\mathbf{d}_{\mathrm{loc}}=M^{-1}\mathbf{d}
$$

（方向变换不含平移分量）。法线为协变向量，世界空间法线取

$$
\mathbf{n}_{\mathrm{world}}=(M^{-1})^{T}\mathbf{n}_{\mathrm{loc}},
$$

再归一化，从而在非均匀缩放下仍与变换后的切平面正交。求交时先在局部空间计算交点，再将参数 $t$、材质与变换后的法线写回世界空间。

## 2 程序设计与实现

### 2.1 总体架构

系统由线性代数库、图像读写库、场景解析模块与渲染主程序组成：

1. 场景解析模块读入场景描述，构造相机、光源、材质与物体层次。
2. 几何模块提供球体、平面、三角形、物体组与仿射变换节点的求交。
3. 相机模块提供正交相机与透视相机的射线生成。
4. 主程序解析命令行，逐像素生成射线、求最近交点，并写入漫反射图、法线图与深度图。

主渲染循环中，像素中心映射到屏幕坐标后生成射线，与场景物体组求交：

```cpp
Ray ray = camera->generateRay(Vec2f(u, v));
Hit hit(max_t, NULL, Vec3f(0, 0, 0));
bool intersected = group->intersect(ray, hit, camera->getTMin());
```

### 2.2 交点信息与球体求交

交点记录同时保存参数 $t$、材质、法线与空间交点坐标：

```cpp
void set(float _t, Material *m, Vec3f n, const Ray &ray) {
  t = _t; material = m; normal = n;
  intersectionPoint = ray.pointAtParameter(t);
}
```

球体求交按二次方程求解近端与远端两根；满足 $t\ge t_{\min}$ 且 $t$ 小于当前最近值时，写入外法线：

```cpp
Vec3f normal = r.pointAtParameter(t) - center;
normal.Normalize();
h.set(t, material, normal, r);
```

### 2.3 平面与三角形

平面求交按 $t=(d-\mathbf{o}\cdot\hat{\mathbf{n}})/(\mathbf{d}\cdot\hat{\mathbf{n}})$ 计算，并在 $|\mathbf{d}\cdot\hat{\mathbf{n}}|$ 过小时判定平行：

```cpp
float denom = normal.Dot3(r.getDirection());
if (fabs(denom) < 1e-6f)
  return false;
float t = (d - normal.Dot3(r.getOrigin())) / denom;
```

三角形构造时用边叉积固定面法线；求交实现 Möller–Trumbore 中的重心坐标检验：

```cpp
float u = tvec.Dot3(pvec) * invDet;
if (u < 0.0f || u > 1.0f)
  return false;
float v = r.getDirection().Dot3(qvec) * invDet;
if (v < 0.0f || u + v > 1.0f)
  return false;
float t = edge2.Dot3(qvec) * invDet;
```

三角网格由外部网格文件的顶点与面索引解析而来，每个三角面作为独立图元加入物体组。

### 2.4 仿射变换节点

变换节点保存物体到世界的矩阵 $M$，并预计算 $(M^{-1})^{T}$。求交时将射线变到局部空间，调用子物体求交后，将法线变回世界空间：

```cpp
objectMatrix.Transform(origin);
objectMatrix.TransformDirection(direction);
Ray localRay(origin, direction);
if (!object->intersect(localRay, localHit, tmin))
  return false;
Vec3f normal = localHit.getNormal();
inverseMatrix.TransformDirection(normal);
normal.Normalize();
h.set(localHit.getT(), localHit.getMaterial(), normal, r);
```

场景中的缩放、旋转、平移及任意矩阵指令按左乘顺序累积为单一变换矩阵，再包装被变换的子物体。

### 2.5 透视相机

构造时由半视场角计算虚拟成像平面半高；生成射线时在 $\mathbf{c}+\hat{\mathbf{d}}$ 处插值采样点并归一化方向：

```cpp
halfHeight = tanf(angle * 0.5f);
Vec3f screenPoint = center + direction
    + (v - 0.5f) * 2.0f * halfHeight * up
    + (u - 0.5f) * 2.0f * halfHeight * horizontal;
Vec3f rayDir = screenPoint - center;
rayDir.Normalize();
return Ray(center, rayDir);
```

正交相机仍发出平行射线，$t_{\min}$ 取大负数；透视相机取 $t_{\min}=10^{-4}$。

### 2.6 着色与多模式输出

命中后根据 $\mathbf{d}\cdot\mathbf{N}$ 判定背面；开启双面着色时翻转法线再计算漫反射。漫反射着色累加环境光与各光源贡献：

```cpp
Vec3f color = componentMultiply(ambient, objectColor);
for (int i = 0; i < parser.getNumLights(); i++) {
  parser.getLight(i)->getIllumination(
      hit.getIntersectionPoint(), lightDir, lightColor);
  float diffuse = normal.Dot3(lightDir);
  if (diffuse > 0.0f)
    color += componentMultiply(lightColor, objectColor) * diffuse;
}
```

法线可视化将几何法线（未经翻转）的分量绝对值映射为 RGB；深度可视化将 $t$ 线性映射到给定近远区间上的灰度，并对区间外取值截断。

## 3 实验结果

测试命令形如：

```text
# -input 输入场景；-size 图像宽高；-output 漫反射着色输出
raytracer -input <漫反射单球场景> -size 200 200 -output output2_01.tga

# -depth 深度映射近远界与深度图；-normals 法线可视化；-shade_back 双面着色
raytracer -input <球体内侧场景> -size 200 200 -output output2_05.tga \
  -depth 9 11 depth2_05.tga -normals normals2_05.tga -shade_back
```

十六组场景均在 $200\times 200$ 分辨率下渲染。

### 3.1 漫反射与环境光

| 方向光漫反射 | 加入环境光 |
|:---:|:---:|
| ![output2_01](OutputFiles/output2_01.png) | ![output2_02](OutputFiles/output2_02.png) |
| 单球一侧亮、一侧暗的 Lambert 分布 | 暗部仍保留基底亮度 |

环境光抬高了背光侧亮度，使球体轮廓在暗部仍可辨认。

### 3.2 多色光源与法线可视化

| 漫反射着色 | 法线可视化 |
|:---:|:---:|
| ![output2_03](OutputFiles/output2_03.png) | ![normals2_03](OutputFiles/normals2_03.png) |
| 三盏不同颜色方向光在球面上叠加 | 球面法线呈平滑 RGB 渐变 |

法线图中朝向坐标轴的区域分别偏红、绿、蓝，与法线分量绝对值映射相符。

### 3.3 透视投影

| 漫反射着色 | 法线可视化 |
|:---:|:---:|
| ![output2_04](OutputFiles/output2_04.png) | ![normals2_04](OutputFiles/normals2_04.png) |
| 三球在深度上形成遮挡，近大远小 | 曲面朝向变化清晰可见 |

透视投影引入汇聚感，前后球之间的遮挡关系正确。

### 3.4 球体内侧与双面着色

| 开启双面着色 | 关闭双面着色 |
|:---:|:---:|
| ![output2_05](OutputFiles/output2_05.png) | ![output2_05_no_back](OutputFiles/output2_05_no_back.png) |
| 相机位于球内，内侧表面受光照亮 | 背面像素为黑色 |

| 深度图 | 法线图 |
|:---:|:---:|
| ![depth2_05](OutputFiles/depth2_05.png) | ![normals2_05](OutputFiles/normals2_05.png) |
| 区分内外壳层深度 | 几何法线分布 |

开启双面着色后，内侧法线经翻转仍能产生有效漫反射；关闭时背面全黑，便于对照面朝向。

### 3.5 无限平面

| 漫反射着色 | 深度图 | 法线图 |
|:---:|:---:|:---:|
| ![output2_06](OutputFiles/output2_06.png) | ![depth2_06](OutputFiles/depth2_06.png) | ![normals2_06](OutputFiles/normals2_06.png) |
| 红、绿、蓝三平面与灰色地面 | 前后层次分明 | 各平面为常色块 |

透视下平面夹角与遮挡正确；法线图上每个平面颜色恒定，说明法线在整张平面上不变。

### 3.6 球体与三角形

| 开启双面着色 | 关闭双面着色 |
|:---:|:---:|
| ![output2_07](OutputFiles/output2_07.png) | ![output2_07_no_back](OutputFiles/output2_07_no_back.png) |
| 背面三角形亦可见 | 背面全黑 |

| 深度图 | 法线图 |
|:---:|:---:|
| ![depth2_07](OutputFiles/depth2_07.png) | ![normals2_07](OutputFiles/normals2_07.png) |

球面与三角面片共存时，最近交点选择正确；双面着色开关对背面可见性影响明显。

### 3.7 三角网格

| 立方体 | Stanford bunny（约 200 面） | Stanford bunny（约 1k 面） |
|:---:|:---:|:---:|
| ![output2_08](OutputFiles/output2_08.png) | ![output2_09](OutputFiles/output2_09.png) | ![output2_10](OutputFiles/output2_10.png) |

面片更密时轮廓更光滑，遮挡关系正确，说明网格解析与三角形求交可用。

### 3.8 仿射变换

| 非均匀缩放 | 旋转 | 缩放与旋转组合 |
|:---:|:---:|:---:|
| ![output2_11](OutputFiles/output2_11.png) | ![output2_12](OutputFiles/output2_12.png) | ![output2_13](OutputFiles/output2_13.png) |

| 对应法线图 | 对应法线图 | 对应法线图 |
|:---:|:---:|:---:|
| ![normals2_11](OutputFiles/normals2_11.png) | ![normals2_12](OutputFiles/normals2_12.png) | ![normals2_13](OutputFiles/normals2_13.png) |

变换后的外形与法线方向和预期几何相符，说明射线逆变换与 $(M^{-1})^{T}$ 法线变换正确。

### 3.9 复杂变换与深度映射

| 坐标轴立方体 | 嵌套变换场景 | 交点参数缩放测试 |
|:---:|:---:|:---:|
| ![output2_14](OutputFiles/output2_14.png) | ![output2_15](OutputFiles/output2_15.png) | ![output2_16](OutputFiles/output2_16.png) |

![depth2_16](OutputFiles/depth2_16.png)

*图：交点参数缩放测试场景的深度图。不同近远区间下灰度映射正确，近处偏白、远处偏黑。*

多层嵌套变换下几何与着色结果正常，说明变换链累积与求交流程可用。

## 4 问题记录

对 -shade_back 参数理解有误。关闭双面着色时，球体内侧场景与球体—三角形场景的对照图与样例不符。把背面像素写成纯绿，内侧大球几乎占满视野，画面为绿底（样例黑底）。之后改为使用场景背景色：内侧场景背景恰为黑色，初步看似正确；但三角形场景背景为紫色，背面三角形被涂成与背景同色，与样例不符。将背面改为纯黑后，问题解决。

场景解析器中光源数组写成旧式 `new (Light*)[n]`，在当前 MinGW 的 g++ 下无法通过编译。改为 `new Light*[n]` 后解决。

在 Windows 下批量渲染时，Makefile 的 `mkdir -p` 与 `../raytracer` 路径写法与 cmd 不兼容，改为按目录是否存在创建输出文件夹，并用反斜杠形式调用可执行文件后，问题解决。

## 5 总结

在射线投射框架上，本实验为交点记录法线，实现 Lambert 漫反射与环境光叠加，并增加法线与深度两种可视化；同时引入透视相机，以及平面、三角形与仿射变换等图元。射线在局部空间求交后按 $(M^{-1})^{T}$ 变换法线，使缩放与旋转后的着色与几何形状相符；着色前根据视线与法线关系处理背面，便于内侧观察与朝向检查。

## 附录（TODO list 记录）

- [x] 在交点记录中保存法线与交点坐标，并在球体求交中写入外法线
- [x] 实现法线可视化，将法线分量绝对值映射为 RGB 并输出图像
- [x] 实现漫反射着色，按环境光与多光源累加公式计算像素颜色
- [x] 实现双面着色选项，在着色前按视线与法线点积判定并翻转背面法线
- [x] 实现透视相机，在虚拟成像平面上插值采样点并生成射线
- [x] 实现无限平面图元及其射线求交
- [x] 实现三角形图元，采用 Möller–Trumbore 算法求交，并解析外部网格文件
- [x] 实现仿射变换节点，将射线逆变换到局部空间求交后变换法线回世界空间
