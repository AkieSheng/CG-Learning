#pragma once

#include <string>
#include "scene.h"
#include "renderer.h"

struct Application final {
  Application();
  ~Application();

  auto initialize(int argc, char** argv, std::string const& modelPath) -> bool;
  auto run() -> void;

  static auto displayCallback() -> void;
  static auto reshapeCallback(int w, int h) -> void;
  static auto keyboardCallback(unsigned char key, int x, int y) -> void;
  static auto mouseButtonCallback(int button, int state, int x, int y) -> void;
  static auto mouseMotionCallback(int x, int y) -> void;

  static Application* instance;

  Scene scene{};
  Renderer renderer{};
  std::string modelPath{};

  int windowWidth{1280};
  int windowHeight{720};
  int mouseX{};
  int mouseY{};
  int mouseButton{-1};
};
