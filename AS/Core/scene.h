#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include <vector>
#include "gltf_loader.h"
#include "orbit_camera.h"
#include "vectors.h"

static const int LIGHT_STRIP_COUNT = 3;

// 不可见工作室水平灯带
struct LightStrip {
  Vec3f center;
  Vec3f halfRight;   // 沿灯带长度的半轴
  Vec3f halfUp;      // 沿灯带宽度的半轴
  Vec3f normal;      // 朝向场景
  Vec3f color;       // 线性 HDR 发光色
};

// 场景
class Scene {
public:
  Scene();
  ~Scene();

  bool loadModel(const std::string &gltfPath, float aspect = 16.0f / 9.0f);
  void clear();

  const std::vector<Mesh *> &getMeshes() const;
  OrbitCamera &getCamera() { return camera; }
  const OrbitCamera &getCamera() const { return camera; }

  Vec3f getBackgroundColor() const { return backgroundColor; }
  void setBackgroundColor(const Vec3f &c) { backgroundColor = c; }

  // 场景 AABB（方向光阴影包围盒）
  void getBounds(Vec3f &bmin, Vec3f &bmax) const;

  const LightStrip *getLightStrips() const { return lightStrips; }
  bool hasLightStrips() const { return lightStripsReady; }

private:
  void destroyLightStrips();
  void createLightStrips(const Vec3f &bmin, const Vec3f &bmax);

  GltfLoader loader;  // glTF 模型加载器
  OrbitCamera camera;  // 轨道相机
  Vec3f backgroundColor;  // 背景颜色

  LightStrip lightStrips[LIGHT_STRIP_COUNT];
  bool lightStripsReady;
};

#endif
