#include "material.h"
#include "glCanvas.h"
#include "gl_options.h"
#include "gl_headers.h"
#include <math.h>
#include <assert.h>

#ifdef SPECULAR_FIX
extern int SPECULAR_FIX_WHICH_PASS;
#endif

// 颜色向量逐分量相乘（Phong 公式中的 c_light ⊙ c_material）
static Vec3f componentMultiply(const Vec3f &a, const Vec3f &b) {
  return Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

// 构造 Phong 材质
PhongMaterial::PhongMaterial(const Vec3f &diffuse, const Vec3f &specular,
                             float exp, const Vec3f &reflective,
                             const Vec3f &transparent, float ior) {
  diffuseColor = diffuse;
  specularColor = specular;
  exponent = exp;
  reflectiveColor = reflective;  // 反射颜色 kr
  transparentColor = transparent;  // 透明颜色 kt
  indexOfRefraction = ior;  // 折射率
}

Vec3f PhongMaterial::getDiffuseColor(const Vec3f &point) const {
  return diffuseColor;
}

Vec3f PhongMaterial::getSpecularColor(const Vec3f &point) const {
  return specularColor;
}

float PhongMaterial::getExponent(const Vec3f &point) const {
  return exponent;
}

Vec3f PhongMaterial::getReflectiveColor(const Vec3f &point) const {
  return reflectiveColor;
}

Vec3f PhongMaterial::getTransparentColor(const Vec3f &point) const {
  return transparentColor;
}

float PhongMaterial::getIndexOfRefraction(const Vec3f &point) const {
  return indexOfRefraction;
}

Vec3f Material::getSpecularColor(const Vec3f &point) const {
  return Vec3f(0, 0, 0);
}

float Material::getExponent(const Vec3f &point) const {
  return 1.0f;
}

Vec3f Material::getReflectiveColor(const Vec3f &point) const {
  return Vec3f(0, 0, 0);
}

Vec3f Material::getTransparentColor(const Vec3f &point) const {
  return Vec3f(0, 0, 0);
}

float Material::getIndexOfRefraction(const Vec3f &point) const {
  return 1.0f;
}

// 局部着色
// diffuse = (N·L) * c_light ⊙ kd
// specular = (N·H)^n * c_light ⊙ ks，H = normalize(L + V)
// 参考 Assignment 3 的实现说明与 OpenGL 的默认光照模型（使用半角向量 H）
Vec3f PhongMaterial::Shade(const Ray &ray, const Hit &hit,
                           const Vec3f &dirToLight,
                           const Vec3f &lightColor) const {
  Vec3f normal = hit.getNormal();
  float nDotL = normal.Dot3(dirToLight);  // N·L，背光侧 <= 0
  if (nDotL <= 0.0f)
    return Vec3f(0, 0, 0);

  Vec3f diffuse = componentMultiply(lightColor, diffuseColor) * nDotL;  // diffuse = (N·L) * c_light ⊙ kd

  // V：指向相机的单位向量；L：dirToLight
  Vec3f viewDir = ray.getDirection() * (-1.0f);
  viewDir.Normalize();
  Vec3f halfVector = dirToLight + viewDir;
  halfVector.Normalize();  // H = (L + V) / |L + V|，半角向量
  float nDotH = normal.Dot3(halfVector);
  if (nDotH <= 0.0f)
    return diffuse;  // 背光侧 <= 0，返回漫反射

  float spec = powf(nDotH, exponent);  // (N·H)^n，高光
  // fix：specular *= N·L，避免高光瓣在掠射角的 artifact
  if (specular_fix)
    spec *= nDotL;
  Vec3f specular = componentMultiply(lightColor, specularColor) * spec;  // specular = (N·H)^n * c_light ⊙ ks
  return diffuse + specular;  // 返回漫反射 + 高光
}

// 设置 OpenGL 材质
void PhongMaterial::glSetMaterial(void) const {
  GLfloat one[4] = { 1.0, 1.0, 1.0, 1.0 };  // 白色
  GLfloat zero[4] = { 0.0, 0.0, 0.0, 0.0 };  // 黑色
  GLfloat specular[4] = {
    getSpecularColor(Vec3f(0, 0, 0)).x(),
    getSpecularColor(Vec3f(0, 0, 0)).y(),
    getSpecularColor(Vec3f(0, 0, 0)).z(), 1.0 };
  GLfloat diffuse[4] = {
    getDiffuseColor(Vec3f(0, 0, 0)).x(),
    getDiffuseColor(Vec3f(0, 0, 0)).y(),
    getDiffuseColor(Vec3f(0, 0, 0)).z(), 1.0 };

  // OpenGL shininess 有效范围 [0, 128]
  float glexponent = exponent;  // 高光指数
  if (glexponent < 0) glexponent = 0;
  if (glexponent > 128) glexponent = 128;

#if !SPECULAR_FIX

  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);  // 漫反射颜色
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);  // 环境光颜色
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);  // 高光颜色
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);  // 高光指数

#else

  // 3-pass 渲染修复 OpenGL 端掠射角高光 artifact（参考 material_additions.txt）
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
