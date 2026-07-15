// OpenGL PBR glTF Viewer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "application.h"

// 打印可用模型
static void printUsage(const char *prog) {
  printf("Available models in Models/:\n");
  printf("  Models/macbook_air_notebook_pbr/scene.gltf\n");
  printf("  Models/chess_set/scene.gltf\n");
  printf("  Models/crystal_stone_rock/scene.gltf\n");
  printf("  Models/aviator_sunglasses/scene.gltf\n");
  printf("  Models/the_ultimate_glass_pack_cups_and_bottles/scene.gltf\n");
  printf("  Models/lmu_main_hall_ceiling_glass_pbr_texture/scene.gltf\n");
  printf("  Models/wuthering_waves_sigillum/scene.gltf\n");
}

int main(int argc, char **argv) {
  std::string modelPath;

  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-model") && i + 1 < argc) {
      modelPath = argv[++i];
    } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      printUsage(argv[0]);
      return 0;
    } else {
      fprintf(stderr, "Unknown argument: %s\n", argv[i]);
      printUsage(argv[0]);
      return 1;
    }
  }

  if (modelPath.empty()) {
    modelPath = "Models/macbook_air_notebook_pbr/scene.gltf";
    printf("No model specified, using default: %s\n", modelPath.c_str());
  }

  Application app;
  if (!app.initialize(argc, argv, modelPath)) {
    fprintf(stderr, "Failed to initialize application.\n");
    return 1;
  }

  app.run();  // GLUT 主循环
  return 0;
}
