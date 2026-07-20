#pragma once

#include <map>
#include <string>

struct ShaderProgram final
{
  ShaderProgram();
  ~ShaderProgram();

  auto loadFromFiles(std::string const& vertPath,
                     std::string const& fragPath) -> bool;
  auto loadFromSource(char const* vertSrc, char const* fragSrc) -> bool;
  auto use() const -> void;
  auto destroy() -> void;

  auto programId() const -> unsigned int { return program; }

  auto setBool(std::string const& name, bool value) const -> void;
  auto setInt(std::string const& name, int value) const -> void;
  auto setFloat(std::string const& name, float value) const -> void;
  auto setVec2(std::string const& name, float x, float y) const -> void;
  auto setVec3(std::string const& name, float x, float y, float z) const -> void;
  auto setVec4(std::string const& name, float x, float y, float z,
               float w) const -> void;
  auto setMat4(std::string const& name, float const* value) const -> void;

  unsigned int program{};
  mutable std::map<std::string, int> uniformCache{};

  auto compileShader(unsigned int type, char const* source) -> unsigned int;
  auto linkProgram(unsigned int vert, unsigned int frag) -> bool;
  auto getUniformLocation(std::string const& name) const -> int;
};
