#include "application.h"
#include "gl_headers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef GLUT_WHEEL_UP
#define GLUT_WHEEL_UP 3
#endif
#ifndef GLUT_WHEEL_DOWN
#define GLUT_WHEEL_DOWN 4
#endif

Application *Application::instance = NULL;

Application::Application()
  : windowWidth(1280),
    windowHeight(720),
    mouseX(0),
    mouseY(0),
    mouseButton(-1) {
  instance = this;
}

Application::~Application() {
  if (instance == this) instance = NULL;
}

bool Application::initialize(int argc, char **argv, const std::string &path) {
  modelPath = path;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutInitWindowSize(windowWidth, windowHeight);
  glutCreateWindow("AS PBR Viewer");

  if (!loadOpenGLFunctions()) {
    fprintf(stderr, "Failed to load OpenGL 3.3 functions.\n");
    return false;
  }

  if (!renderer.initialize(windowWidth, windowHeight)) {
    return false;
  }

  if (!modelPath.empty()) {
    if (!scene.loadModel(modelPath, (float)windowWidth / (float)windowHeight)) {
      fprintf(stderr, "Warning: model not loaded, showing empty scene.\n");
    }
  }

  glutDisplayFunc(displayCallback);
  glutReshapeFunc(reshapeCallback);
  glutKeyboardFunc(keyboardCallback);
  glutMouseFunc(mouseButtonCallback);
  glutMotionFunc(mouseMotionCallback);

  printf("Controls: LMB=rotate, RMB/MMB=pan, wheel/+/-=zoom, S=supersampling, F=FXAA, Q=quit\n");
  return true;
}

void Application::run() {
  glutMainLoop();
}

void Application::displayCallback() {
  if (!instance) return;
  instance->renderer.render(instance->scene);
  glutSwapBuffers();
}

void Application::reshapeCallback(int w, int h) {
  if (!instance) return;
  instance->windowWidth = w;
  instance->windowHeight = h;
  instance->renderer.resize(w, h);
}

void Application::keyboardCallback(unsigned char key, int x, int y) {
  (void)x; (void)y;
  if (!instance) return;

  OrbitCamera &cam = instance->scene.getCamera();
  switch (key) {
    case 27:
    case 'q':
    case 'Q':
      exit(0);
      break;
    case 'f':
    case 'F': {
      bool enabled = instance->renderer.toggleFXAA();
      fprintf(stderr, "FXAA: %s\n", enabled ? "on" : "off");
      break;
    }
    case 's':
    case 'S':
      instance->renderer.cycleSupersampling();
      break;
    case '+':
    case '=':
      cam.zoom(0.1f);
      break;
    case '-':
    case '_':
      cam.zoom(-0.1f);
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

void Application::mouseButtonCallback(int button, int state, int x, int y) {
  if (!instance) return;

  if (state == GLUT_DOWN &&
      (button == GLUT_WHEEL_UP || button == GLUT_WHEEL_DOWN)) {
    OrbitCamera &cam = instance->scene.getCamera();
    if (button == GLUT_WHEEL_UP) {
      cam.zoom(0.1f);
    } else {
      cam.zoom(-0.1f);
    }
    glutPostRedisplay();
    return;
  }

  if (state == GLUT_DOWN) {
    instance->mouseButton = button;
    instance->mouseX = x;
    instance->mouseY = y;
  } else {
    instance->mouseButton = -1;
  }
}

void Application::mouseMotionCallback(int x, int y) {
  if (!instance || instance->mouseButton < 0) return;

  int dx = x - instance->mouseX;
  int dy = y - instance->mouseY;
  instance->mouseX = x;
  instance->mouseY = y;

  OrbitCamera &cam = instance->scene.getCamera();
  float sensitivity = 0.3f;
  float panScale = cam.getDistance() * 0.002f;

  if (instance->mouseButton == GLUT_LEFT_BUTTON) {
    cam.rotate((float)dx * sensitivity, (float)-dy * sensitivity);
  } else if (instance->mouseButton == GLUT_RIGHT_BUTTON ||
             instance->mouseButton == GLUT_MIDDLE_BUTTON) {
    cam.pan((float)-dx * panScale, (float)dy * panScale);
  }
  glutPostRedisplay();
}
