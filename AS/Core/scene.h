#pragma once

#include <string>
#include <vector>
#include "gltf_loader.h"
#include "orbit_camera.h"
#include "vectors.h"

static int const LIGHT_STRIP_COUNT = 3;

struct LightStrip {
  Vec3f center{};
  Vec3f halfRight{};
  Vec3f halfUp{};
  Vec3f normal{};
  Vec3f color{};
};

struct Scene final {
  Scene();
  ~Scene();

  auto loadModel(std::string const& gltfPath, float aspect = 16.0f / 9.0f)
      -> bool;
  auto clear() -> void;

  auto getMeshes() const -> std::vector<Mesh*> const&;
  auto getCamera() -> OrbitCamera& { return camera; }
  auto getCamera() const -> OrbitCamera const& { return camera; }

  auto getBackgroundColor() const -> Vec3f { return backgroundColor; }
  auto setBackgroundColor(Vec3f const& c) -> void { backgroundColor = c; }

  auto getBounds(Vec3f& bmin, Vec3f& bmax) const -> void;

  auto getLightStrips() const -> LightStrip const* { return lightStrips; }
  auto hasLightStrips() const -> bool { return lightStripsReady; }

  GltfLoader loader{};
  OrbitCamera camera{};
  Vec3f backgroundColor{0.10f, 0.10f, 0.11f};

  LightStrip lightStrips[LIGHT_STRIP_COUNT]{};
  bool lightStripsReady{};

  auto destroyLightStrips() -> void;
  auto createLightStrips(Vec3f const& bmin, Vec3f const& bmax) -> void;
};
