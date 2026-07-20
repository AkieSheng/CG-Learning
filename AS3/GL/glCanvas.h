#pragma once

#include <cstdlib>


struct SceneParser;

#define SPECULAR_FIX 0

struct GLCanvas final
{
  static void (*renderFunction)(void);
  static SceneParser* scene;
  static int mouseButton;
  static int mouseX;
  static int mouseY;

  static auto drawAxes() -> void;
  static auto display() -> void;
  static auto reshape(int w, int h) -> void;
  static auto mouse(int button, int state, int x, int y) -> void;
  static auto motion(int x, int y) -> void;
  static auto keyboard(unsigned char key, int x, int y) -> void;

  GLCanvas() { renderFunction = nullptr; }
  ~GLCanvas() {}

  auto initialize(SceneParser* _scene, void (*_renderFunction)(void)) -> void;
};
