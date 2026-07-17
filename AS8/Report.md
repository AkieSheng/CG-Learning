# Assignment 8: Curves & Surfaces

作者：王洵

## 1 实验原理

### 1.1 参数三次曲线的矩阵形式

参数三次曲线将几何形状表示为控制点几何与固定基函数的乘积。设参数 $t\in[0,1]$，幂基向量

$$
\mathbf{T}=(t^3,t^2,t,1)^\mathrm{T},
$$

控制点 $\mathbf{p}_0,\mathbf{p}_1,\mathbf{p}_2,\mathbf{p}_3$ 按列排成几何矩阵 $G$（各列为齐次坐标 $(\mathbf{p}_i,1)^\mathrm{T}$），基矩阵为 $B$，则曲线上的点满足

$$
\mathbf{Q}(t)=G\,B\,\mathbf{T}.
$$

其中 $B$ 只依赖样条类型，与具体控制点无关；改变 $G$ 即改变曲线形状。三次 Bézier 曲线（Bézier curve）与均匀三次 B 样条（B-spline）分别对应常值基矩阵 $B_{\mathrm{bezier}}$ 与 $B_{\mathrm{bspline}}$。前者在端点插值首尾控制点，曲线落在控制多边形的凸包内；后者一般不插值控制点，但相邻段之间具有 $C^2$ 连续性，适于平滑造型。

### 1.2 Bézier 与 B 样条的四点互转

同一条几何曲线同时写成 $G_{\mathrm{bezier}}B_{\mathrm{bezier}}\mathbf{T}$ 与 $G_{\mathrm{bspline}}B_{\mathrm{bspline}}\mathbf{T}$。令二者对任意 $\mathbf{T}$ 相等，得到

$$
G_{\mathrm{dst}}=G_{\mathrm{src}}\,B_{\mathrm{src}}\,B_{\mathrm{dst}}^{-1}.
$$

由此，一组 Bézier 控制点经上式得到产生相同 $\mathbf{Q}(t)$ 的 B 样条控制点，反向转换同理。该关系在单段、四个控制点时自由度匹配；多于四点时，两种表示的控制点个数与段间连接条件一般不再一一对应。

### 1.3 多控制点的局部三次拼接

多于四个控制点时，整条曲线由若干局部三次段拼接而成。

- **均匀 B 样条**：对控制点序列 $\{ \mathbf{p}_0,\ldots,\mathbf{p}_{n+2} \}$，每次取连续四点 $(\mathbf{p}_i,\mathbf{p}_{i+1},\mathbf{p}_{i+2},\mathbf{p}_{i+3})$ 构成一段，窗口每次前移一个点；$n+3$ 个控制点产生 $n$ 段。
- **Bézier 曲线**：控制点个数取 $3n+1$，第 $k$ 段使用下标 $3k$ 至 $3k+3$ 的四点；相邻段共享一个端点，段间 $G^1$ 或 $C^1$ 连续性由相邻控制点的相对位置决定。

沿整条曲线的全局参数 $u\in[0,1]$ 按段数线性映射到各段：设共有 $S$ 段，取 $\tilde{u}=uS$，段索引 $s=\lfloor\tilde{u}\rfloor$，局部参数 $t=\tilde{u}-s$，再在该段上求值 $\mathbf{Q}_s(t)$，使轮廓采样在整条曲线上均匀分布。

### 1.4 旋转曲面

旋转曲面（surface of revolution）由平面轮廓绕固定轴扫掠生成。设轮廓点在 $xy$ 平面内为 $(x,y)$，绕 $y$ 轴旋转角度 $\theta$，则三维点为

$$
\mathbf{r}(\theta)=(x\cos\theta,\,y,\,x\sin\theta).
$$

参数域上沿轮廓方向与旋转方向分别离散为 $N_u$、$N_v$ 步，在规则四边形网格的角点上赋值 $\mathbf{r}$，再剖分为三角形，得到逼近光滑曲面的三角网格。细分步数增大时，网格更贴近连续曲面。

### 1.5 双三次 Bézier 曲面片

双三次 Bézier 曲面片（Bézier patch）由 $4\times 4$ 控制点网格 $\{ \mathbf{p}_{ij} \}_{i,j=0}^{3}$ 与两个参数 $s,t\in[0,1]$ 确定。先对每一行四个控制点用参数 $t$ 做三次 Bézier 插值，得到四个中间点 $\mathbf{q}_i(t)$；再对这四个点用参数 $s$ 做一次三次 Bézier 插值，得到曲面上的点 $\mathbf{S}(s,t)$。该过程等价于张量积形式

$$
\mathbf{S}(s,t)=\mathbf{T}_s^\mathrm{T}\,B^\mathrm{T}\,P\,B\,\mathbf{T}_t,
$$

其中 $P$ 为控制点矩阵，$\mathbf{T}_s$、$\mathbf{T}_t$ 为对应参数的幂基向量。曲面在参数矩形上光滑，边界分别为四条三次 Bézier。对 $(s,t)$ 做均匀网格采样并连接三角形后，曲面片导出为光线追踪器可读的多边形网格。

## 2 程序设计与实现

### 2.1 总体架构

样条编辑系统由样条抽象层、配置解析模块、交互预览模块与网格导出模块组成。样条抽象层给出绘制、格式互转、控制点访问与编辑、以及三角网格输出等接口；曲线与曲面为两条派生分支——曲线侧实现三次 Bézier 与均匀 B 样条，曲面侧实现旋转曲面与双三次 Bézier 曲面片。解析模块读入场景描述并构造相应样条对象，同时完成屏幕坐标下的控制点与边拾取，以及按命令行选项写出 Bézier、B 样条或三角网格文件。交互预览在图形界面中调用各对象的绘制接口；导出网格由既有光线追踪器加载渲染。

样条基类接口如下：

```cpp
class Spline {
public:
  virtual void Paint(ArgParser *args) = 0;
  virtual void OutputBezier(FILE *file) = 0;
  virtual void OutputBSpline(FILE *file) = 0;
  virtual int getNumVertices() = 0;
  virtual Vec3f getVertex(int i) = 0;
  virtual void moveControlPoint(int selectedPoint, float x, float y) = 0;
  virtual void addControlPoint(int selectedPoint, float x, float y) = 0;
  virtual void deleteControlPoint(int selectedPoint) = 0;
  virtual TriangleMesh* OutputTriangles(ArgParser *args) = 0;
};
```

### 2.2 三次曲线求值与可视化

曲线对象保存控制点序列，并由具体类型给出分段数量、每段四点控制点及对应基矩阵。公共求值过程构造几何矩阵 $G$，按 $\mathbf{Q}(t)=G\cdot B\cdot\mathbf{T}$ 得到曲线上的点：

```cpp
Vec3f EvaluateCubicCurve(const Vec3f pts[4], const Matrix &basis, float t) {
  Matrix G = GeometryMatrixFromControlPoints(pts);
  Matrix GB = G * basis;
  Vec4f T(t * t * t, t * t, t, 1.0f);
  GB.Transform(T);
  return Vec3f(T.x(), T.y(), T.z());
}
```

绘制时依次画出灰色控制多边形、黄色控制点，再按曲线细分参数对各段均匀采样，以青色折线逼近样条。全局参数 $u\in[0,1]$ 先映射到段索引与局部 $t$，再调用分段求值，供旋转曲面沿轮廓采样时复用：

```cpp
float scaled = u * numSegs;
int segment = (int)scaled;
float t = scaled - segment;
return evaluateSegment(segment, t);
```

### 2.3 Bézier / B 样条互转与多控制点分段

系统给出三次 Bézier 与均匀 B 样条的基矩阵。四点互转时，由 $G_{\mathrm{dst}}=G_{\mathrm{src}}B_{\mathrm{src}}B_{\mathrm{dst}}^{-1}$ 得到目标控制点：

```cpp
Matrix G = GeometryMatrixFromControlPoints(src);
Matrix dstInv = dstBasis;
dstInv.Inverse();
Matrix Gdst = G * srcBasis * dstInv;
ControlPointsFromGeometryMatrix(Gdst, dst);
```

B 样条以滑动四点窗口取段，控制点数为 $n+3$ 时产生 $n$ 段；Bézier 要求控制点数为 $3n+1$，相邻段起点间隔三个下标：

```cpp
// B 样条：窗口每次前移一个控制点
pts[i] = vertices[segment + i];

// Bézier：段起点步长为 3
int start = segment * 3;
pts[i] = vertices[start + i];
```

### 2.4 控制点编辑

通过查询控制点个数与坐标，解析模块在屏幕上选取最近点或最近边。移动操作更新选中点的平面坐标 $(x,y)$；添加与删除则在序列中插入或移除控制点。B 样条在控制点多于四个时执行增删；Bézier 因 $3n+1$ 的段结构，增删会破坏分段对齐，因此增删操作直接返回。旋转曲面的编辑操作转交内部的二维轮廓曲线。

### 2.5 旋转曲面

旋转曲面内部保存一条轮廓曲线。导出网格时，沿轮廓方向的细分份数取为“曲线段数 × 曲线细分参数”，沿旋转方向取旋转细分参数。轮廓点 $(x,y)$ 绕 $y$ 轴映射为 $(x\cos\theta,\,y,\,x\sin\theta)$：

```cpp
static Vec3f RevolveProfilePoint(const Vec3f &profile, float theta) {
  float x = profile.x();
  float y = profile.y();
  return Vec3f(x * cosf(theta), y, x * sinf(theta));
}
```

在规则四边形网格各角点写入旋转后的三维坐标：

```cpp
int u_tess = profile_curve->numSegments() * curveTess;
int v_tess = revTess;
TriangleNet *net = new TriangleNet(u_tess, v_tess);
for (int i = 0; i <= u_tess; i++) {
  float u = (float)i / (float)u_tess;
  Vec3f profile = profile_curve->evaluateAlongCurve(u);
  for (int j = 0; j <= v_tess; j++) {
    float theta = twoPi * (float)j / (float)v_tess;
    net->SetVertex(i, j, RevolveProfilePoint(profile, theta));
  }
}
```

预览时除绘制轮廓外，还显示若干旋转圆环。

### 2.6 双三次 Bézier 曲面片

曲面片存储 $16$ 个控制点。求值时先对每行用参数 $t$ 做三次 Bézier 插值，再用参数 $s$ 在所得四点间插值：

```cpp
static Vec3f EvaluateBezierPatch(const Vec3f control[16], float s, float t) {
  const Matrix &B = GetBezierBasisMatrix();
  Vec3f rowPoints[4];
  for (int i = 0; i < 4; i++) {
    Vec3f pts[4];
    for (int j = 0; j < 4; j++)
      pts[j] = control[i * 4 + j];
    rowPoints[i] = EvaluateCubicCurve(pts, B, t);
  }
  return EvaluateCubicCurve(rowPoints, B, s);
}
```

按曲面片细分参数在 $(s,t)$ 参数域上生成三角网格，并调整三角形顶点绕序使正面朝向正确。多条样条的网格合并后写出为单一物体文件，用于茶壶等由旋转体与曲面片拼接的复合模型：

```cpp
for (int i = 0; i < getNumSplines(); i++) {
  TriangleMesh *m2 = getSpline(i)->OutputTriangles(args);
  mesh.Merge(*m2);
  delete m2;
}
```

## 3 实验结果

测试命令形如：

```text
# 圆环旋转曲面；-curve_tessellation 曲线细分，-revolution_tessellation 旋转细分，-output 导出网格
curve_editor -input <圆环轮廓场景> -curve_tessellation 4 -revolution_tessellation 10 -output <低细分圆环网格>
# Bézier 曲面片；-patch_tessellation 曲面片细分
curve_editor -input <Bézier 曲面片场景> -patch_tessellation 40 -output <高细分曲面片网格>
# 茶壶复合模型
curve_editor -input <茶壶场景> -patch_tessellation 30 -curve_tessellation 30 -revolution_tessellation 100 -output <高细分茶壶网格>
# 光线追踪渲染；-gui 交互预览，-size 图像分辨率
raytracer -input <高细分圆环渲染场景> -gui -size 300 300
# 透明花瓶；-grid 体素网格，-bounces 反弹次数，-shade_back 背面着色，
# -jittered_samples 抖动超采样，-tent_filter 帐篷滤波，-shadows 阴影
raytracer -input <透明花瓶场景> -output <透明花瓶结果> -grid 30 30 30 -size 300 300 -bounces 4 -shade_back -jittered_samples 9 -tent_filter 1.0 -shadows
```

曲线编辑器交互绘制 Bézier / B 样条的控制点、控制多边形与细分曲线，并完成四点表示互转与 B 样条控制点增删。曲面经细分导出为三角网格后，由光线追踪器在 $300\times 300$ 下渲染；提高曲线、旋转与曲面片细分参数后，表面折痕减弱、曲率过渡更平滑。

### 3.1 圆环（旋转曲面）

| 低细分（曲线 $=4$，旋转 $=10$） | 高细分（曲线 $=30$，旋转 $=60$） |
|:---:|:---:|
| ![output8_06_torus_low](OutputFiles/output8_06_torus_low.png) | ![output8_06_torus_high](OutputFiles/output8_06_torus_high.png) |
| 明显折面 | 接近光滑圆环 |

B 样条圆环轮廓绕 $y$ 轴扫掠。低细分网格呈明显折面；高细分后接近光滑圆环，说明旋转映射与规则三角剖分正确。

### 3.2 花瓶（旋转曲面）

| 低细分花瓶 | 高细分花瓶 |
|:---:|:---:|
| ![output8_07_vase_low](OutputFiles/output8_07_vase_low.png) | ![output8_07_vase_high](OutputFiles/output8_07_vase_high.png) |
| 肩部与瓶身可见棱角 | 曲率过渡更平滑 |

编辑后的轮廓曲线旋转生成花瓶。低细分时肩部与瓶身可见棱角；高细分后曲率过渡更平滑，与轮廓参数采样密度提高的预期相符。

### 3.3 双三次 Bézier 曲面片

| 细分份数 $=4$ | 细分份数 $=10$ | 细分份数 $=40$ |
|:---:|:---:|:---:|
| ![output8_08_patch_low](OutputFiles/output8_08_patch_low.png) | ![output8_08_patch_med](OutputFiles/output8_08_patch_med.png) | ![output8_08_patch_high](OutputFiles/output8_08_patch_high.png) |
| 粗网格 | 中等平滑 | 接近连续曲面 |

单一 $4\times 4$ 控制点曲面片随细分份数增加，由粗网格过渡到接近连续曲面的效果，说明张量积求值与参数域网格化正确。

### 3.4 茶壶

| 低细分茶壶 | 高细分茶壶 |
|:---:|:---:|
| ![output8_09_teapot_low](OutputFiles/output8_09_teapot_low.png) | ![output8_09_teapot_high](OutputFiles/output8_09_teapot_high.png) |
| 分片边界清晰 | 壶身、壶嘴与把手过渡更完整 |

茶壶由多条旋转轮廓与多张 Bézier 曲面片拼接。低细分时分片边界清晰；高细分后壶身、壶嘴与把手过渡更完整，网格合并导出工作正常。

### 3.5 透明花瓶与反射茶壶

| 高细分透明花瓶（折射＋阴影＋超采样） | 高细分反射茶壶 |
|:---:|:---:|
| ![output8_10](OutputFiles/output8_10.png) | ![output8_11](OutputFiles/output8_11.png) |
| 玻璃质感，折射与阴影正确 | 镜面反射映出环境 |

花瓶网格在折射与阴影下呈现玻璃质感；茶壶在镜面反射下映出环境，说明导出网格已接入既有光线追踪流程，并在间接光照下表现合理。

## 4 问题记录

最初实现的四点 Bézier 与 B 样条互转中，曲线形状与原场景不一致，采样误差明显。调试后发现均匀三次 B 样条基矩阵未进行转置，更正后问题解决。

交互编辑时，左键拖拽与中键插点常无法选中控制点或控制边。检查逻辑发现，最近距离比较中虽计算了 $d$，但没有把当前最小距离写回阈值变量，导致第一次落入容差的候选之后不再更新，导致结果错乱。补上距离更新后，控制点移动与 B 样条插点恢复正常。

圆环、花瓶等旋转曲面在预览中能画出轮廓控制点，却无法拖动或增删。原因是旋转曲面对象把控制点个数固定返回为零，循环遍历不到内部轮廓上的顶点，移动与增删也未转交轮廓曲线。将移动、添加、删除全部委托给内部轮廓曲线后，编辑与保存生效。

Bézier 曲面片导出网格后，部分光线追踪结果整片发黑。调试后发现，顶点位置正确，但规则网格按原三角形绕序使法线背向相机，而既有追踪器对背面剔除后没有正面着色。解决方法是在曲面片网格生成中翻转每个四边形剖分三角形的顶点顺序，使正面朝向相机后，曲面片与茶壶场景恢复了正常着色。

## 5 总结

以矩阵形式 $\mathbf{Q}(t)=G B \mathbf{T}$ 完成三次 Bézier 与均匀 B 样条的求值，并在四点情形下通过基矩阵变换实现两种控制点表示的互转；再以滑动窗口或 $3n+1$ 分段将曲线推广到任意长度，配合交互预览与控制点编辑。旋转曲面将轮廓绕 $y$ 轴扫掠为规则三角网格，双三次 Bézier 曲面片按张量积对 $(s,t)$ 细分，二者都导出为多边形网格并由光线追踪器渲染。提高细分参数后，圆环、花瓶、曲面片与茶壶等测试场景的几何平滑度与着色结果和连续样条曲面的预期相符。

## 附录（TODO list 记录）

- [x] 实现样条抽象层及曲线、曲面派生层次（Bézier 曲线、B 样条、旋转曲面、Bézier 曲面片），并给出绘制、互转、控制点编辑与网格输出接口
- [x] 在曲线模块中按 $\mathbf{Q}(t)=G\cdot B\cdot\mathbf{T}$ 求值，并绘制控制多边形、控制点与细分样条
- [x] 给出三次 Bézier 与均匀 B 样条基矩阵，并在四点情形下完成两种控制点表示的互转与文件导出
- [x] 将 Bézier 与 B 样条推广为局部三次分段（滑动窗口 / $3n+1$ 结构），并将全局参数映射到各段局部参数
- [x] 接入控制点拾取与移动、添加、删除；B 样条在多于四点时执行增删
- [x] 在旋转曲面中实现轮廓求值、绕 $y$ 轴映射，并按曲线与旋转细分生成规则三角网格，同时完成多网格合并导出
- [x] 在 Bézier 曲面片中实现张量积求值、参数线预览与按细分参数生成三角网格
