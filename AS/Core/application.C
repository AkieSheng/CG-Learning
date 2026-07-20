#include "application.h"
#include "gl_headers.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef GLUT_WHEEL_UP
#define GLUT_WHEEL_UP 3
#endif
#ifndef GLUT_WHEEL_DOWN
#define GLUT_WHEEL_DOWN 4
#endif

Application* Application::instance = nullptr;

Application::Application() {
  instance = this;
}

Application::~Application() {
  if (instance == this) {
    instance = nullptr;
  }
}

auto Application::initialize(int argc, char** argv, std::string const& path)
    -> bool {
  modelPath = path;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutInitWindowSize(windowWidth, windowHeight);
  glutCreateWindow("AS PBR Viewer");

  if (!loadOpenGLFunctions()) {
    std::fprintf(stderr, "Failed to load OpenGL 3.3 functions.\n");
    return false;
  }

  if (!renderer.initialize(windowWidth, windowHeight)) {
    return false;
  }

  if (!modelPath.empty()) {
    if (!scene.loadModel(modelPath,
                         static_cast<float>(windowWidth) /
                             static_cast<float>(windowHeight))) {
      std::fprintf(stderr, "Warning: model not loaded, showing empty scene.\n");
    }
  }

  glutDisplayFunc(displayCallback);
  glutReshapeFunc(reshapeCallback);
  glutKeyboardFunc(keyboardCallback);
  glutMouseFunc(mouseButtonCallback);
  glutMotionFunc(mouseMotionCallback);

  std::printf(
      "Controls: LMB=rotate, RMB/MMB=pan, wheel/+/-=zoom, S=supersampling, "
      "F=FXAA, Q=quit\n");
  return true;
}

auto Application::run() -> void { glutMainLoop(); }

auto Application::displayCallback() -> void {
  if (!instance) {
    return;
  }
  instance->renderer.render(instance->scene);
  glutSwapBuffers();
}

auto Application::reshapeCallback(int w, int h) -> void {
  if (!instance) {
    return;
  }
  instance->windowWidth = w;
  instance->windowHeight = h;
  instance->renderer.resize(w, h);
}

auto Application::keyboardCallback(unsigned char key, int x, int y) -> void {
  (void)x;
  (void)y;
  if (!instance) {
    return;
  }

  OrbitCamera& cam = instance->scene.getCamera();
  switch (key) {
    case 27:
    case 'q':
    case 'Q':
      std::exit(0);
      break;
    case 'f':
    case 'F': {
      bool enabled = instance->renderer.toggleFXAA();
      std::fprintf(stderr, "FXAA: %s\n", enabled ? "on" : "off");
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

auto Application::mouseButtonCallback(int button, int state, int x, int y)
    -> void {
  if (!instance) {
    return;
  }

  if (state == GLUT_DOWN &&
      (button == GLUT_WHEEL_UP || button == GLUT_WHEEL_DOWN)) {
    OrbitCamera& cam = instance->scene.getCamera();
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

auto Application::mouseMotionCallback(int x, int y) -> void {
  if (!instance || instance->mouseButton < 0) {
    return;
  }

  int dx = x - instance->mouseX;
  int dy = y - instance->mouseY;
  instance->mouseX = x;
  instance->mouseY = y;

  OrbitCamera& cam = instance->scene.getCamera();
  float sensitivity = 0.3f;
  float panScale = cam.getDistance() * 0.002f;

  if (instance->mouseButton == GLUT_LEFT_BUTTON) {
    cam.rotate(static_cast<float>(dx) * sensitivity,
               static_cast<float>(-dy) * sensitivity);
  } else if (instance->mouseButton == GLUT_RIGHT_BUTTON ||
             instance->mouseButton == GLUT_MIDDLE_BUTTON) {
    cam.pan(static_cast<float>(-dx) * panScale,
            static_cast<float>(dy) * panScale);
  }
  glutPostRedisplay();
}
