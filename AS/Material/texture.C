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

bool Texture::loadFromFile(const std::string &path, bool srgb) {
  destroy();
  srgbColorSpace = srgb;

  int channels = 0;

  // 纹理不翻转
  stbi_set_flip_vertically_on_load(false);

  unsigned char *data = stbi_load(path.c_str(), &texWidth, &texHeight, &channels, 0);
  if (!data) {
    fprintf(stderr, "Failed to load texture: %s\n", path.c_str());
    return false;
  }

  GLenum internalFormat = GL_RGB;
  GLenum format = GL_RGB;
  std::vector<unsigned char> expanded;
  unsigned char *uploadData = data;

  if (channels == 1) {
    internalFormat = GL_RED;
    format = GL_RED;
  } else if (channels == 2) {
    // 灰度+Alpha 等双通道贴图扩展为 RGBA
    expanded.resize((size_t)texWidth * (size_t)texHeight * 4);
    size_t pixels = (size_t)texWidth * (size_t)texHeight;
    for (size_t i = 0; i < pixels; i++) {
      unsigned char lum = data[i * 2 + 0];
      unsigned char alpha = data[i * 2 + 1];
      expanded[i * 4 + 0] = lum;
      expanded[i * 4 + 1] = lum;
      expanded[i * 4 + 2] = lum;
      expanded[i * 4 + 3] = alpha;
    }
    stbi_image_free(data);
    uploadData = expanded.data();
    internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
    format = GL_RGBA;
  } else if (channels == 3) {
    internalFormat = srgb ? GL_SRGB : GL_RGB;
    format = GL_RGB;
  } else if (channels == 4) {
    internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
    format = GL_RGBA;
  } else {
    fprintf(stderr, "Unsupported channel count %d in texture: %s\n", channels, path.c_str());
    stbi_image_free(data);
    return false;
  }

  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, uploadData);
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

  if (uploadData != expanded.data())
    stbi_image_free(data);
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
