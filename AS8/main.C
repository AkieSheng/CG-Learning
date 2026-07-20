#include <cstdio>
#include <cassert>

#include "arg_parser.h"
#include "gl_headers.h"
#include "glCanvas.h"
#include "spline_parser.h"

auto main(int argc, char* argv[]) -> int
{
  auto* args = new ArgParser(argc, argv);
  auto* splines = new SplineParser(args->input_file);

  if (args->gui)
  {
    ::glutInit(&argc, argv);
    GLCanvas glcanvas;
    glcanvas.initialize(args, splines);
  }

  splines->SaveBezier(args);
  splines->SaveBSpline(args);
  splines->SaveTriangles(args);

  delete args;
  delete splines;
  return 0;
}
