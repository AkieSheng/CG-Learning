#include "material.h"
#include <cmath>

extern bool specular_fix;

static Vec3f componentMultiply(Vec3f const&a, Vec3f const&b)
{
  return Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

PhongMaterial::PhongMaterial(Vec3f const&diffuse, Vec3f const&specular,
                             float exp, Vec3f const&reflective,
                             Vec3f const&transparent, float ior)
{
  diffuseColor = diffuse;
  specularColor = specular;
  exponent = exp;
  reflectiveColor = reflective;
  transparentColor = transparent;
  indexOfRefraction = ior;
}

Vec3f PhongMaterial::getDiffuseColor(Vec3f const&point) const
{
  return diffuseColor;
}

Vec3f PhongMaterial::getSpecularColor(Vec3f const&point) const
{
  return specularColor;
}

float PhongMaterial::getExponent(Vec3f const&point) const
{
  return exponent;
}

Vec3f PhongMaterial::getReflectiveColor(Vec3f const&point) const
{
  return reflectiveColor;
}

Vec3f PhongMaterial::getTransparentColor(Vec3f const&point) const
{
  return transparentColor;
}

float PhongMaterial::getIndexOfRefraction(Vec3f const&point) const
{
  return indexOfRefraction;
}

Vec3f Material::getSpecularColor(Vec3f const&point) const
{
  return Vec3f(0, 0, 0);
}

float Material::getExponent(Vec3f const&point) const
{
  return 1.0f;
}

Vec3f Material::getReflectiveColor(Vec3f const&point) const
{
  return Vec3f(0, 0, 0);
}

Vec3f Material::getTransparentColor(Vec3f const&point) const
{
  return Vec3f(0, 0, 0);
}

float Material::getIndexOfRefraction(Vec3f const&point) const
{
  return 1.0f;
}

Vec3f PhongMaterial::Shade(Ray const&ray, Hit const&hit,
                           Vec3f const&dirToLight,
                           Vec3f const&lightColor) const {
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
