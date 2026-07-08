#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "vectors.h"
#include "ray.h"
#include "hit.h"

// 材质抽象基类
class Material {

public:
  virtual ~Material() {}

  virtual Vec3f getDiffuseColor() const = 0;
  // 计算单光源下的局部着色
  virtual Vec3f Shade(const Ray &ray, const Hit &hit,
                      const Vec3f &dirToLight,
                      const Vec3f &lightColor) const = 0;
  // 设置材质参数
  virtual void glSetMaterial(void) const = 0;
};

// Phong 材质
class PhongMaterial : public Material {

public:
  PhongMaterial(const Vec3f &diffuseColor, const Vec3f &specularColor,
                float exponent);

  virtual Vec3f getDiffuseColor() const { return diffuseColor; }
  // 获取高光颜色
  Vec3f getSpecularColor() const { return specularColor; }
  // 获取高光指数
  float getExponent() const { return exponent; }

  // 计算单光源下的局部着色
  virtual Vec3f Shade(const Ray &ray, const Hit &hit,
                      const Vec3f &dirToLight,
                      const Vec3f &lightColor) const;
  virtual void glSetMaterial(void) const;

private:
  Vec3f diffuseColor;   // 漫反射颜色 kd
  Vec3f specularColor;  // 高光颜色 ks
  float exponent;       // 高光指数 n
};

#endif
