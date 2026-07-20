#include "shader_program.h"
#include "gl_headers.h"

#include <cstdio>
#include <fstream>
#include <map>
#include <sstream>

ShaderProgram::ShaderProgram() {}

ShaderProgram::~ShaderProgram() { destroy(); }

static auto readFile(std::string const& path) -> std::string
{
  std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
  if (!file)
    return "";
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

auto ShaderProgram::compileShader(unsigned int type, char const* source)
    -> unsigned int
{
  unsigned int shader = ::glCreateShader(type);
  ::glShaderSource(shader, 1, &source, nullptr);
  ::glCompileShader(shader);

  int success = 0;
  ::glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char log[1024];
    ::glGetShaderInfoLog(shader, 1024, nullptr, log);
    std::fprintf(stderr, "Shader compile error:\n%s\n", log);
    ::glDeleteShader(shader);
    return 0;
  }
  return shader;
}

auto ShaderProgram::linkProgram(unsigned int vert, unsigned int frag) -> bool
{
  program = ::glCreateProgram();
  ::glAttachShader(program, vert);
  ::glAttachShader(program, frag);
  ::glLinkProgram(program);

  int success = 0;
  ::glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char log[1024];
    ::glGetProgramInfoLog(program, 1024, nullptr, log);
    std::fprintf(stderr, "Shader link error:\n%s\n", log);
    ::glDeleteProgram(program);
    program = 0;
    return false;
  }
  return true;
}

auto ShaderProgram::loadFromFiles(std::string const& vertPath,
                                  std::string const& fragPath) -> bool
{
  std::string vertSrc = readFile(vertPath);
  std::string fragSrc = readFile(fragPath);
  if (vertSrc.empty() || fragSrc.empty()) {
    std::fprintf(stderr, "Failed to read shader files: %s, %s\n",
                 vertPath.c_str(), fragPath.c_str());
    return false;
  }
  return loadFromSource(vertSrc.c_str(), fragSrc.c_str());
}

auto ShaderProgram::loadFromSource(char const* vertSrc, char const* fragSrc)
    -> bool
{
  destroy();
  unsigned int vert = compileShader(GL_VERTEX_SHADER, vertSrc);
  unsigned int frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
  if (!vert || !frag)
    return false;

  bool ok = linkProgram(vert, frag);
  ::glDeleteShader(vert);
  ::glDeleteShader(frag);
  return ok;
}

auto ShaderProgram::use() const -> void { ::glUseProgram(program); }

auto ShaderProgram::destroy() -> void
{
  if (program) {
    ::glDeleteProgram(program);
    program = 0;
  }
  uniformCache.clear();
}

auto ShaderProgram::getUniformLocation(std::string const& name) const -> int
{
  std::map<std::string, int>::const_iterator it = uniformCache.find(name);
  if (it != uniformCache.end())
    return it->second;
  int loc = ::glGetUniformLocation(program, name.c_str());
  uniformCache[name] = loc;
  return loc;
}

auto ShaderProgram::setBool(std::string const& name, bool value) const -> void
{
  ::glUniform1i(getUniformLocation(name), value ? 1 : 0);
}

auto ShaderProgram::setInt(std::string const& name, int value) const -> void
{
  ::glUniform1i(getUniformLocation(name), value);
}

auto ShaderProgram::setFloat(std::string const& name, float value) const -> void
{
  ::glUniform1f(getUniformLocation(name), value);
}

auto ShaderProgram::setVec2(std::string const& name, float x, float y) const
    -> void
{
  ::glUniform2f(getUniformLocation(name), x, y);
}

auto ShaderProgram::setVec3(std::string const& name, float x, float y,
                            float z) const -> void
{
  ::glUniform3f(getUniformLocation(name), x, y, z);
}

auto ShaderProgram::setVec4(std::string const& name, float x, float y, float z,
                            float w) const -> void
{
  ::glUniform4f(getUniformLocation(name), x, y, z, w);
}

auto ShaderProgram::setMat4(std::string const& name, float const* value) const
    -> void
{
  ::glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value);
}
