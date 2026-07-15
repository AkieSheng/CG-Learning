#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include <vector>
#include "gltf_loader.h"
#include "orbit_camera.h"

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

private:
  GltfLoader loader;  // glTF 模型加载器
  OrbitCamera camera;  // 轨道相机
  Vec3f backgroundColor;  // 背景颜色
};

#endif
