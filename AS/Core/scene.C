#include "scene.h"
#include <stdio.h>

Scene::Scene() : backgroundColor(0.10f, 0.10f, 0.11f) {}

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
  fprintf(stderr, "Scene: bounds [%.3f,%.3f,%.3f] - [%.3f,%.3f,%.3f], camera dist=%.3f\n",
          bmin.x(), bmin.y(), bmin.z(), bmax.x(), bmax.y(), bmax.z(),
          camera.getDistance());
  return true;
}

void Scene::clear() {
  loader.destroy();
}

const std::vector<Mesh *> &Scene::getMeshes() const {
  return loader.getMeshes();
}
