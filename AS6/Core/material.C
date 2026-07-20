#include "material.h"

#include "glCanvas.h"
#include "gl_options.h"
#include "gl_headers.h"

#include <cmath>
#include <cassert>

#ifdef SPECULAR_FIX
  extern int SPECULAR_FIX_WHICH_PASS;
#endif


static auto componentMultiply(Vec3f const&a, Vec3f const&b) -> Vec3f {
  return Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}


PhongMaterial::PhongMaterial(Vec3f const& diffuse, Vec3f const& specular,
                             float exp, Vec3f const& reflective,
                             Vec3f const& transparent, float ior) {
  diffuseColor = diffuse;
  specularColor = specular;
  exponent = exp;
  reflectiveColor = reflective;
  transparentColor = transparent;
  indexOfRefraction = ior;
}

auto Material::getSpecularColor(Vec3f const& point) const -> Vec3f {
  return Vec3f(0, 0, 0);
}

auto Material::getExponent(Vec3f const&point)const -> float {
  return 1.0f;
}

auto Material::getReflectiveColor(Vec3f const&point)const -> Vec3f {
  return Vec3f(0, 0, 0);
}

auto Material::getTransparentColor(Vec3f const&point)const -> Vec3f {
  return Vec3f(0, 0, 0);
}

auto Material::getIndexOfRefraction(Vec3f const&point)const -> float {
  return 1.0f;
}





auto PhongMaterial::Shade(Ray const& ray, Hit const& hit,
                           Vec3f const& dirToLight,
                           Vec3f const& lightColor) const -> Vec3f {
  Vec3f normal = hit.getNormal();
  float nDotL = normal.Dot3(dirToLight);
  if (nDotL <= 0.0f)
    return Vec3f(0, 0, 0);

  Vec3f diffuse = componentMultiply(lightColor, diffuseColor) * nDotL;


  Vec3f viewDir = ray.getDirection() * (-1.0f);
  viewDir.Normalize();
  Vec3f halfVector = dirToLight + viewDir;
  halfVector.Normalize();
  float nDotH = normal.Dot3(halfVector);
  if (nDotH <= 0.0f)
    return diffuse;

  float spec = ::powf(nDotH, exponent);

  if (specular_fix)
    spec *= nDotL;
  Vec3f specular = componentMultiply(lightColor, specularColor) * spec;
  return diffuse + specular;
}


auto PhongMaterial::glSetMaterial() const -> void {
  GLfloat one[4] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat zero[4] = { 0.0, 0.0, 0.0, 0.0 };
  GLfloat specular[4] = {
    getSpecularColor(Vec3f(0, 0, 0)).x(),
    getSpecularColor(Vec3f(0, 0, 0)).y(),
    getSpecularColor(Vec3f(0, 0, 0)).z(), 1.0 };
  GLfloat diffuse[4] = {
    getDiffuseColor(Vec3f(0, 0, 0)).x(),
    getDiffuseColor(Vec3f(0, 0, 0)).y(),
    getDiffuseColor(Vec3f(0, 0, 0)).z(), 1.0 };


  float glexponent = exponent;
  if (glexponent < 0) glexponent = 0;
  if (glexponent > 128) glexponent = 128;

#if !SPECULAR_FIX

  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);

#else


  if (SPECULAR_FIX_WHICH_PASS == 0) {
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, zero);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);
  } else if (SPECULAR_FIX_WHICH_PASS == 1) {
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, one);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
  } else {
    assert(SPECULAR_FIX_WHICH_PASS == 2);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
  }

#endif
}
