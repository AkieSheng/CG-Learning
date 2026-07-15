#ifndef _MESH_H_
#define _MESH_H_

#include <vector>
#include "vertex.h"
#include "pbr_material.h"
#include "matrix.h"

// 单个 glTF primitive 对应的可绘制网格
class Mesh {
public:
  Mesh();
  ~Mesh();

  // 创建 VAO/VOB/EBO，配置 4 个顶点属性
  void upload(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);
  // 绘制网格
  void draw() const;
  void destroy();

  void setMaterial(PBRMaterial *mat) { material = mat; }
  PBRMaterial *getMaterial() const { return material; }

  void setModelMatrix(const Matrix &m) { modelMatrix = m; }
  const Matrix &getModelMatrix() const { return modelMatrix; }

  void setWorldCenter(const Vec3f &c) { worldCenter = c; }
  const Vec3f &getWorldCenter() const { return worldCenter; }

  int indexCount() const { return numIndices; }

private:
  unsigned int vao;  // 顶点数组对象
  unsigned int vbo;  // 顶点缓冲对象
  unsigned int ebo;  // 索引缓冲对象
  int numIndices;  // 三角形索引数量
  PBRMaterial *material;  // 关联的材质
  Matrix modelMatrix;  // 局部坐标系到世界坐标系的变换矩阵
  Vec3f worldCenter;  // 世界空间包围中心（透明排序）
};

#endif
