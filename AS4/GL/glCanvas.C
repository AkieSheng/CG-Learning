#include "glCanvas.h"
#include "scene_parser.h"
#include "light.h"
#include "camera.h"
#include "group.h"
#include "rayTree.h"

#include <GL/gl.h>
#include <GL/glut.h>
#include <cstdio>
#include <cstdlib>

void (*GLCanvas::renderFunction)();
void (*GLCanvas::traceRayFunction)(float, float);
SceneParser* GLCanvas::scene;
int GLCanvas::mouseButton;
int GLCanvas::mouseX;
int GLCanvas::mouseY;

#ifdef SPECULAR_FIX
int SPECULAR_FIX_WHICH_PASS = 0;
#endif

auto GLCanvas::drawAxes() -> void {
  ::glDisable(GL_LIGHTING);
  ::glColor3f(1.0, 0.0, 0.0);
  ::glBegin(GL_LINES);
  ::glVertex3f(0, 0, 0);
  ::glVertex3f(1, 0, 0);
  ::glEnd();
  ::glBegin(GL_TRIANGLE_FAN);
  ::glVertex3f(1.0, 0.0, 0.0);
  ::glVertex3f(0.8, 0.07, 0.0);
  ::glVertex3f(0.8, 0.0, 0.07);
  ::glVertex3f(0.8, -0.07, 0.0);
  ::glVertex3f(0.8, 0.0, -0.07);
  ::glVertex3f(0.8, 0.07, 0.0);
  ::glEnd();

  ::glColor3f(0.0, 1.0, 0.0);
  ::glBegin(GL_LINES);
  ::glVertex3f(0, 0, 0);
  ::glVertex3f(0, 1, 0);
  ::glEnd();
  ::glBegin(GL_TRIANGLE_FAN);
  ::glVertex3f(0.0, 1.0, 0.0);
  ::glVertex3f(0.07, 0.8, 0.0);
  ::glVertex3f(0.0, 0.8, 0.07);
  ::glVertex3f(-0.07, 0.8, 0.0);
  ::glVertex3f(0.0, 0.8, -0.07);
  ::glVertex3f(0.07, 0.8, 0.0);
  ::glEnd();

  ::glColor3f(0.0, 0.0, 1.0);
  ::glBegin(GL_LINES);
  ::glVertex3f(0, 0, 0);
  ::glVertex3f(0, 0, 1);
  ::glEnd();
  ::glBegin(GL_TRIANGLE_FAN);
  ::glVertex3f(0.0, 0.0, 1.0);
  ::glVertex3f(0.07, 0.0, 0.8);
  ::glVertex3f(0.0, 0.07, 0.8);
  ::glVertex3f(-0.07, 0.0, 0.8);
  ::glVertex3f(0.0, -0.07, 0.8);
  ::glVertex3f(0.07, 0.0, 0.8);
  ::glEnd();
}

auto GLCanvas::display() -> void {
  auto bgColor = scene->getBackgroundColor();
  ::glClearColor(bgColor.x(), bgColor.y(), bgColor.z(), 1.0);
  ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ::glMatrixMode(GL_MODELVIEW);
  ::glLoadIdentity();
  scene->getCamera()->glPlaceCamera();

  ::glEnable(GL_LIGHTING);
  ::glEnable(GL_DEPTH_TEST);

  for (auto i = 0; i < scene->getNumLights(); i++) {
    scene->getLight(i)->glInit(i);
  }

#if !SPECULAR_FIX

  SPECULAR_FIX_WHICH_PASS = 0;
  scene->getGroup()->paint();

#else

  SPECULAR_FIX_WHICH_PASS = 0;
  scene->getGroup()->paint();

  ::glDepthFunc(GL_EQUAL);
  ::glEnable(GL_BLEND);

  SPECULAR_FIX_WHICH_PASS = 1;
  ::glBlendFunc(GL_DST_COLOR, GL_ZERO);
  scene->getGroup()->paint();

  SPECULAR_FIX_WHICH_PASS = 2;
  ::glBlendFunc(GL_ONE, GL_ONE);
  scene->getGroup()->paint();

  ::glDepthFunc(GL_LESS);
  ::glDisable(GL_BLEND);

#endif

  ::glDisable(GL_LIGHTING);
  RayTree::paint();
  ::glEnable(GL_LIGHTING);

  ::glutSwapBuffers();
}

auto GLCanvas::reshape(int w, int h) -> void {
  ::glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  scene->getCamera()->glInit(w, h);
}

auto GLCanvas::mouse(int button, int state, int x, int y) -> void {
  mouseButton = button;
  mouseX = x;
  mouseY = y;
}

auto GLCanvas::motion(int x, int y) -> void {
  if (mouseButton == GLUT_LEFT_BUTTON) {
    scene->getCamera()->rotateCamera(0.005f * (mouseX - x),
                                     0.005f * (mouseY - y));
    mouseX = x;
    mouseY = y;
  } else if (mouseButton == GLUT_MIDDLE_BUTTON) {
    scene->getCamera()->truckCamera((mouseX - x) * 0.05f,
                                    (y - mouseY) * 0.05f);
    mouseX = x;
    mouseY = y;
  } else if (mouseButton == GLUT_RIGHT_BUTTON) {
    scene->getCamera()->dollyCamera((x - mouseX) * 0.05f);
    mouseX = x;
    mouseY = y;
  }

  ::glutPostRedisplay();
}

auto GLCanvas::keyboard(unsigned char key, int i, int j) -> void {
  switch (key) {
    case 'r':
    case 'R':
      ::printf("Rendering scene... ");
      ::fflush(stdout);
      if (renderFunction) {
        renderFunction();
      }
      ::printf("done.\n");
      break;
    case 't':
    case 'T': {
      auto width = ::glutGet(GLUT_WINDOW_WIDTH);
      auto height = ::glutGet(GLUT_WINDOW_HEIGHT);
      j = height - j;
      auto max = (width > height) ? width : height;
      auto x = ((i + 0.5f) - width / 2.0f) / static_cast<float>(max) + 0.5f;
      auto y = ((j + 0.5f) - height / 2.0f) / static_cast<float>(max) + 0.5f;
      RayTree::Activate();
      if (traceRayFunction) {
        traceRayFunction(x, y);
      }
      RayTree::Deactivate();
      display();
      break;
    }
    case 'q':
    case 'Q':
      ::exit(0);
      break;
    default:
      ::printf("UNKNOWN KEYBOARD INPUT  '%c'\n", key);
  }
}

auto GLCanvas::initialize(SceneParser* _scene, void (*_renderFunction)(),
                          void (*_traceRayFunction)(float, float)) -> void {
  scene = _scene;
  renderFunction = _renderFunction;
  traceRayFunction = _traceRayFunction;

  ::glEnable(GL_LIGHTING);
  ::glShadeModel(GL_SMOOTH);
  ::glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  ::glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
  ::glEnable(GL_DEPTH_TEST);
  ::glutInitWindowSize(400, 400);
  ::glutInitWindowPosition(100, 100);
  ::glutCreateWindow("OpenGL Viewer");

  ::glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  ::glEnable(GL_NORMALIZE);

  auto ambColor = scene->getAmbientLight();
  GLfloat ambArr[] = {ambColor.x(), ambColor.y(), ambColor.z(), 1.0f};
  ::glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambArr);

  ::glutMouseFunc(mouse);
  ::glutMotionFunc(motion);
  ::glutDisplayFunc(display);
  ::glutReshapeFunc(reshape);
  ::glutKeyboardFunc(keyboard);

  ::glutMainLoop();
}
