#pragma once

#include <cstdlib>

struct ArgParser;
struct SplineParser;
struct Spline;

struct GLCanvas final {
  GLCanvas()
  { }
  ~GLCanvas()
  { }

  static auto initialize(ArgParser* args, SplineParser* splines) -> void;

  static ArgParser* args;
  static SplineParser* splines;
  static int width;
  static int height;
  static float size;
  static Spline* selected_spline;
  static int selected_control_point;

  static auto display() -> void;
  static auto reshape(int w, int h) -> void;
  static auto mouse(int button, int state, int x, int y) -> void;
  static auto motion(int x, int y) -> void;
  static auto keyboard(unsigned char key, int x, int y) -> void;
  static auto mouseToScreen(int i, int j, float& x, float& y, float& epsilon) -> void;
};
