#pragma once

#include "vectors.h"
#include <cassert>
#include <cstdio>

struct Camera;
struct Material;
struct Object3D;
struct Group;
struct Sphere;

constexpr auto MAX_PARSER_TOKEN_LENGTH = 100;

struct SceneParser final {
  SceneParser(char const* filename);
  ~SceneParser();

  auto getCamera() const -> Camera* { return camera; }
  auto getBackgroundColor() const -> Vec3f { return background_color; }
  auto getNumMaterials() const -> int { return num_materials; }
  auto getMaterial(int i) const -> Material* {
    assert((i >= 0) && (i < num_materials));
    return materials[i];
  }
  auto getGroup() const -> Group* { return group; }

  SceneParser() { assert(0); }

  auto parseFile() -> void;
  auto parseOrthographicCamera() -> void;
  auto parseBackground() -> void;
  auto parseMaterials() -> void;
  auto parseMaterial() -> Material*;

  auto parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) -> Object3D*;
  auto parseGroup() -> Group*;
  auto parseSphere() -> Sphere*;

  auto getToken(char token[MAX_PARSER_TOKEN_LENGTH]) -> int;
  auto readVec3f() -> Vec3f;
  auto readVec2f() -> Vec2f;
  auto readFloat() -> float;
  auto readInt() -> int;

  FILE* file{};
  Camera* camera{};
  Vec3f background_color{};
  int num_materials{};
  Material** materials{};
  Material* current_material{};
  Group* group{};
};
