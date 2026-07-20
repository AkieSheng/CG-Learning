#pragma once

#include <string>
#include <vector>

struct Texture final {
  Texture();
  ~Texture();

  auto loadFromFile(std::string const& path, bool srgb = false) -> bool;
  auto bind(unsigned int unit) const -> void;
  auto destroy() -> void;

  auto id() const -> unsigned int { return textureId; }
  auto width() const -> int { return texWidth; }
  auto height() const -> int { return texHeight; }
  auto valid() const -> bool { return textureId != 0; }

  static auto createSolid(unsigned char r, unsigned char g, unsigned char b,
                          unsigned char a = 255) -> Texture*;
  static auto createDefaultNormal() -> Texture*;

  static auto loadPixelsRGBA(std::string const& path,
                             std::vector<unsigned char>& rgba, int& outWidth,
                             int& outHeight) -> bool;
  static auto createFromRGBA(unsigned char const* rgba, int width, int height,
                             bool srgb) -> Texture*;

  unsigned int textureId{};
  int texWidth{};
  int texHeight{};
  bool srgbColorSpace{};

  auto uploadRGBA(unsigned char const* rgba, int width, int height, bool srgb)
      -> void;
};
