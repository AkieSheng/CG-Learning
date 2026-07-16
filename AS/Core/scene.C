#include "scene.h"
#include <stdio.h>
#include <math.h>

Scene::Scene()
  : backgroundColor(0.10f, 0.10f, 0.11f),
    lightStripsReady(false) {
  for (int i = 0; i < LIGHT_STRIP_COUNT; i++) {
    lightStrips[i].center = Vec3f(0, 0, 0);
    lightStrips[i].halfRight = Vec3f(1, 0, 0);
    lightStrips[i].halfUp = Vec3f(0, 0, 0.05f);
    lightStrips[i].normal = Vec3f(0, -1, 0);
    lightStrips[i].color = Vec3f(0, 0, 0);
  }
}

Scene::~Scene() {
  clear();
}

bool Scene::loadModel(const std::string &gltfPath, float aspect) {
  clear();
  if (!loader.load(gltfPath)) {
    fprintf(stderr, "Scene: failed to load model %s\n", gltfPath.c_str());
    return false;
  }

  Vec3f bmin, bmax;
  loader.getBounds(bmin, bmax);
  camera.setAspect(aspect);
  camera.frameBounds(bmin, bmax);
  createLightStrips(bmin, bmax);
  fprintf(stderr,
          "Scene: bounds [%.3f,%.3f,%.3f] - [%.3f,%.3f,%.3f], camera dist=%.3f near=%.3f far=%.3f\n",
          bmin.x(), bmin.y(), bmin.z(), bmax.x(), bmax.y(), bmax.z(),
          camera.getDistance(), camera.getNear(), camera.getFar());
  return true;
}

void Scene::destroyLightStrips() {
  lightStripsReady = false;
}

void Scene::clear() {
  destroyLightStrips();
  loader.destroy();
}

void Scene::createLightStrips(const Vec3f &bmin, const Vec3f &bmax) {
  destroyLightStrips();

  Vec3f extent = bmax - bmin;
  float sizeX = extent.x();
  float sizeY = extent.y();
  float sizeZ = extent.z();
  if (sizeX < 0.01f) sizeX = 1.0f;
  if (sizeY < 0.01f) sizeY = 1.0f;
  if (sizeZ < 0.01f) sizeZ = 1.0f;

  float cx = 0.5f * (bmin.x() + bmax.x());
  float cy = 0.5f * (bmin.y() + bmax.y());
  float cz = 0.5f * (bmin.z() + bmax.z());
  float radius = sizeX;
  if (sizeZ > radius) radius = sizeZ;
  if (sizeY > radius) radius = sizeY;

  // 相机放在 +Z 侧：灯带放在上方偏 -Z
  float length = sizeX * 1.25f;
  float width = radius * 0.12f;
  if (width < 0.05f) width = 0.05f;
  float height = bmax.y() + radius * 0.35f;

  // 三条灯带覆盖背面上方区域
  float zBack = bmin.z() - sizeZ * 0.08f;
  float zNearBack = cz - sizeZ * 0.15f;
  float zs[LIGHT_STRIP_COUNT] = {
    zBack,
    0.5f * (zBack + zNearBack),
    zNearBack
  };

  // 法线朝向模型中心偏下前方：照亮顶面与背面
  Vec3f stripColor(14.0f, 13.5f, 12.6f);

  for (int i = 0; i < LIGHT_STRIP_COUNT; i++) {
    Vec3f center(cx, height, zs[i]);
    Vec3f toModel = Vec3f(cx, cy, cz) - center;
    toModel.Normalize();
    // 略压低，保证以顶光为主、背光为辅
    Vec3f n = toModel + Vec3f(0.0f, -0.35f, 0.0f);
    n.Normalize();

    lightStrips[i].center = center;
    lightStrips[i].halfRight = Vec3f(0.5f * length, 0.0f, 0.0f);
    lightStrips[i].halfUp = Vec3f(0.0f, 0.0f, 0.5f * width);
    lightStrips[i].normal = n;
    lightStrips[i].color = stripColor;
  }

  lightStripsReady = true;
  fprintf(stderr,
          "Scene: %d invisible fill strips (top/back), y=%.3f, length=%.3f\n",
          LIGHT_STRIP_COUNT, height, length);
}

const std::vector<Mesh *> &Scene::getMeshes() const {
  return loader.getMeshes();
}

void Scene::getBounds(Vec3f &bmin, Vec3f &bmax) const {
  loader.getBounds(bmin, bmax);
}
