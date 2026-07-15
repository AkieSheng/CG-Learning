#ifndef _SHADER_PROGRAM_H_
#define _SHADER_PROGRAM_H_

#include <string>
#include <map>

// 着色器，封装 GLSL 编译、链接和 Uniform 设置
class ShaderProgram {
public:
  ShaderProgram();
  ~ShaderProgram();

  bool loadFromFiles(const std::string &vertPath, const std::string &fragPath);
  bool loadFromSource(const char *vertSrc, const char *fragSrc);
  void use() const;
  void destroy();

  unsigned int programId() const { return program; }

  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec2(const std::string &name, float x, float y) const;
  void setVec3(const std::string &name, float x, float y, float z) const;
  void setVec4(const std::string &name, float x, float y, float z, float w) const;
  void setMat4(const std::string &name, const float *value) const;

private:
  unsigned int compileShader(unsigned int type, const char *source);  // 从文件读源码并编译链接
  bool linkProgram(unsigned int vert, unsigned int frag);  // 从字符串编译链接
  int getUniformLocation(const std::string &name) const;

  unsigned int program;  // 着色器程序 ID
  mutable std::map<std::string, int> uniformCache;  // 缓存 uniform 位置
};

#endif
