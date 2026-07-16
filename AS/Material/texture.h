#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <string>
#include <vector>

// OpenGL 纹理（PNG/JPEG）
class Texture {
public:
  Texture();
  ~Texture();

  bool loadFromFile(const std::string &path, bool srgb = false);
  void bind(unsigned int unit) const;
  void destroy();

  unsigned int id() const { return textureId; }
  int width() const { return texWidth; }
  int height() const { return texHeight; }
  bool valid() const { return textureId != 0; }

  static Texture *createSolid(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);  // 默认纯色纹理
  static Texture *createDefaultNormal();  // 默认法线纹理

  // 读入文件为 RGBA8
  static bool loadPixelsRGBA(const std::string &path,
                             std::vector<unsigned char> &rgba,
                             int &outWidth, int &outHeight);
  // 从 RGBA8 像素上传，srgb 控制内部格式（baseColor 用 true，MR 用 false）
  static Texture *createFromRGBA(const unsigned char *rgba, int width, int height,
                                 bool srgb);

private:
  unsigned int textureId;  // OpenGL 纹理ID
  // 纹理分辨率
  int texWidth;
  int texHeight;
  bool srgbColorSpace;  // 是否用 sRGB 内部格式

  void uploadRGBA(const unsigned char *rgba, int width, int height, bool srgb);
};

#endif
