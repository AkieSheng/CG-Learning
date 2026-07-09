#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "vectors.h"
#include "ray.h"
#include "hit.h"

// 材质抽象基类
class Material {

public:
  virtual ~Material() {}

  virtual Vec3f getDiffuseColor(const Vec3f &point) const = 0;
  virtual Vec3f getSpecularColor(const Vec3f &point) const;
  virtual float getExponent(const Vec3f &point) const;
  virtual Vec3f getReflectiveColor(const Vec3f &point) const;
  virtual Vec3f getTransparentColor(const Vec3f &point) const;
  virtual float getIndexOfRefraction(const Vec3f &point) const;

  virtual Vec3f Shade(const Ray &ray, const Hit &hit,
                      const Vec3f &dirToLight,
                      const Vec3f &lightColor) const = 0;
  virtual void glSetMaterial(void) const = 0;
};

// Phong 材质
class PhongMaterial : public Material {

public:
  PhongMaterial(const Vec3f &diffuseColor, const Vec3f &specularColor,
                float exponent, const Vec3f &reflectiveColor,
                const Vec3f &transparentColor, float indexOfRefraction);

  virtual Vec3f getDiffuseColor(const Vec3f &point) const;
  virtual Vec3f getSpecularColor(const Vec3f &point) const;
  virtual float getExponent(const Vec3f &point) const;
  virtual Vec3f getReflectiveColor(const Vec3f &point) const;
  virtual Vec3f getTransparentColor(const Vec3f &point) const;
  virtual float getIndexOfRefraction(const Vec3f &point) const;

  virtual Vec3f Shade(const Ray &ray, const Hit &hit,
                      const Vec3f &dirToLight,
                      const Vec3f &lightColor) const;
  virtual void glSetMaterial(void) const;

private:
  Vec3f diffuseColor;  // 漫反射颜色 kd
  Vec3f specularColor;  // 高光颜色 ks
  float exponent;  // 高光指数 n
  Vec3f reflectiveColor;  // 反射颜色 kr
  Vec3f transparentColor;  // 透明颜色 kt
  float indexOfRefraction;  // 折射率
};

#endif
