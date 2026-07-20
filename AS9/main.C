#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <cassert>

#include "gl_headers.h"
#include "glCanvas.h"
#include "parser.h"

auto main(int argc, char* argv[]) -> int {
  char const* filename = nullptr;
  auto refresh = 0.1f;
  auto dt = 0.1f;
  auto integrator_color = 0;
  auto draw_vectors = 0;
  auto acceleration_scale = 1.0f;
  auto motion_blur = 0;

  for (auto i = 1; i < argc; i++) {
    if (!::strcmp(argv[i], "-input")) {
      i++;
      assert(i < argc);
      filename = argv[i];
    } else if (!::strcmp(argv[i], "-refresh")) {
      i++;
      assert(i < argc);
      refresh = static_cast<float>(::atof(argv[i]));
    } else if (!::strcmp(argv[i], "-dt")) {
      i++;
      assert(i < argc);
      dt = static_cast<float>(::atof(argv[i]));
    } else if (!::strcmp(argv[i], "-integrator_color")) {
      integrator_color = 1;
    } else if (!::strcmp(argv[i], "-motion_blur")) {
      motion_blur = 1;
    } else if (!::strcmp(argv[i], "-draw_vectors")) {
      draw_vectors = 1;
      i++;
      assert(i < argc);
      acceleration_scale = static_cast<float>(::atof(argv[i]));
    } else {
      ::printf("WARNING:  unknown command line argument %s\n", argv[i]);
      assert(0);
    }
  }

  assert(filename != nullptr);
  auto* parser = new Parser(filename);

  ::glutInit(&argc, argv);
  GLCanvas glcanvas;
  glcanvas.initialize(parser, refresh, dt, integrator_color, draw_vectors, acceleration_scale,
                      motion_blur);
  return 0;
}
