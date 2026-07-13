#include <stdio.h>
#include <assert.h>
using namespace std;

#include "arg_parser.h"
#include "gl_headers.h"
#include "glCanvas.h"
#include "spline_parser.h"

// ====================================================================
// ====================================================================

int main(int argc, char *argv[]) {

  ArgParser *args = new ArgParser(argc,argv);
  SplineParser* splines = new SplineParser(args->input_file);

  if (args->gui) {
    glutInit(&argc, argv);
    GLCanvas glcanvas;
    glcanvas.initialize(args,splines);
  }

  splines->SaveBezier(args);
  splines->SaveBSpline(args);
  splines->SaveTriangles(args);

  delete args;
  delete splines;
  return 0;
}

// ====================================================================
// ====================================================================


