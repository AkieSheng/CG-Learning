#include "application.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static auto printUsage(char const* prog) -> void
{
  std::printf("Usage: %s -model <path/to/scene.gltf>\n\n", prog);
  std::printf("Available models in Models/:\n");
  std::printf("  Models/macbook_air_notebook_pbr/scene.gltf\n");
  std::printf("  Models/chess_set/scene.gltf\n");
  std::printf("  Models/low_poly_chess_set/scene.gltf\n");
  std::printf("  Models/crystal_stone_rock/scene.gltf\n");
  std::printf("  Models/aviator_sunglasses/scene.gltf\n");
  std::printf("  Models/the_ultimate_glass_pack_cups_and_bottles/scene.gltf\n");
  std::printf("  Models/lmu_main_hall_ceiling_glass_pbr_texture/scene.gltf\n");
  std::printf("  Models/cosmetic_serum_bottle/scene.gltf\n");
  std::printf("  Models/ship_in_a_bottle/scene.gltf\n");
  std::printf("  Models/primogemmorastardust_from_genshin_impact_free/scene.gltf\n");
  std::printf("  Models/bouquet_de_fleurs_tropicales__new_version_pbr/scene.gltf\n");
  std::printf("    (SpecGloss materials are auto-converted to Metallic-Roughness)\n");
}

auto main(int argc, char** argv) -> int
{
  std::string modelPath;

  for (int i = 1; i < argc; i++) {
    if (!std::strcmp(argv[i], "-model") && i + 1 < argc) {
      i += 1;
      modelPath = argv[i];
    } else if (!std::strcmp(argv[i], "-h") || !std::strcmp(argv[i], "--help")) {
      printUsage(argv[0]);
      return 0;
    } else {
      std::fprintf(stderr, "Unknown argument: %s\n", argv[i]);
      printUsage(argv[0]);
      return 1;
    }
  }

  if (modelPath.empty()) {
    modelPath = "Models/macbook_air_notebook_pbr/scene.gltf";
    std::printf("No model specified, using default: %s\n", modelPath.c_str());
  }

  Application app;
  if (!app.initialize(argc, argv, modelPath)) {
    std::fprintf(stderr, "Failed to initialize application.\n");
    return 1;
  }

  app.run();
  return 0;
}
