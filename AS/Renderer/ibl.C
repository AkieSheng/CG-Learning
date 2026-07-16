#include "ibl.h"
#include "gl_headers.h"

#include <math.h>
#include <stdio.h>
#include <vector>

static const int ENV_SIZE = 128;
static const int IRRADIANCE_SIZE = 32;
static const int PREFILTER_SIZE = 128;
static const int PREFILTER_SAMPLE_COUNT = 64;
static const int BRDF_LUT_SIZE = 512;
static const float PI = 3.14159265359f;

// 平滑工作室渐变（天空盒可见 + 辐照度漫反射）
static void proceduralSkyDiffuse(float dx, float dy, float dz,
                                 float *r, float *g, float *b) {
  float len = sqrtf(dx * dx + dy * dy + dz * dz);
  if (len < 1e-6f) len = 1.0f;
  dx /= len; dy /= len; dz /= len;

  float t = 0.5f * (dy + 1.0f);
  float ground = 0.055f;
  float sky    = 0.10f;
  float base = ground * (1.0f - t) + sky * t;
  *r = base * 1.02f;
  *g = base;
  *b = base * 0.98f;

  // 极弱宽填充：与主光同向，作环境底
  float kx = -0.54f, ky = 0.50f, kz = 0.68f;
  float klen = sqrtf(kx * kx + ky * ky + kz * kz);
  kx /= klen; ky /= klen; kz /= klen;
  float keyDot = dx * kx + dy * ky + dz * kz;
  if (keyDot > 0.0f) {
    float keyWide = powf(keyDot, 8.0f) * 0.06f;
    *r += keyWide;
    *g += keyWide;
    *b += keyWide * 1.02f;
  }
}

// 天空盒显示：暗灰平滑渐变
static void proceduralSkyDisplay(float dx, float dy, float dz,
                                 float *r, float *g, float *b) {
  float len = sqrtf(dx * dx + dy * dy + dz * dz);
  if (len < 1e-6f) len = 1.0f;
  dy /= len;

  float t = 0.5f * (dy + 1.0f);
  float ground = 0.11f;
  float sky    = 0.20f;
  float base = ground * (1.0f - t) + sky * t;
  *r = base * 0.98f;
  *g = base;
  *b = base * 1.02f;
}

// 照明 HDR：漫反射底 + 强对比镜面亮斑
static void proceduralSkyLighting(float dx, float dy, float dz,
                                  float *r, float *g, float *b) {
  proceduralSkyDiffuse(dx, dy, dz, r, g, b);

  float len = sqrtf(dx * dx + dy * dy + dz * dz);
  if (len < 1e-6f) len = 1.0f;
  dx /= len; dy /= len; dz /= len;

  // 主光：降低高度以拉长阴影；偏左、偏初始相机侧（+Z）
  // 与 Renderer 的固定方向光一致，作为同一盏工作室灯的反射亮斑。
  float kx = -0.54f, ky = 0.50f, kz = 0.68f;
  float klen = sqrtf(kx * kx + ky * ky + kz * kz);
  kx /= klen; ky /= klen; kz /= klen;
  float keyDot = dx * kx + dy * ky + dz * kz;
  if (keyDot > 0.0f) {
    float keySharp = powf(keyDot, 140.0f) * 36.0f;
    *r += keySharp;
    *g += keySharp;
    *b += keySharp * 1.05f;
  }

  // 背光轮廓：保留背面金属可读性
  float bx = 0.0f, by = 0.55f, bz = -0.72f;
  float blen = sqrtf(bx * bx + by * by + bz * bz);
  bx /= blen; by /= blen; bz /= blen;
  float backDot = dx * bx + dy * by + dz * bz;
  if (backDot > 0.0f) {
    float backSharp = powf(backDot, 90.0f) * 4.0f;
    *r += backSharp;
    *g += backSharp;
    *b += backSharp * 1.03f;
  }
}

// 将 UV 转换为方向向量
static void cubemapUVToDir(int face, float u, float v, float *dx, float *dy, float *dz) {
  float s = 2.0f * u - 1.0f;
  float t = 2.0f * v - 1.0f;
  switch (face) {
    case 0: *dx =  1.0f; *dy = -t;   *dz = -s;   break;
    case 1: *dx = -1.0f; *dy = -t;   *dz =  s;   break;
    case 2: *dx =  s;    *dy =  1.0f; *dz =  t;   break;
    case 3: *dx =  s;    *dy = -1.0f; *dz = -t;   break;
    case 4: *dx =  s;    *dy = -t;   *dz =  1.0f; break;
    default:*dx = -s;    *dy = -t;   *dz = -1.0f; break;
  }
}

// 计算两个向量的叉积
static void cross3(float ax, float ay, float az,
                   float bx, float by, float bz,
                   float *cx, float *cy, float *cz) {
  *cx = ay * bz - az * by;
  *cy = az * bx - ax * bz;
  *cz = ax * by - ay * bx;
}

// 归一化一个向量
static void normalize3(float *x, float *y, float *z) {
  float len = sqrtf((*x) * (*x) + (*y) * (*y) + (*z) * (*z));
  if (len > 1e-6f) { *x /= len; *y /= len; *z /= len; }
}

// GGX Split-Sum 预滤波
static float radicalInverseVdC(unsigned int bits) {
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return (float)bits * 2.3283064365386963e-10f;
}

static void hammersley(unsigned int i, unsigned int n, float *u, float *v) {
  *u = (float)i / (float)n;
  *v = radicalInverseVdC(i);
}

static void importanceSampleGGX(float xi0, float xi1,
                                float nx, float ny, float nz,
                                float roughness,
                                float *hx, float *hy, float *hz) {
  float a = roughness * roughness;
  float phi = 2.0f * PI * xi0;
  float cosTheta = sqrtf((1.0f - xi1) / (1.0f + (a * a - 1.0f) * xi1));
  float sinTheta = sqrtf(fmaxf(1.0f - cosTheta * cosTheta, 0.0f));

  float hxLocal = cosf(phi) * sinTheta;
  float hyLocal = sinf(phi) * sinTheta;
  float hzLocal = cosTheta;

  float upx = 0.0f, upy = 0.0f, upz = 1.0f;
  if (fabsf(nz) >= 0.999f) {
    upx = 1.0f; upy = 0.0f; upz = 0.0f;
  }
  float tx, ty, tz;
  cross3(upx, upy, upz, nx, ny, nz, &tx, &ty, &tz);
  normalize3(&tx, &ty, &tz);
  float bx, by, bz;
  cross3(nx, ny, nz, tx, ty, tz, &bx, &by, &bz);

  *hx = tx * hxLocal + bx * hyLocal + nx * hzLocal;
  *hy = ty * hxLocal + by * hyLocal + ny * hzLocal;
  *hz = tz * hxLocal + bz * hyLocal + nz * hzLocal;
  normalize3(hx, hy, hz);
}

// 对反射方向 R 做 GGX 镜面预滤波（roughness=0 时退化为环境采样）
static void convolvePrefilter(float rx, float ry, float rz, float roughness,
                              float *outR, float *outG, float *outB) {
  normalize3(&rx, &ry, &rz);

  if (roughness < 1e-4f) {
    proceduralSkyLighting(rx, ry, rz, outR, outG, outB);
    return;
  }

  float ir = 0.0f, ig = 0.0f, ib = 0.0f;
  float totalWeight = 0.0f;

  for (unsigned int i = 0; i < (unsigned int)PREFILTER_SAMPLE_COUNT; i++) {
    float xi0, xi1;
    hammersley(i, (unsigned int)PREFILTER_SAMPLE_COUNT, &xi0, &xi1);

    float hx, hy, hz;
    importanceSampleGGX(xi0, xi1, rx, ry, rz, roughness, &hx, &hy, &hz);

    // L = reflect(-V, H)；Split-Sum 取 V = R = N
    float vdh = rx * hx + ry * hy + rz * hz;
    float lx = 2.0f * vdh * hx - rx;
    float ly = 2.0f * vdh * hy - ry;
    float lz = 2.0f * vdh * hz - rz;
    normalize3(&lx, &ly, &lz);

    float ndotl = rx * lx + ry * ly + rz * lz;
    if (ndotl > 0.0f) {
      float sr, sg, sb;
      proceduralSkyLighting(lx, ly, lz, &sr, &sg, &sb);
      ir += sr * ndotl;
      ig += sg * ndotl;
      ib += sb * ndotl;
      totalWeight += ndotl;
    }
  }

  if (totalWeight > 1e-6f) {
    *outR = ir / totalWeight;
    *outG = ig / totalWeight;
    *outB = ib / totalWeight;
  } else {
    proceduralSkyLighting(rx, ry, rz, outR, outG, outB);
  }
}

// 卷积辐照度
static void convolveIrradiance(float nx, float ny, float nz,
                               float *r, float *g, float *b) {
  float upx = 0.0f, upy = 1.0f, upz = 0.0f;
  float rx, ry, rz;
  cross3(upy, upz, 0.0f, nx, ny, nz, &rx, &ry, &rz);
  float rlen = sqrtf(rx * rx + ry * ry + rz * rz);
  if (rlen < 1e-4f) {
    upx = 1.0f; upy = 0.0f; upz = 0.0f;
    cross3(upx, upy, upz, nx, ny, nz, &rx, &ry, &rz);
  }
  normalize3(&rx, &ry, &rz);
  float ux, uy, uz;
  cross3(nx, ny, nz, rx, ry, rz, &ux, &uy, &uz);
  normalize3(&ux, &uy, &uz);

  float ir = 0.0f, ig = 0.0f, ib = 0.0f;
  float sampleDelta = 0.04f;
  int nrSamples = 0;
  for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta) {
    for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta) {
      float sinT = sinf(theta);
      float cosT = cosf(theta);
      float sinP = sinf(phi);
      float cosP = cosf(phi);
      float sx = cosP * sinT * rx + sinP * sinT * ux + cosT * nx;
      float sy = cosP * sinT * ry + sinP * sinT * uy + cosT * ny;
      float sz = cosP * sinT * rz + sinP * sinT * uz + cosT * nz;
      float sr, sg, sb;
      proceduralSkyDiffuse(sx, sy, sz, &sr, &sg, &sb);
      ir += sr * cosT * sinT;
      ig += sg * cosT * sinT;
      ib += sb * cosT * sinT;
      nrSamples++;
    }
  }
  float scale = PI / (float)nrSamples;
  *r = ir * scale;
  *g = ig * scale;
  *b = ib * scale;
}

IBL::IBL()
  : envCubemap(0),
    skyboxCubemap(0),
    irradianceMap(0),
    prefilterMap(0),
    brdfLUT(0),
    skyboxVAO(0),
    skyboxVBO(0),
    captureFBO(0),
    maxReflectionLod_(0.0f) {}

IBL::~IBL() {
  destroy();
}

void IBL::destroy() {
  brdfLUTShader.destroy();
  if (skyboxVBO) { glDeleteBuffers(1, &skyboxVBO); skyboxVBO = 0; }
  if (skyboxVAO) { glDeleteVertexArrays(1, &skyboxVAO); skyboxVAO = 0; }
  if (captureFBO) { glDeleteFramebuffers(1, &captureFBO); captureFBO = 0; }
  if (brdfLUT) { glDeleteTextures(1, &brdfLUT); brdfLUT = 0; }
  if (prefilterMap && prefilterMap != envCubemap) {
    glDeleteTextures(1, &prefilterMap);
  }
  prefilterMap = 0;
  if (irradianceMap) { glDeleteTextures(1, &irradianceMap); irradianceMap = 0; }
  if (envCubemap) { glDeleteTextures(1, &envCubemap); envCubemap = 0; }
  if (skyboxCubemap) { glDeleteTextures(1, &skyboxCubemap); skyboxCubemap = 0; }
}

// 照明 HDR cubemap（窄高光，供 prefilter 镜面采样）
bool IBL::createEnvironmentCubemap() {
  glGenTextures(1, &envCubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  std::vector<float> pixels((size_t)ENV_SIZE * ENV_SIZE * 3);
  for (int face = 0; face < 6; face++) {
    for (int y = 0; y < ENV_SIZE; y++) {
      for (int x = 0; x < ENV_SIZE; x++) {
        float u = ((float)x + 0.5f) / (float)ENV_SIZE;
        float v = ((float)y + 0.5f) / (float)ENV_SIZE;
        float dx, dy, dz, r, g, b;
        cubemapUVToDir(face, u, v, &dx, &dy, &dz);
        proceduralSkyLighting(dx, dy, dz, &r, &g, &b);
        size_t idx = (size_t)(y * ENV_SIZE + x) * 3;
        pixels[idx + 0] = r;
        pixels[idx + 1] = g;
        pixels[idx + 2] = b;
      }
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F,
                 ENV_SIZE, ENV_SIZE, 0, GL_RGB, GL_FLOAT, &pixels[0]);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return true;
}

// 天空盒显示 cubemap（平滑暗灰，无灯珠）
bool IBL::createSkyboxCubemap() {
  glGenTextures(1, &skyboxCubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);

  std::vector<unsigned char> pixels((size_t)ENV_SIZE * ENV_SIZE * 3);
  for (int face = 0; face < 6; face++) {
    for (int y = 0; y < ENV_SIZE; y++) {
      for (int x = 0; x < ENV_SIZE; x++) {
        float u = ((float)x + 0.5f) / (float)ENV_SIZE;
        float v = ((float)y + 0.5f) / (float)ENV_SIZE;
        float dx, dy, dz, r, g, b;
        cubemapUVToDir(face, u, v, &dx, &dy, &dz);
        proceduralSkyDisplay(dx, dy, dz, &r, &g, &b);
        size_t idx = (size_t)(y * ENV_SIZE + x) * 3;
        pixels[idx + 0] = (unsigned char)(fminf(r * 255.0f, 255.0f));
        pixels[idx + 1] = (unsigned char)(fminf(g * 255.0f, 255.0f));
        pixels[idx + 2] = (unsigned char)(fminf(b * 255.0f, 255.0f));
      }
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB,
                 ENV_SIZE, ENV_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return true;
}

// 创建辐照度卷积 cubemap
bool IBL::createIrradianceMap() {
  glGenTextures(1, &irradianceMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

  std::vector<float> pixels((size_t)IRRADIANCE_SIZE * IRRADIANCE_SIZE * 3);
  for (int face = 0; face < 6; face++) {
    for (int y = 0; y < IRRADIANCE_SIZE; y++) {
      for (int x = 0; x < IRRADIANCE_SIZE; x++) {
        float u = ((float)x + 0.5f) / (float)IRRADIANCE_SIZE;
        float v = ((float)y + 0.5f) / (float)IRRADIANCE_SIZE;
        float dx, dy, dz, r, g, b;
        cubemapUVToDir(face, u, v, &dx, &dy, &dz);
        convolveIrradiance(dx, dy, dz, &r, &g, &b);
        size_t idx = (size_t)(y * IRRADIANCE_SIZE + x) * 3;
        pixels[idx + 0] = r;
        pixels[idx + 1] = g;
        pixels[idx + 2] = b;
      }
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F,
                 IRRADIANCE_SIZE, IRRADIANCE_SIZE, 0, GL_RGB, GL_FLOAT, &pixels[0]);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return true;
}

// 创建 GGX 镜面预滤波 cubemap（mip 对应 roughness，供 Split-Sum 采样）
bool IBL::createPrefilterMap() {
  int mipLevels = 1;
  int size = PREFILTER_SIZE;
  while (size > 1) {
    mipLevels++;
    size /= 2;
  }
  maxReflectionLod_ = (float)(mipLevels - 1);

  glGenTextures(1, &prefilterMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

  size = PREFILTER_SIZE;
  for (int mip = 0; mip < mipLevels; mip++) {
    for (int face = 0; face < 6; face++) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, GL_RGB16F,
                   size, size, 0, GL_RGB, GL_FLOAT, NULL);
    }
    size = size > 1 ? size / 2 : 1;
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  size = PREFILTER_SIZE;
  for (int mip = 0; mip < mipLevels; mip++) {
    // mip 0 → roughness 0（镜面），最高 mip → roughness 1
    float roughness = (mipLevels > 1)
      ? (float)mip / (float)(mipLevels - 1)
      : 0.0f;

    std::vector<float> pixels((size_t)size * (size_t)size * 3);
    for (int face = 0; face < 6; face++) {
      for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
          float u = ((float)x + 0.5f) / (float)size;
          float v = ((float)y + 0.5f) / (float)size;
          float dx, dy, dz, r, g, b;
          cubemapUVToDir(face, u, v, &dx, &dy, &dz);
          convolvePrefilter(dx, dy, dz, roughness, &r, &g, &b);
          size_t idx = (size_t)(y * size + x) * 3;
          pixels[idx + 0] = r;
          pixels[idx + 1] = g;
          pixels[idx + 2] = b;
        }
      }
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, GL_RGB16F,
                   size, size, 0, GL_RGB, GL_FLOAT, &pixels[0]);
    }
    size = size > 1 ? size / 2 : 1;
  }

  fprintf(stderr, "IBL: GGX prefilter %dx%d, %d mips, %d samples/texel\n",
          PREFILTER_SIZE, PREFILTER_SIZE, mipLevels, PREFILTER_SAMPLE_COUNT);
  return true;
}

// 创建 BRDF LUT
bool IBL::createBrdfLUT() {
  if (!brdfLUTShader.loadFromFiles("Shader/brdf_lut.vert", "Shader/brdf_lut.frag")) {
    fprintf(stderr, "IBL: failed to load BRDF LUT shader\n");
    return false;
  }

  glGenTextures(1, &brdfLUT);
  glBindTexture(GL_TEXTURE_2D, brdfLUT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, BRDF_LUT_SIZE, BRDF_LUT_SIZE,
               0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenFramebuffers(1, &captureFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "IBL: BRDF LUT framebuffer incomplete\n");
    return false;
  }

  GLint prevViewport[4];
  glGetIntegerv(GL_VIEWPORT, prevViewport);

  glViewport(0, 0, BRDF_LUT_SIZE, BRDF_LUT_SIZE);
  brdfLUTShader.use();
  glDisable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glEnable(GL_DEPTH_TEST);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
  return true;
}

// 创建天空盒 VAO
bool IBL::createSkyboxVAO() {
  float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
  };

  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glBindVertexArray(0);
  return true;
}

// 初始化 IBL
bool IBL::initialize() {
  destroy();
  if (!createEnvironmentCubemap()) return false;
  if (!createSkyboxCubemap()) return false;
  if (!createIrradianceMap()) return false;
  if (!createPrefilterMap()) return false;
  if (!createBrdfLUT()) return false;
  if (!createSkyboxVAO()) return false;
  fprintf(stderr,
          "IBL: initialized (env %dx%d, irradiance %d, prefilter %dx%d lod=%.0f, BRDF LUT %d)\n",
          ENV_SIZE, ENV_SIZE, IRRADIANCE_SIZE,
          PREFILTER_SIZE, PREFILTER_SIZE, maxReflectionLod_, BRDF_LUT_SIZE);
  return true;
}

// 渲染天空盒
void IBL::renderSkybox(const ShaderProgram &skyboxShader,
                       const float *view, const float *projection) const {
  if (!valid()) return;

  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);

  skyboxShader.use();
  skyboxShader.setMat4("uView", view);
  skyboxShader.setMat4("uProjection", projection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
  skyboxShader.setInt("uEnvironmentMap", 0);

  glBindVertexArray(skyboxVAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);

  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
}

// 绑定 IBL 环境光
void IBL::bindForPBR(ShaderProgram &pbrShader) const {
  if (!valid()) return;

  glActiveTexture(GL_TEXTURE0 + IRRADIANCE_UNIT);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  pbrShader.setInt("uIrradianceMap", IRRADIANCE_UNIT);

  glActiveTexture(GL_TEXTURE0 + PREFILTER_UNIT);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  pbrShader.setInt("uPrefilterMap", PREFILTER_UNIT);

  glActiveTexture(GL_TEXTURE0 + BRDF_LUT_UNIT);
  glBindTexture(GL_TEXTURE_2D, brdfLUT);
  pbrShader.setInt("uBrdfLUT", BRDF_LUT_UNIT);

  pbrShader.setBool("uUseIBL", true);
  pbrShader.setFloat("uMaxReflectionLOD", maxReflectionLod_);
}
