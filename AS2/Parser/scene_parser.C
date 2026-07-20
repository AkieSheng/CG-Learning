#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cmath>

#include "scene_parser.h"
#include "matrix.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "object3d.h"
#include "group.h"
#include "sphere.h"
#include "plane.h"
#include "triangle.h"
#include "transform.h"

constexpr auto DegreesToRadians(float x) -> float {
  return (static_cast<float>(M_PI) * x) / 180.0f;
}

SceneParser::SceneParser(char const* filename) {
  group = nullptr;
  camera = nullptr;
  background_color = Vec3f(0.5, 0.5, 0.5);
  ambient_light = Vec3f(0, 0, 0);
  num_lights = 0;
  lights = nullptr;
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

  if (num_lights == 0) {
    ::printf("WARNING:  No lights specified\n");
    ambient_light = Vec3f(1, 1, 1);
  }
}

SceneParser::~SceneParser() {
  if (group != nullptr) {
    delete group;
  }
  if (camera != nullptr) {
    delete camera;
  }
  for (auto i = 0; i < num_materials; i++) {
    delete materials[i];
  }
  delete[] materials;
  for (auto i = 0; i < num_lights; i++) {
    delete lights[i];
  }
  delete[] lights;
}

auto SceneParser::parseFile() -> void {
  char token[MAX_PARSER_TOKEN_LENGTH];
  while (getToken(token)) {
    if (!::strcmp(token, "OrthographicCamera")) {
      parseOrthographicCamera();
    } else if (!::strcmp(token, "PerspectiveCamera")) {
      parsePerspectiveCamera();
    } else if (!::strcmp(token, "Background")) {
      parseBackground();
    } else if (!::strcmp(token, "Lights")) {
      parseLights();
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

auto SceneParser::parseOrthographicCamera() -> void {
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

auto SceneParser::parsePerspectiveCamera() -> void {
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
  assert(!::strcmp(token, "angle"));
  auto angle_degrees = readFloat();
  auto angle_radians = DegreesToRadians(angle_degrees);
  getToken(token);
  assert(!::strcmp(token, "}"));
  camera = new PerspectiveCamera(center, direction, up, angle_radians);
}

auto SceneParser::parseBackground() -> void {
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  while (1) {
    getToken(token);
    if (!::strcmp(token, "}")) {
      break;
    } else if (!::strcmp(token, "color")) {
      background_color = readVec3f();
    } else if (!::strcmp(token, "ambientLight")) {
      ambient_light = readVec3f();
    } else {
      ::printf("Unknown token in parseBackground: '%s'\n", token);
      assert(0);
    }
  }
}

auto SceneParser::parseLights() -> void {
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  getToken(token);
  assert(!::strcmp(token, "numLights"));
  num_lights = readInt();
  lights = new Light*[num_lights];
  auto count = 0;
  while (num_lights > count) {
    getToken(token);
    if (!::strcmp(token, "DirectionalLight")) {
      lights[count] = parseDirectionalLight();
    } else {
      ::printf("Unknown token in parseLight: '%s'\n", token);
      ::exit(0);
    }
    count++;
  }
  getToken(token);
  assert(!::strcmp(token, "}"));
}

auto SceneParser::parseDirectionalLight() -> Light* {
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  getToken(token);
  assert(!::strcmp(token, "direction"));
  auto direction = readVec3f();
  getToken(token);
  assert(!::strcmp(token, "color"));
  auto color = readVec3f();
  getToken(token);
  assert(!::strcmp(token, "}"));
  return new DirectionalLight(direction, color);
}

auto SceneParser::parseMaterials() -> void {
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
    if (!::strcmp(token, "Material")) {
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

auto SceneParser::parseMaterial() -> Material* {
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

auto SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) -> Object3D* {
  Object3D* answer = nullptr;
  if (!::strcmp(token, "Group")) {
    answer = static_cast<Object3D*>(parseGroup());
  } else if (!::strcmp(token, "Sphere")) {
    answer = static_cast<Object3D*>(parseSphere());
  } else if (!::strcmp(token, "Plane")) {
    answer = static_cast<Object3D*>(parsePlane());
  } else if (!::strcmp(token, "Triangle")) {
    answer = static_cast<Object3D*>(parseTriangle());
  } else if (!::strcmp(token, "TriangleMesh")) {
    answer = static_cast<Object3D*>(parseTriangleMesh());
  } else if (!::strcmp(token, "Transform")) {
    answer = static_cast<Object3D*>(parseTransform());
  } else {
    ::printf("Unknown token in parseObject: '%s'\n", token);
    ::exit(0);
  }
  return answer;
}

auto SceneParser::parseGroup() -> Group* {
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
      assert(index >= 0 && index <= getNumMaterials());
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

auto SceneParser::parseSphere() -> Sphere* {
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

auto SceneParser::parsePlane() -> Plane* {
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  getToken(token);
  assert(!::strcmp(token, "normal"));
  auto normal = readVec3f();
  getToken(token);
  assert(!::strcmp(token, "offset"));
  auto offset = readFloat();
  getToken(token);
  assert(!::strcmp(token, "}"));
  assert(current_material != nullptr);
  return new Plane(normal, offset, current_material);
}

auto SceneParser::parseTriangle() -> Triangle* {
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  getToken(token);
  assert(!::strcmp(token, "vertex0"));
  auto v0 = readVec3f();
  getToken(token);
  assert(!::strcmp(token, "vertex1"));
  auto v1 = readVec3f();
  getToken(token);
  assert(!::strcmp(token, "vertex2"));
  auto v2 = readVec3f();
  getToken(token);
  assert(!::strcmp(token, "}"));
  assert(current_material != nullptr);
  return new Triangle(v0, v1, v2, current_material);
}

auto SceneParser::parseTriangleMesh() -> Group* {
  char token[MAX_PARSER_TOKEN_LENGTH];
  char filename[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  getToken(token);
  assert(!::strcmp(token, "obj_file"));
  getToken(filename);
  getToken(token);
  assert(!::strcmp(token, "}"));
  auto const* ext = &filename[::strlen(filename) - 4];
  assert(!::strcmp(ext, ".obj"));
  auto* mesh_file = ::fopen(filename, "r");
  assert(mesh_file != nullptr);
  auto vcount = 0;
  auto fcount = 0;
  while (1) {
    auto c = ::fgetc(mesh_file);
    if (c == EOF) {
      break;
    } else if (c == 'v') {
      assert(fcount == 0);
      float v0{};
      float v1{};
      float v2{};
      ::fscanf(mesh_file, "%f %f %f", &v0, &v1, &v2);
      vcount++;
    } else if (c == 'f') {
      int f0{};
      int f1{};
      int f2{};
      ::fscanf(mesh_file, "%d %d %d", &f0, &f1, &f2);
      fcount++;
    }
  }
  ::fclose(mesh_file);
  auto* verts = new Vec3f[vcount];
  auto* answer = new Group(fcount);
  mesh_file = ::fopen(filename, "r");
  assert(mesh_file != nullptr);
  auto new_vcount = 0;
  auto new_fcount = 0;
  while (1) {
    auto c = ::fgetc(mesh_file);
    if (c == EOF) {
      break;
    } else if (c == 'v') {
      assert(new_fcount == 0);
      float v0{};
      float v1{};
      float v2{};
      ::fscanf(mesh_file, "%f %f %f", &v0, &v1, &v2);
      verts[new_vcount] = Vec3f(v0, v1, v2);
      new_vcount++;
    } else if (c == 'f') {
      assert(vcount == new_vcount);
      int f0{};
      int f1{};
      int f2{};
      ::fscanf(mesh_file, "%d %d %d", &f0, &f1, &f2);
      assert(f0 > 0 && f0 <= vcount);
      assert(f1 > 0 && f1 <= vcount);
      assert(f2 > 0 && f2 <= vcount);
      assert(current_material != nullptr);
      auto* t = new Triangle(verts[f0 - 1], verts[f1 - 1], verts[f2 - 1], current_material);
      answer->addObject(new_fcount, t);
      new_fcount++;
    }
  }
  delete[] verts;
  assert(fcount == new_fcount);
  assert(vcount == new_vcount);
  ::fclose(mesh_file);
  return answer;
}

auto SceneParser::parseTransform() -> Transform* {
  char token[MAX_PARSER_TOKEN_LENGTH];
  Matrix matrix;
  matrix.SetToIdentity();
  Object3D* object = nullptr;
  getToken(token);
  assert(!::strcmp(token, "{"));
  getToken(token);
  while (1) {
    if (!::strcmp(token, "Scale")) {
      matrix *= Matrix::MakeScale(readVec3f());
    } else if (!::strcmp(token, "UniformScale")) {
      auto s = readFloat();
      matrix *= Matrix::MakeScale(Vec3f(s, s, s));
    } else if (!::strcmp(token, "Translate")) {
      matrix *= Matrix::MakeTranslation(readVec3f());
    } else if (!::strcmp(token, "XRotate")) {
      matrix *= Matrix::MakeXRotation(DegreesToRadians(readFloat()));
    } else if (!::strcmp(token, "YRotate")) {
      matrix *= Matrix::MakeYRotation(DegreesToRadians(readFloat()));
    } else if (!::strcmp(token, "ZRotate")) {
      matrix *= Matrix::MakeZRotation(DegreesToRadians(readFloat()));
    } else if (!::strcmp(token, "Rotate")) {
      getToken(token);
      assert(!::strcmp(token, "{"));
      auto axis = readVec3f();
      auto degrees = readFloat();
      matrix *= Matrix::MakeAxisRotation(axis, DegreesToRadians(degrees));
      getToken(token);
      assert(!::strcmp(token, "}"));
    } else if (!::strcmp(token, "Matrix")) {
      Matrix matrix2;
      matrix2.SetToIdentity();
      getToken(token);
      assert(!::strcmp(token, "{"));
      for (auto j = 0; j < 4; j++) {
        for (auto i = 0; i < 4; i++) {
          auto v = readFloat();
          matrix2.Set(i, j, v);
        }
      }
      getToken(token);
      assert(!::strcmp(token, "}"));
      matrix = matrix2 * matrix;
    } else {
      object = parseObject(token);
      break;
    }
    getToken(token);
  }
  assert(object != nullptr);
  getToken(token);
  assert(!::strcmp(token, "}"));
  return new Transform(matrix, object);
}

auto SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) -> int {
  assert(file != nullptr);
  auto success = ::fscanf(file, "%s ", token);
  if (success == EOF) {
    token[0] = '\0';
    return 0;
  }
  return 1;
}

auto SceneParser::readVec3f() -> Vec3f {
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

auto SceneParser::readVec2f() -> Vec2f {
  float u{};
  float v{};
  auto count = ::fscanf(file, "%f %f", &u, &v);
  if (count != 2) {
    ::printf("Error trying to read 2 floats to make a Vec2f\n");
    assert(0);
  }
  return Vec2f(u, v);
}

auto SceneParser::readFloat() -> float {
  float answer{};
  auto count = ::fscanf(file, "%f", &answer);
  if (count != 1) {
    ::printf("Error trying to read 1 float\n");
    assert(0);
  }
  return answer;
}

auto SceneParser::readInt() -> int {
  int answer{};
  auto count = ::fscanf(file, "%d", &answer);
  if (count != 1) {
    ::printf("Error trying to read 1 int\n");
    assert(0);
  }
  return answer;
}
