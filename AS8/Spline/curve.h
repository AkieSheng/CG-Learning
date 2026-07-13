#ifndef _CURVE_H_
#define _CURVE_H_

#include "spline.h"
#include "matrix.h"

class ArgParser;

#define DEBUG_CURVE 0

// 曲线基层
class Curve : public Spline {

public:
  Curve(int _num_vertices);
  virtual ~Curve();

  int getNumVertices() { return num_vertices; }
  Vec3f getVertex(int i);
  void set(int i, Vec3f v);

  void moveControlPoint(int selectedPoint, float x, float y);
  void addControlPoint(int selectedPoint, float x, float y);
  void deleteControlPoint(int selectedPoint);

  void Paint(ArgParser *args);

  TriangleMesh* OutputTriangles(ArgParser *args);

  // 对旋转曲面沿轮廓采样
  int numSegments() const; // 获取分段数量
  Vec3f evaluateAlongCurve(float u) const; // 沿曲线参数 u 求值

protected:
  virtual int getNumSegments() const = 0; // 获取分段数量
  virtual void getSegmentControlPoints(int segment, Vec3f pts[4]) const = 0; // 获取分段控制点
  virtual bool allowAddControlPoints() const = 0; // 是否允许添加控制点
  virtual bool allowDeleteControlPoints() const = 0; // 是否允许删除控制点

  Vec3f evaluateSegment(int segment, float t) const; // 分段求值
  virtual const Matrix &getSegmentBasis() const = 0; // 获取分段基矩阵

  void writeControlPoints(FILE *file, const char *type) const; // 输出控制点
  void insertControlPoint(int index, Vec3f v); // 插入控制点
  void removeControlPoint(int index); // 删除控制点

  int num_vertices;  // 控制点数量
  Vec3f *vertices;  // 控制点
};

Matrix GetBezierBasisMatrix();  // 三次贝塞尔基矩阵
Matrix GetBSplineBasisMatrix();  // 三次均匀B样条基矩阵

Matrix GeometryMatrixFromControlPoints(const Vec3f pts[4]); // 从控制点获取控制点矩阵
void ControlPointsFromGeometryMatrix(const Matrix &G, Vec3f pts[4]); // 从控制点矩阵获取控制点

Vec3f EvaluateCubicCurve(const Vec3f pts[4], const Matrix &basis, float t); // 三次样条曲线求值

// 控制点转换
void ConvertBezierControlPointsToBSpline(const Vec3f bezier[4], Vec3f bspline[4]);
void ConvertBSplineControlPointsToBezier(const Vec3f bspline[4], Vec3f bezier[4]);

#if DEBUG_CURVE
// 在 Bezier/BSpline 互转时打印采样对比
void DebugVerifyCurveConversion(const Vec3f src[4], const Vec3f dst[4],
                                const Matrix &srcBasis, const Matrix &dstBasis,
                                const char *label);
#endif

#endif
