#include "image.h"
#include "ifs.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

auto main(int argc, char* argv[]) -> int
{
  char* input_file = nullptr;
  auto num_points = 10000;
  auto num_iters = 10;
  auto size = 100;
  char* output_file = nullptr;

  for (auto i = 1; i < argc; i++) {
    if (!::strcmp(argv[i], "-input")) {
      i++;
      assert(i < argc);
      input_file = argv[i];
    } else if (!::strcmp(argv[i], "-points")) {
      i++;
      assert(i < argc);
      num_points = ::atoi(argv[i]);
    } else if (!::strcmp(argv[i], "-iters")) {
      i++;
      assert(i < argc);
      num_iters = ::atoi(argv[i]);
    } else if (!::strcmp(argv[i], "-size")) {
      i++;
      assert(i < argc);
      size = ::atoi(argv[i]);
    } else if (!::strcmp(argv[i], "-output")) {
      i++;
      assert(i < argc);
      output_file = argv[i];
    } else {
      ::printf("whoops error with command line argument\n");
      assert(0);
    }
  }

  assert(input_file != nullptr);
  assert(output_file != nullptr);

  ::srand(unsigned(::time(nullptr)));

  auto ifs = IFS{};
  ifs.Input(input_file);

  auto image = Image(size, size);
  ifs.Render(&image, num_points, num_iters);
  image.SaveTGA(output_file);

  return 0;
}
