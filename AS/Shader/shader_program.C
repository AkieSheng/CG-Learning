#include "shader_program.h"
#include "gl_headers.h"

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <map>

ShaderProgram::ShaderProgram() : program(0) {}

ShaderProgram::~ShaderProgram() {
  destroy();
}

static std::string readFile(const std::string &path) {
  std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
  if (!file) return "";
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

// 编译顶点/片段着色器，返回着色器ID
unsigned int ShaderProgram::compileShader(unsigned int type, const char *source) {
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  int success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char log[1024];
    glGetShaderInfoLog(shader, 1024, NULL, log);
    fprintf(stderr, "Shader compile error:\n%s\n", log);
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

// 链接着色器程序
bool ShaderProgram::linkProgram(unsigned int vert, unsigned int frag) {
  program = glCreateProgram();
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);

  int success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char log[1024];
    glGetProgramInfoLog(program, 1024, NULL, log);
    fprintf(stderr, "Shader link error:\n%s\n", log);
    glDeleteProgram(program);
    program = 0;
    return false;
  }
  return true;
}

bool ShaderProgram::loadFromFiles(const std::string &vertPath, const std::string &fragPath) {
  std::string vertSrc = readFile(vertPath);
  std::string fragSrc = readFile(fragPath);
  if (vertSrc.empty() || fragSrc.empty()) {
    fprintf(stderr, "Failed to read shader files: %s, %s\n", vertPath.c_str(), fragPath.c_str());
    return false;
  }
  return loadFromSource(vertSrc.c_str(), fragSrc.c_str());
}

bool ShaderProgram::loadFromSource(const char *vertSrc, const char *fragSrc) {
  destroy();
  unsigned int vert = compileShader(GL_VERTEX_SHADER, vertSrc);
  unsigned int frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
  if (!vert || !frag) return false;

  bool ok = linkProgram(vert, frag);
  glDeleteShader(vert);
  glDeleteShader(frag);
  return ok;
}

void ShaderProgram::use() const {
  glUseProgram(program);
}

void ShaderProgram::destroy() {
  if (program) {
    glDeleteProgram(program);
    program = 0;
  }
  uniformCache.clear();
}

int ShaderProgram::getUniformLocation(const std::string &name) const {
  std::map<std::string, int>::const_iterator it = uniformCache.find(name);
  if (it != uniformCache.end()) return it->second;
  int loc = glGetUniformLocation(program, name.c_str());
  uniformCache[name] = loc;
  return loc;
}

void ShaderProgram::setBool(const std::string &name, bool value) const {
  glUniform1i(getUniformLocation(name), value ? 1 : 0);
}

void ShaderProgram::setInt(const std::string &name, int value) const {
  glUniform1i(getUniformLocation(name), value);
}

void ShaderProgram::setFloat(const std::string &name, float value) const {
  glUniform1f(getUniformLocation(name), value);
}

void ShaderProgram::setVec2(const std::string &name, float x, float y) const {
  glUniform2f(getUniformLocation(name), x, y);
}

void ShaderProgram::setVec3(const std::string &name, float x, float y, float z) const {
  glUniform3f(getUniformLocation(name), x, y, z);
}

void ShaderProgram::setVec4(const std::string &name, float x, float y, float z, float w) const {
  glUniform4f(getUniformLocation(name), x, y, z, w);
}

void ShaderProgram::setMat4(const std::string &name, const float *value) const {
  glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value);
}
