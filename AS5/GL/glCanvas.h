#pragma once

#include <cstdlib>

struct SceneParser;
struct Grid;

#define SPECULAR_FIX 0

struct GLCanvas final {
  GLCanvas() {
    renderFunction = nullptr;
    traceRayFunction = nullptr;
  }
  ~GLCanvas() {}

  auto initialize(SceneParser* _scene,
                  void (*_renderFunction)(),
                  void (*_traceRayFunction)(float, float),
                  Grid* _grid, bool _visualize_grid) -> void;

private:
  static void (*renderFunction)();
  static void (*traceRayFunction)(float, float);
  static SceneParser* scene;
  static Grid* grid;
  static bool visualize_grid;
  static int visualize_grid_march;

  static auto drawAxes() -> void;

  static int mouseButton;
  static int mouseX;
  static int mouseY;

  static auto display() -> void;
  static auto reshape(int w, int h) -> void;
  static auto mouse(int button, int state, int x, int y) -> void;
  static auto motion(int x, int y) -> void;
  static auto keyboard(unsigned char key, int x, int y) -> void;
};
