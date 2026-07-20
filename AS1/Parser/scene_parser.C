#include "scene_parser.h"

#include "camera.h"
#include "group.h"
#include "material.h"
#include "matrix.h"
#include "object3d.h"
#include "sphere.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

SceneParser::SceneParser(char const* filename)
{
  group = nullptr;
  camera = nullptr;
  background_color = Vec3f(0.5, 0.5, 0.5);
  num_materials = 0;
  materials = nullptr;
  current_material = nullptr;

  assert(filename != nullptr);
  auto const* ext = &filename[::strlen(filename) - 4];
  assert(!::strcmp(ext, ".txt"));
  file = ::fopen(filename, "r");
  assert(file != nullptr);
  parseFile();
  ::fclose(file);
  file = nullptr;
}

SceneParser::~SceneParser()
{
  if (group != nullptr)
    delete group;
  if (camera != nullptr)
    delete camera;
  for (auto i = 0; i < num_materials; i++) {
    delete materials[i];
  }
  delete[] materials;
}

auto SceneParser::parseFile() -> void
{
  char token[MAX_PARSER_TOKEN_LENGTH];
  while (getToken(token)) {
    if (!::strcmp(token, "OrthographicCamera")) {
      parseOrthographicCamera();
    } else if (!::strcmp(token, "Background")) {
      parseBackground();
    } else if (!::strcmp(token, "Materials")) {
      parseMaterials();
    } else if (!::strcmp(token, "Group")) {
      group = parseGroup();
    } else {
      ::printf("Unknown token in parseFile: '%s'\n", token);
      ::exit(0);
    }
  }
}

auto SceneParser::parseOrthographicCamera() -> void
{
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  getToken(token);
  assert(!::strcmp(token, "center"));
  auto center = readVec3f();
  getToken(token);
  assert(!::strcmp(token, "direction"));
  auto direction = readVec3f();
  getToken(token);
  assert(!::strcmp(token, "up"));
  auto up = readVec3f();
  getToken(token);
  assert(!::strcmp(token, "size"));
  auto size = readFloat();
  getToken(token);
  assert(!::strcmp(token, "}"));
  camera = new OrthographicCamera(center, direction, up, size);
}

auto SceneParser::parseBackground() -> void
{
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  while (1) {
    getToken(token);
    if (!::strcmp(token, "}")) {
      break;
    } else if (!::strcmp(token, "color")) {
      background_color = readVec3f();
    } else {
      ::printf("Unknown token in parseBackground: '%s'\n", token);
      assert(0);
    }
  }
}

auto SceneParser::parseMaterials() -> void
{
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  getToken(token);
  assert(!::strcmp(token, "numMaterials"));
  num_materials = readInt();
  materials = new Material*[num_materials];
  auto count = 0;
  while (num_materials > count) {
    getToken(token);
    if (!::strcmp(token, "Material") || !::strcmp(token, "PhongMaterial")) {
      materials[count] = parseMaterial();
    } else {
      ::printf("Unknown token in parseMaterial: '%s'\n", token);
      ::exit(0);
    }
    count++;
  }
  getToken(token);
  assert(!::strcmp(token, "}"));
}

auto SceneParser::parseMaterial() -> Material*
{
  char token[MAX_PARSER_TOKEN_LENGTH];
  auto diffuseColor = Vec3f(1, 1, 1);
  getToken(token);
  assert(!::strcmp(token, "{"));
  while (1) {
    getToken(token);
    if (!::strcmp(token, "diffuseColor")) {
      diffuseColor = readVec3f();
    } else {
      assert(!::strcmp(token, "}"));
      break;
    }
  }
  auto* answer = new Material(diffuseColor);
  return answer;
}

auto SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) -> Object3D*
{
  Object3D* answer = nullptr;
  if (!::strcmp(token, "Group")) {
    answer = static_cast<Object3D*>(parseGroup());
  } else if (!::strcmp(token, "Sphere")) {
    answer = static_cast<Object3D*>(parseSphere());
  } else {
    ::printf("Unknown token in parseObject: '%s'\n", token);
    ::exit(0);
  }
  return answer;
}

auto SceneParser::parseGroup() -> Group*
{
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));

  getToken(token);
  assert(!::strcmp(token, "numObjects"));
  auto num_objects = readInt();

  auto* answer = new Group(num_objects);

  auto count = 0;
  while (num_objects > count) {
    getToken(token);
    if (!::strcmp(token, "MaterialIndex")) {
      auto index = readInt();
      assert((index >= 0) && (index <= getNumMaterials()));
      current_material = getMaterial(index);
    } else {
      auto* object = parseObject(token);
      assert(object != nullptr);
      answer->addObject(count, object);
      count++;
    }
  }
  getToken(token);
  assert(!::strcmp(token, "}"));

  return answer;
}

auto SceneParser::parseSphere() -> Sphere*
{
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  getToken(token);
  assert(!::strcmp(token, "center"));
  auto center = readVec3f();
  getToken(token);
  assert(!::strcmp(token, "radius"));
  auto radius = readFloat();
  getToken(token);
  assert(!::strcmp(token, "}"));
  assert(current_material != nullptr);
  return new Sphere(center, radius, current_material);
}

auto SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) -> int
{
  assert(file != nullptr);
  auto success = ::fscanf(file, "%s ", token);
  if (success == EOF) {
    token[0] = '\0';
    return 0;
  }
  return 1;
}

auto SceneParser::readVec3f() -> Vec3f
{
  float x{};
  float y{};
  float z{};
  auto count = ::fscanf(file, "%f %f %f", &x, &y, &z);
  if (count != 3) {
    ::printf("Error trying to read 3 floats to make a Vec3f\n");
    assert(0);
  }
  return Vec3f(x, y, z);
}

auto SceneParser::readVec2f() -> Vec2f
{
  float u{};
  float v{};
  auto count = ::fscanf(file, "%f %f", &u, &v);
  if (count != 2) {
    ::printf("Error trying to read 2 floats to make a Vec2f\n");
    assert(0);
  }
  return Vec2f(u, v);
}

auto SceneParser::readFloat() -> float
{
  float answer{};
  auto count = ::fscanf(file, "%f", &answer);
  if (count != 1) {
    ::printf("Error trying to read 1 float\n");
    assert(0);
  }
  return answer;
}

auto SceneParser::readInt() -> int
{
  int answer{};
  auto count = ::fscanf(file, "%d", &answer);
  if (count != 1) {
    ::printf("Error trying to read 1 int\n");
    assert(0);
  }
  return answer;
}
