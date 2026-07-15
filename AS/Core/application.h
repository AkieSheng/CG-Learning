#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <string>
#include "scene.h"
#include "renderer.h"

// GLUT 窗口、输入、渲染调度
class Application {
public:
  Application();
  ~Application();

  bool initialize(int argc, char **argv, const std::string &modelPath);
  void run();

private:
  static void displayCallback();  // 渲染回调
  static void reshapeCallback(int w, int h);  // 窗口大小回调
  static void keyboardCallback(unsigned char key, int x, int y);  // 键盘回调
  static void mouseButtonCallback(int button, int state, int x, int y);  // 鼠标按钮回调
  static void mouseMotionCallback(int x, int y);  // 鼠标移动回调

  static Application *instance;

  Scene scene;  // 场景
  Renderer renderer;  // 渲染器
  std::string modelPath;  // 模型路径

  int windowWidth;
  int windowHeight;
  int mouseX;
  int mouseY;
  int mouseButton;
};

#endif
