#pragma once

#include "vectors.h"

#include <cassert>
#include <cstdio>

struct Camera;
struct Light;
struct Material;
struct Object3D;
struct Group;
struct Sphere;
struct Plane;
struct Triangle;
struct Transform;
struct Matrix;

constexpr auto MAX_PARSER_TOKEN_LENGTH = 100;

struct SceneParser final {
  SceneParser(char const* filename);
  ~SceneParser();

  auto getCamera() const -> Camera* { return camera; }
  auto getBackgroundColor() const -> Vec3f { return background_color; }
  auto getAmbientLight() const -> Vec3f { return ambient_light; }
  auto getNumLights() const -> int { return num_lights; }
  auto getLight(int i) const -> Light* {
    assert(i >= 0 && i < num_lights);
    return lights[i];
  }
  auto getNumMaterials() const -> int { return num_materials; }
  auto getMaterial(int i) const -> Material* {
    assert(i >= 0 && i < num_materials);
    return materials[i];
  }
  auto getGroup() const -> Group* { return group; }

  SceneParser() { assert(0); }

  auto parseFile() -> void;
  auto parseOrthographicCamera() -> void;
  auto parsePerspectiveCamera() -> void;
  auto parseBackground() -> void;
  auto parseLights() -> void;
  auto parseDirectionalLight() -> Light*;
  auto parsePointLight() -> Light*;
  auto parseMaterials() -> void;
  auto parsePhongMaterial() -> Material*;
  auto parseCheckerboard(int count) -> Material*;
  auto parseNoise(int count) -> Material*;
  auto parseMarble(int count) -> Material*;
  auto parseWood(int count) -> Material*;

  auto parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) -> Object3D*;
  auto parseGroup() -> Group*;
  auto parseSphere() -> Sphere*;
  auto parsePlane() -> Plane*;
  auto parseTriangle() -> Triangle*;
  auto parseTriangleMesh() -> Group*;
  auto parseTransform() -> Transform*;
  auto parseMatrixHelper(Matrix& matrix, char token[MAX_PARSER_TOKEN_LENGTH]) -> void;

  auto getToken(char token[MAX_PARSER_TOKEN_LENGTH]) -> int;
  auto readVec3f() -> Vec3f;
  auto readVec2f() -> Vec2f;
  auto readFloat() -> float;
  auto readInt() -> int;

  FILE* file{};
  Camera* camera{};
  Vec3f background_color{};
  Vec3f ambient_light{};
  int num_lights{};
  Light** lights{};
  int num_materials{};
  Material** materials{};
  Material* current_material{};
  Group* group{};
};
