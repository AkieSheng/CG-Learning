#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "texture.h"
#include "gl_headers.h"

#include <stdio.h>
#include <string>
#include <vector>

Texture::Texture()
  : textureId(0), texWidth(0), texHeight(0), srgbColorSpace(false) {}

Texture::~Texture() {
  destroy();
}

void Texture::uploadRGBA(const unsigned char *rgba, int width, int height, bool srgb) {
  destroy();
  srgbColorSpace = srgb;
  texWidth = width;
  texHeight = height;

  GLenum internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texWidth, texHeight, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, rgba);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // 各向异性过滤：减轻斜视 mip 发糊；上限钳到 8 以控制开销
  GLfloat maxAniso = 1.0f;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
  if (maxAniso > 1.0f) {
    GLfloat aniso = maxAniso > 8.0f ? 8.0f : maxAniso;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
  }
}

bool Texture::loadPixelsRGBA(const std::string &path,
                             std::vector<unsigned char> &rgba,
                             int &outWidth, int &outHeight) {
  rgba.clear();
  outWidth = outHeight = 0;

  stbi_set_flip_vertically_on_load(false);
  int channels = 0;
  unsigned char *data = stbi_load(path.c_str(), &outWidth, &outHeight, &channels, 4);
  if (!data) {
    fprintf(stderr, "Failed to load texture pixels: %s\n", path.c_str());
    return false;
  }

  size_t bytes = (size_t)outWidth * (size_t)outHeight * 4;
  rgba.assign(data, data + bytes);
  stbi_image_free(data);
  return true;
}

Texture *Texture::createFromRGBA(const unsigned char *rgba, int width, int height,
                                 bool srgb) {
  if (!rgba || width <= 0 || height <= 0) return NULL;
  Texture *tex = new Texture();
  tex->uploadRGBA(rgba, width, height, srgb);
  return tex;
}

bool Texture::loadFromFile(const std::string &path, bool srgb) {
  std::vector<unsigned char> rgba;
  int w = 0, h = 0;
  if (!loadPixelsRGBA(path, rgba, w, h))
    return false;
  uploadRGBA(rgba.data(), w, h, srgb);
  return true;
}

void Texture::bind(unsigned int unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, textureId);
}

void Texture::destroy() {
  if (textureId != 0) {
    glDeleteTextures(1, &textureId);
    textureId = 0;
  }
}

// 创建 1x1 纯色占位纹理
Texture *Texture::createSolid(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
  Texture *tex = new Texture();
  unsigned char data[4] = {r, g, b, a};
  glGenTextures(1, &tex->textureId);
  glBindTexture(GL_TEXTURE_2D, tex->textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  tex->texWidth = 1;
  tex->texHeight = 1;
  return tex;
}

// 创建默认法线纹理
Texture *Texture::createDefaultNormal() {
  return createSolid(128, 128, 255, 255);
}
