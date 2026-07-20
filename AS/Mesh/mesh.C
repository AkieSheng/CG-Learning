#include "mesh.h"
#include "shader_program.h"
#include "gl_headers.h"

#include <cstddef>
#include <cstdio>

Mesh::Mesh() { modelMatrix.SetToIdentity(); }

Mesh::~Mesh() { destroy(); }

auto Mesh::upload(std::vector<Vertex> const& vertices,
                  std::vector<unsigned int> const& indices) -> void {
  destroy();
  numIndices = static_cast<int>(indices.size());

  ::glGenVertexArrays(1, &vao);
  ::glGenBuffers(1, &vbo);
  ::glGenBuffers(1, &ebo);

  ::glBindVertexArray(vao);

  ::glBindBuffer(GL_ARRAY_BUFFER, vbo);
  ::glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);

  ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  ::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                 indices.data(), GL_STATIC_DRAW);

  GLsizei stride = static_cast<GLsizei>(sizeof(Vertex));

  ::glEnableVertexAttribArray(0);
  ::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          static_cast<void*>(0));

  ::glEnableVertexAttribArray(1);
  ::glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));

  ::glEnableVertexAttribArray(2);
  ::glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, tangent)));

  ::glEnableVertexAttribArray(3);
  ::glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, texCoord0)));

  ::glBindVertexArray(0);
}

auto Mesh::draw() const -> void {
  if (vao == 0 || numIndices == 0) {
    return;
  }
  ::glBindVertexArray(vao);
  ::glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
  ::glBindVertexArray(0);
}

auto Mesh::destroy() -> void {
  if (ebo) {
    ::glDeleteBuffers(1, &ebo);
    ebo = 0;
  }
  if (vbo) {
    ::glDeleteBuffers(1, &vbo);
    vbo = 0;
  }
  if (vao) {
    ::glDeleteVertexArrays(1, &vao);
    vao = 0;
  }
  numIndices = 0;
}
