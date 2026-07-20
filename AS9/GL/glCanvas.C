#include "glCanvas.h"
#include "gl_headers.h"
#include "parser.h"
#include "system.h"
#include "particle.h"
#include "matrix.h"

Parser* GLCanvas::parser = nullptr;

Vec3f GLCanvas::camera_pos = Vec3f(0, 0, 20);
int GLCanvas::width = 250;
int GLCanvas::height = 250;
int GLCanvas::mouse_button = -1;
int GLCanvas::mouse_x = 0;
int GLCanvas::mouse_y = 0;

int GLCanvas::paused = 0;
float GLCanvas::refresh = 100;
float GLCanvas::dt = 100;
int GLCanvas::integrator_color = 0;
int GLCanvas::draw_vectors = 0;
float GLCanvas::acceleration_scale = 0;
int GLCanvas::motion_blur = 0;

auto GLCanvas::initialize(Parser* _parser, float _refresh, float _dt, int _integrator_color,
                          int _draw_vectors, float _acceleration_scale, int _motion_blur) -> void {
  parser = _parser;
  refresh = _refresh;
  dt = _dt;
  integrator_color = _integrator_color;
  draw_vectors = _draw_vectors;
  acceleration_scale = _acceleration_scale;
  motion_blur = _motion_blur;

  ::glEnable(GL_LIGHTING);
  ::glShadeModel(GL_SMOOTH);

  ::glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
  ::glutInitWindowSize(width, height);
  ::glutInitWindowPosition(100, 100);
  ::glutCreateWindow("Particle System");

  ::glutSetMenu(0);

  GLfloat ambArr[] = {0.1, 0.1, 0.1, 1};
  ::glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambArr);
  ::glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  ::glDisable(GL_CULL_FACE);

  ::glutMouseFunc(mouse);
  ::glutMotionFunc(motion);
  ::glutDisplayFunc(display);
  ::glutReshapeFunc(reshape);
  ::glutKeyboardFunc(keyboard);

  ::glutTimerFunc(0, idle, 0);
  ::glutMainLoop();
}

auto GLCanvas::display() -> void
{
  ::glClearColor(0, 0, 0, 1.0);
  ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ::glEnable(GL_DEPTH_TEST);

  GLfloat pos[4] = {1, 1, 1, 1};
  GLfloat one[4] = {1, 1, 1, 1};
  GLfloat zero[4] = {0, 0, 0, 1};
  ::glLightfv(GL_LIGHT1, GL_POSITION, pos);
  ::glLightfv(GL_LIGHT1, GL_DIFFUSE, one);
  ::glLightfv(GL_LIGHT1, GL_SPECULAR, zero);
  ::glLightfv(GL_LIGHT1, GL_AMBIENT, zero);
  ::glEnable(GL_LIGHT1);

  ::glMatrixMode(GL_MODELVIEW);
  ::glLoadIdentity();
  ::gluLookAt(camera_pos.x(), camera_pos.y(), camera_pos.z(), 0, 0, 0, 0, 1, 0);

  ::glEnable(GL_LIGHTING);
  for (auto i = 0; i < parser->getNumSystems(); i++) {
    parser->getSystem(i)->PaintGeometry();
  }
  ::glDisable(GL_LIGHTING);

  for (auto i = 0; i < parser->getNumSystems(); i++) {
    parser->getSystem(i)->Paint(dt, integrator_color, draw_vectors, acceleration_scale,
                                motion_blur);
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
  auto aspect = static_cast<float>(w) / static_cast<float>(h);
  auto asp_angle = 30.0f;
  if (aspect > 1)
  {
    asp_angle /= aspect;
  }
  ::gluPerspective(asp_angle, aspect, 0.1, 1000.0);
}

auto GLCanvas::mouse(int button, int state, int x, int y) -> void
{
  if (button != GLUT_LEFT_BUTTON && button != GLUT_RIGHT_BUTTON)
  {
    return;
  }

  if (state == GLUT_DOWN)
  {
    mouse_x = x;
    mouse_y = y;
    mouse_button = button;
  } else if (state == GLUT_UP)
  {
    mouse_button = -1;
  }

  ::glutPostRedisplay();
}

auto GLCanvas::motion(int x, int y) -> void
{
  if (mouse_button == -1)
  {
    return;
  }

  if (mouse_button == GLUT_LEFT_BUTTON)
  {
    auto rx = 0.01f * static_cast<float>(mouse_x - x);
    auto ry = -0.01f * static_cast<float>(mouse_y - y);
    auto dir = camera_pos;
    dir.Normalize();
    auto up = Vec3f(0, 1, 0);
    Vec3f horiz;
    Vec3f::Cross3(horiz, dir, up);
    horiz.Normalize();
    auto tiltAngle = ::acosf(up.Dot3(dir));
    if (tiltAngle - ry > 3.13f)
    {
      ry = tiltAngle - 3.13f;
    } else if (tiltAngle - ry < 0.01f)
    {
      ry = tiltAngle - 0.01f;
    }
    auto rotMat = Matrix::MakeAxisRotation(up, rx);
    rotMat *= Matrix::MakeAxisRotation(horiz, ry);
    rotMat.TransformDirection(dir);
    auto length = camera_pos.Length();
    camera_pos = dir * length;
  } else if (mouse_button == GLUT_RIGHT_BUTTON)
  {
    auto dolly = 0.05f * static_cast<float>((mouse_y - y) + 0.25f * (mouse_x - x));
    auto dist = camera_pos.Length();
    dist -= dolly;
    if (dist < 2.0f)
    {
      dist = 2.0f;
    }
    if (dist > 500.0f)
    {
      dist = 500.0f;
    }
    auto dir = camera_pos;
    dir.Normalize();
    camera_pos = dir * dist;
  }

  mouse_x = x;
  mouse_y = y;
  ::glutPostRedisplay();
}

auto GLCanvas::keyboard(unsigned char key, int x, int y) -> void
{
  switch (key)
  {
    case 'p':
    case 'P':
      if (paused == 0)
      {
        ::printf("pause (press 'p' again to un-pause)\n");
        paused = 1;
      } else {
        ::printf("un-pause\n");
        paused = 0;
      }
      break;
    case 'r':
    case 'R':
      ::printf("restart\n");
      restart();
      break;
    case 's':
    case 'S':
      step();
      break;
    case 'q':
    case 'Q':
      ::printf("quit!\n");
      ::exit(0);
      break;
    default:
      ::printf("UNKNOWN KEYBOARD INPUT  '%c'\n", key);
  }
}

auto GLCanvas::idle(int value) -> void
{
  auto refresh_milliseconds = static_cast<int>(1000 * refresh);
  ::glutTimerFunc(refresh_milliseconds, idle, 0);
  if (paused)
  {
    return;
  }
  step();
}

auto GLCanvas::step() -> void
{
  for (auto i = 0; i < parser->getNumSystems(); i++) {
    parser->getSystem(i)->Update(dt);
  }
  ::glutPostRedisplay();
}

auto GLCanvas::restart() -> void
{
  for (auto i = 0; i < parser->getNumSystems(); i++) {
    parser->getSystem(i)->Restart();
  }
  ::glutPostRedisplay();
}
