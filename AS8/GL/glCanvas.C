#include "glCanvas.h"
#include "gl_headers.h"
#include "arg_parser.h"
#include "spline.h"
#include "spline_parser.h"

ArgParser* GLCanvas::args = nullptr;
SplineParser* GLCanvas::splines = nullptr;
int GLCanvas::width = 300;
int GLCanvas::height = 300;
float GLCanvas::size = 10;
Spline* GLCanvas::selected_spline;
int GLCanvas::selected_control_point;

constexpr auto PIXEL_EPSILON = 10;

auto GLCanvas::initialize(ArgParser* _args, SplineParser* _splines) -> void
{
  args = _args;
  splines = _splines;

  ::glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
  ::glutInitWindowSize(width, height);
  ::glutInitWindowPosition(100, 100);
  ::glutCreateWindow("Curve Editor");

  ::glutMouseFunc(mouse);
  ::glutMotionFunc(motion);
  ::glutDisplayFunc(display);
  ::glutReshapeFunc(reshape);
  ::glutKeyboardFunc(keyboard);

  ::glutMainLoop();
}

auto GLCanvas::display() -> void
{
  ::glClearColor(0, 0, 0, 1.0);
  ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ::glMatrixMode(GL_MODELVIEW);
  ::glLoadIdentity();
  ::gluLookAt(0, 0, 10, 0, 0, 0, 0, 1, 0);

  for (auto i = 0; i < splines->getNumSplines(); i++) {
    splines->getSpline(i)->Paint(args);
  }

  ::glutSwapBuffers();
}

auto GLCanvas::reshape(int w, int h) -> void
{
  ::glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));

  width = w;
  height = h;

  ::glMatrixMode(GL_PROJECTION);
  ::glLoadIdentity();
  auto horiz = size / 2.0f;
  auto vert = horiz * height / static_cast<float>(width);
  ::glOrtho(-horiz, horiz, -vert, vert, 0.1, 1000.0);
}

auto GLCanvas::mouseToScreen(int i, int j, float& x, float& y, float& epsilon) -> void
{
  x = ((i / static_cast<float>(width)) - 0.5f) * size;
  y = -((j / static_cast<float>(height)) - 0.5f) * size * height / static_cast<float>(width);
  epsilon = PIXEL_EPSILON * size / static_cast<float>(width);
}

auto GLCanvas::mouse(int button, int state, int i, int j) -> void
{
  if (state == 1)
  {
    selected_spline = nullptr;
    return;
  }

  float x{};
  float y{};
  float epsilon{};
  mouseToScreen(i, j, x, y, epsilon);

  if (button == GLUT_LEFT_BUTTON)
  {
    Spline* s{};
    int pt{};
    splines->Pick(x, y, epsilon, s, pt);
    if (s == nullptr)
    {
      return;
    }
    s->moveControlPoint(pt, x, y);
    selected_spline = s;
    selected_control_point = pt;
  }

  if (button == GLUT_MIDDLE_BUTTON)
  {
    Spline* s{};
    int pt{};
    splines->PickEdge(x, y, epsilon, s, pt);
    if (s == nullptr)
    {
      return;
    }
    s->addControlPoint(pt, x, y);
    selected_spline = s;
    selected_control_point = pt;
  }

  if (button == GLUT_RIGHT_BUTTON)
  {
    Spline* s{};
    int pt{};
    splines->Pick(x, y, epsilon, s, pt);
    if (s == nullptr)
    {
      return;
    }
    s->deleteControlPoint(pt);
  }

  ::glutPostRedisplay();
}

auto GLCanvas::motion(int i, int j) -> void
{
  if (selected_spline == nullptr)
  {
    return;
  }
  float x{};
  float y{};
  float epsilon{};
  mouseToScreen(i, j, x, y, epsilon);
  selected_spline->moveControlPoint(selected_control_point, x, y);
  ::glutPostRedisplay();
}

auto GLCanvas::keyboard(unsigned char key, int x, int y) -> void
{
  switch (key)
  {
    case 's':
    case 'S':
      ::printf("Saving... ");
      ::fflush(stdout);
      splines->SaveBezier(args);
      splines->SaveBSpline(args);
      splines->SaveTriangles(args);
      ::printf("done\n");
      break;
    case 'q':
    case 'Q':
      ::exit(0);
      break;
    default:
      ::printf("UNKNOWN KEYBOARD INPUT  '%c'\n", key);
  }
}
