#ifndef _VERTEX_H_
#define _VERTEX_H_

#include "vectors.h"

// glTF 顶点属性布局（参考 VAO 顶点缓冲格式）
struct Vertex {
  Vec3f position;  // 位置
  Vec3f normal;  // 法线
  Vec4f tangent;   // xyz = tangent, w = bitangent 方向
  Vec2f texCoord0;  // 首组 UV 坐标
};

#endif
