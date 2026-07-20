#include "texture.h"
#include "gl_headers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

Texture::Texture() {}

Texture::~Texture() { destroy(); }

auto Texture::uploadRGBA(unsigned char const* rgba, int width, int height,
                         bool srgb) -> void
{
  destroy();
  srgbColorSpace = srgb;
  texWidth = width;
  texHeight = height;

  GLenum internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
  ::glGenTextures(1, &textureId);
  ::glBindTexture(GL_TEXTURE_2D, textureId);
  ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texWidth, texHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba);
  ::glGenerateMipmap(GL_TEXTURE_2D);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  GLfloat maxAniso = 1.0f;
  ::glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
  if (maxAniso > 1.0f) {
    GLfloat aniso = maxAniso > 8.0f ? 8.0f : maxAniso;
    ::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
  }
}

auto Texture::loadPixelsRGBA(std::string const& path,
                             std::vector<unsigned char>& rgba, int& outWidth,
                             int& outHeight) -> bool
{
  rgba.clear();
  outWidth = outHeight = 0;

  stbi_set_flip_vertically_on_load(false);
  int channels = 0;
  unsigned char* data =
      stbi_load(path.c_str(), &outWidth, &outHeight, &channels, 4);
  if (!data) {
    std::fprintf(stderr, "Failed to load texture pixels: %s\n", path.c_str());
    return false;
  }

  size_t bytes = static_cast<size_t>(outWidth) * static_cast<size_t>(outHeight) * 4;
  rgba.assign(data, data + bytes);
  stbi_image_free(data);
  return true;
}

auto Texture::createFromRGBA(unsigned char const* rgba, int width, int height,
                             bool srgb) -> Texture*
{
  if (!rgba || width <= 0 || height <= 0)
    return nullptr;
  Texture* tex = new Texture();
  tex->uploadRGBA(rgba, width, height, srgb);
  return tex;
}

auto Texture::loadFromFile(std::string const& path, bool srgb) -> bool
{
  std::vector<unsigned char> rgba;
  int w = 0, h = 0;
  if (!loadPixelsRGBA(path, rgba, w, h))
    return false;
  uploadRGBA(rgba.data(), w, h, srgb);
  return true;
}

auto Texture::bind(unsigned int unit) const -> void
{
  ::glActiveTexture(GL_TEXTURE0 + unit);
  ::glBindTexture(GL_TEXTURE_2D, textureId);
}

auto Texture::destroy() -> void
{
  if (textureId != 0) {
    ::glDeleteTextures(1, &textureId);
    textureId = 0;
  }
}

auto Texture::createSolid(unsigned char r, unsigned char g, unsigned char b,
                          unsigned char a) -> Texture*
{
  Texture* tex = new Texture();
  unsigned char data[4] = {r, g, b, a};
  ::glGenTextures(1, &tex->textureId);
  ::glBindTexture(GL_TEXTURE_2D, tex->textureId);
  ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  tex->texWidth = 1;
  tex->texHeight = 1;
  return tex;
}

auto Texture::createDefaultNormal() -> Texture*
{
  return createSolid(128, 128, 255, 255);
}
