#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>

#include "image.h"

namespace {

auto ReadByte(FILE* file) -> unsigned char {
  unsigned char b{};
  auto success = int(::fread(static_cast<void*>(&b), sizeof(unsigned char), 1, file));
  assert(success == 1);
  return b;
}

auto WriteByte(FILE* file, unsigned char b) -> void {
  auto success = int(::fwrite(static_cast<void*>(&b), sizeof(unsigned char), 1, file));
  assert(success == 1);
}

auto ClampColorComponent(float c) -> unsigned char {
  auto tmp = int(c * 255);
  if (tmp < 0) {
    tmp = 0;
  }
  if (tmp > 255) {
    tmp = 255;
  }
  return static_cast<unsigned char>(tmp);
}

}  // namespace

auto Image::SaveTGA(char const* filename) const -> void {
  assert(filename != nullptr);
  auto const* ext = &filename[::strlen(filename) - 4];
  assert(!::strcmp(ext, ".tga"));
  auto* file = ::fopen(filename, "wb");
  for (auto i = 0; i < 18; i++) {
    if (i == 2) {
      WriteByte(file, 2);
    } else if (i == 12) {
      WriteByte(file, width % 256);
    } else if (i == 13) {
      WriteByte(file, width / 256);
    } else if (i == 14) {
      WriteByte(file, height % 256);
    } else if (i == 15) {
      WriteByte(file, height / 256);
    } else if (i == 16) {
      WriteByte(file, 24);
    } else if (i == 17) {
      WriteByte(file, 32);
    } else {
      WriteByte(file, 0);
    }
  }
  for (auto y = height - 1; y >= 0; y--) {
    for (auto x = 0; x < width; x++) {
      auto v = GetPixel(x, y);
      WriteByte(file, ClampColorComponent(v.b()));
      WriteByte(file, ClampColorComponent(v.g()));
      WriteByte(file, ClampColorComponent(v.r()));
    }
  }
  ::fclose(file);
}

auto Image::LoadTGA(char const* filename) -> Image* {
  assert(filename != nullptr);
  auto const* ext = &filename[::strlen(filename) - 4];
  assert(!::strcmp(ext, ".tga"));
  auto* file = ::fopen(filename, "rb");
  auto width = 0;
  auto height = 0;
  for (auto i = 0; i < 18; i++) {
    auto tmp = ReadByte(file);
    if (i == 2) {
      assert(tmp == 2);
    } else if (i == 12) {
      width += tmp;
    } else if (i == 13) {
      width += 256 * tmp;
    } else if (i == 14) {
      height += tmp;
    } else if (i == 15) {
      height += 256 * tmp;
    } else if (i == 16) {
      assert(tmp == 24);
    } else if (i == 17) {
      assert(tmp == 32);
    } else {
      assert(tmp == 0);
    }
  }
  auto* answer = new Image(width, height);
  for (auto y = height - 1; y >= 0; y--) {
    for (auto x = 0; x < width; x++) {
      auto b = ReadByte(file);
      auto g = ReadByte(file);
      auto r = ReadByte(file);
      auto color = Vec3f(r / 255.0f, g / 255.0f, b / 255.0f);
      answer->SetPixel(x, y, color);
    }
  }
  ::fclose(file);
  return answer;
}

auto Image::SavePPM(char const* filename) const -> void {
  assert(filename != nullptr);
  auto const* ext = &filename[::strlen(filename) - 4];
  assert(!::strcmp(ext, ".ppm"));
  auto* file = ::fopen(filename, "w");
  assert(file != nullptr);
  ::fprintf(file, "P6\n");
  ::fprintf(file, "# Creator: Image::SavePPM()\n");
  ::fprintf(file, "%d %d\n", width, height);
  ::fprintf(file, "255\n");
  for (auto y = height - 1; y >= 0; y--) {
    for (auto x = 0; x < width; x++) {
      auto v = GetPixel(x, y);
      ::fputc(ClampColorComponent(v.r()), file);
      ::fputc(ClampColorComponent(v.g()), file);
      ::fputc(ClampColorComponent(v.b()), file);
    }
  }
  ::fclose(file);
}

auto Image::LoadPPM(char const* filename) -> Image* {
  assert(filename != nullptr);
  auto const* ext = &filename[::strlen(filename) - 4];
  assert(!::strcmp(ext, ".ppm"));
  auto* file = ::fopen(filename, "rb");
  auto width = 0;
  auto height = 0;
  char tmp[100];
  ::fgets(tmp, 100, file);
  assert(::strstr(tmp, "P6"));
  ::fgets(tmp, 100, file);
  assert(tmp[0] == '#');
  ::fgets(tmp, 100, file);
  ::sscanf(tmp, "%d %d", &width, &height);
  ::fgets(tmp, 100, file);
  assert(::strstr(tmp, "255"));
  auto* answer = new Image(width, height);
  for (auto y = height - 1; y >= 0; y--) {
    for (auto x = 0; x < width; x++) {
      auto r = static_cast<unsigned char>(::fgetc(file));
      auto g = static_cast<unsigned char>(::fgetc(file));
      auto b = static_cast<unsigned char>(::fgetc(file));
      auto color = Vec3f(r / 255.0f, g / 255.0f, b / 255.0f);
      answer->SetPixel(x, y, color);
    }
  }
  ::fclose(file);
  return answer;
}

auto Image::Compare(Image* img1, Image* img2) -> Image* {
  assert(img1->Width() == img2->Width());
  assert(img1->Height() == img2->Height());

  auto* img3 = new Image(img1->Width(), img1->Height());

  for (auto x = 0; x < img1->Width(); x++) {
    for (auto y = 0; y < img1->Height(); y++) {
      auto color1 = img1->GetPixel(x, y);
      auto color2 = img2->GetPixel(x, y);
      auto color3 = Vec3f(
          ::fabs(color1.r() - color2.r()),
          ::fabs(color1.g() - color2.g()),
          ::fabs(color1.b() - color2.b()));
      img3->SetPixel(x, y, color3);
    }
  }

  return img3;
}
