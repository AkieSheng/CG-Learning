#include "renderer.h"
#include "ibl.h"
#include "gl_headers.h"
#include <stdio.h>
#include <vector>
#include <algorithm>

Renderer::Renderer()
  : sceneFBO(0),
    sceneColorTex(0),
    sceneDepthRbo(0),
    sceneSampleTex(0),
    fullscreenVAO(0),
    viewportWidth(800),
    viewportHeight(600),
    lightDirection(0.40f, -0.72f, -0.56f),
    lightColor(1.0f, 1.0f, 1.0f),
    ambientColor(0.03f, 0.03f, 0.035f) {}

Renderer::~Renderer() {
  destroy();
}

bool Renderer::initialize(int width, int height) {
  viewportWidth = width;
  viewportHeight = height;
  setupGLState();
  if (!loadShaders()) {
    return false;
  }
  if (!createSceneTargets(width, height)) {
    fprintf(stderr, "Renderer: failed to create scene targets\n");
    return false;
  }
  glGenVertexArrays(1, &fullscreenVAO);
  glViewport(0, 0, width, height);
  return true;
}

void Renderer::resize(int width, int height) {
  if (width <= 0 || height <= 0) return;
  viewportWidth = width;
  viewportHeight = height;
  createSceneTargets(width, height);
  glViewport(0, 0, width, height);
}

void Renderer::setupGLState() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
}

bool Renderer::loadShaders() {
  if (!pbrShader.loadFromFiles("Shader/pbr.vert", "Shader/pbr.frag")) {
    fprintf(stderr, "Renderer: failed to load PBR shader\n");
    return false;
  }
  if (!skyboxShader.loadFromFiles("Shader/skybox.vert", "Shader/skybox.frag")) {
    fprintf(stderr, "Renderer: failed to load skybox shader\n");
    return false;
  }
  if (!tonemapShader.loadFromFiles("Shader/tonemap.vert", "Shader/tonemap.frag")) {
    fprintf(stderr, "Renderer: failed to load tonemap shader\n");
    return false;
  }
  if (!ibl.initialize()) {
    fprintf(stderr, "Renderer: failed to initialize IBL\n");
    return false;
  }
  return true;
}

void Renderer::destroySceneTargets() {
  if (sceneFBO) { glDeleteFramebuffers(1, &sceneFBO); sceneFBO = 0; }
  if (sceneColorTex) { glDeleteTextures(1, &sceneColorTex); sceneColorTex = 0; }
  if (sceneDepthRbo) { glDeleteRenderbuffers(1, &sceneDepthRbo); sceneDepthRbo = 0; }
  if (sceneSampleTex) { glDeleteTextures(1, &sceneSampleTex); sceneSampleTex = 0; }
}

bool Renderer::createSceneTargets(int width, int height) {
  destroySceneTargets();
  if (width <= 0 || height <= 0) return false;

  glGenTextures(1, &sceneColorTex);
  glBindTexture(GL_TEXTURE_2D, sceneColorTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // 深度用 renderbuffer，比 depth texture 在 freeglut/兼容配置下更稳
  glGenRenderbuffers(1, &sceneDepthRbo);
  glBindRenderbuffer(GL_RENDERBUFFER, sceneDepthRbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

  // 折射采样源：先只分配 base level；拷贝后再 generateMipmap
  glGenTextures(1, &sceneSampleTex);
  glBindTexture(GL_TEXTURE_2D, sceneSampleTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenFramebuffers(1, &sceneFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTex, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneDepthRbo);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "Renderer: scene framebuffer incomplete (0x%X)\n", (unsigned)status);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    destroySceneTargets();
    return false;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  return true;
}

void Renderer::setLightDirection(const Vec3f &dir) {
  lightDirection = dir;
  lightDirection.Normalize();
}

void Renderer::setLightColor(const Vec3f &color) {
  lightColor = color;
}

void Renderer::setAmbientColor(const Vec3f &color) {
  ambientColor = color;
}

bool Renderer::isTransparentMesh(const Mesh *mesh) {
  if (!mesh) return false;
  const PBRMaterial *mat = mesh->getMaterial();
  if (!mat) return false;
  if (mat->alphaMode == ALPHA_BLEND) return true;
  if (mat->hasTransmission && mat->transmissionFactor > 0.001f) return true;
  return false;
}

float Renderer::meshSortKey(const Mesh *mesh, const Vec3f &camPos) {
  const Vec3f &c = mesh->getWorldCenter();
  float dx = c.x() - camPos.x();
  float dy = c.y() - camPos.y();
  float dz = c.z() - camPos.z();
  return dx * dx + dy * dy + dz * dz;
}

static float modelDeterminant3x3(const Matrix &m) {
  return
    m.Get(0, 0) * (m.Get(1, 1) * m.Get(2, 2) - m.Get(2, 1) * m.Get(1, 2)) -
    m.Get(1, 0) * (m.Get(0, 1) * m.Get(2, 2) - m.Get(2, 1) * m.Get(0, 2)) +
    m.Get(2, 0) * (m.Get(0, 1) * m.Get(1, 2) - m.Get(1, 1) * m.Get(0, 2));
}

void Renderer::drawSingleMesh(Mesh *mesh, bool transparentPass) {
  if (!mesh) return;

  PBRMaterial *mat = mesh->getMaterial();
  float *modelPtr = mesh->getModelMatrix().glGet();
  pbrShader.setMat4("uModel", modelPtr);

  // 法线矩阵 = (M^{-1})^T；OpenGL mat3 为列主序，调整到与 Matrix::glGet 一致
  // Get(x,y) → data[y][x]，故列 j 为 Get(j,0), Get(j,1), Get(j,2)
  Matrix normalMat = mesh->getModelMatrix();
  normalMat.Inverse();
  normalMat.Transpose();
  float nm[9] = {
    normalMat.Get(0, 0), normalMat.Get(0, 1), normalMat.Get(0, 2),
    normalMat.Get(1, 0), normalMat.Get(1, 1), normalMat.Get(1, 2),
    normalMat.Get(2, 0), normalMat.Get(2, 1), normalMat.Get(2, 2)
  };
  glUniformMatrix3fv(glGetUniformLocation(pbrShader.programId(), "uNormalMatrix"),
                     1, GL_FALSE, nm);
  delete [] modelPtr;

  if (mat) {
    if (mat->doubleSided || transparentPass) {
      glDisable(GL_CULL_FACE);
    } else {
      glEnable(GL_CULL_FACE);
      // glTF：负缩放会反转三角形绕序
      if (modelDeterminant3x3(mesh->getModelMatrix()) < 0.0f) {
        glFrontFace(GL_CW);
      } else {
        glFrontFace(GL_CCW);
      }
    }

    if (transparentPass) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDepthMask(GL_FALSE);
    } else {
      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
    }

    mat->bindTextures(pbrShader);
    mat->setUniforms(pbrShader);
  } else if (transparentPass) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
  } else {
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
  }

  mesh->draw();
}

void Renderer::bindCommonPBRUniforms(Scene &scene) {
  OrbitCamera &cam = scene.getCamera();
  cam.setAspect((float)viewportWidth / (float)viewportHeight);
  cam.updateMatrices();

  pbrShader.use();
  pbrShader.setBool("uOutputLinear", true);

  float *viewPtr = cam.getViewMatrix().glGet();
  float *projPtr = cam.getProjectionMatrix().glGet();
  pbrShader.setMat4("uView", viewPtr);
  pbrShader.setMat4("uProjection", projPtr);
  delete [] viewPtr;
  delete [] projPtr;

  Vec3f camPos = cam.getPosition();
  pbrShader.setVec3("uCameraPos", camPos.x(), camPos.y(), camPos.z());
  pbrShader.setVec3("uLightDir", lightDirection.x(), lightDirection.y(), lightDirection.z());
  pbrShader.setVec3("uLightColor", lightColor.x(), lightColor.y(), lightColor.z());
  pbrShader.setVec3("uAmbientColor", ambientColor.x(), ambientColor.y(), ambientColor.z());
  pbrShader.setFloat("uDirectLightScale", 1.0f);
  pbrShader.setFloat("uDiffuseEnvScale", 1.0f);
  pbrShader.setFloat("uSpecularEnvScale", 1.0f);
  pbrShader.setFloat("uHemiFillScale", 0.20f);
  pbrShader.setVec2("uFramebufferSize", (float)viewportWidth, (float)viewportHeight);
  ibl.bindForPBR(pbrShader);
}

void Renderer::drawMeshes(Scene &scene, bool transparentPassOnly,
                          const std::vector<Mesh *> *opaque,
                          const std::vector<Mesh *> *transparent) {
  bindCommonPBRUniforms(scene);

  if (!transparentPassOnly && opaque) {
    pbrShader.setBool("uHasSceneColor", false);
    for (size_t i = 0; i < opaque->size(); i++) {
      drawSingleMesh((*opaque)[i], false);
    }
  }

  if (transparentPassOnly && transparent) {
    glActiveTexture(GL_TEXTURE0 + SCENE_SAMPLE_UNIT);
    glBindTexture(GL_TEXTURE_2D, sceneSampleTex);
    pbrShader.setInt("uSceneColorMap", SCENE_SAMPLE_UNIT);
    pbrShader.setBool("uHasSceneColor", true);

    for (size_t i = 0; i < transparent->size(); i++) {
      drawSingleMesh((*transparent)[i], true);
    }
  }

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void Renderer::captureSceneColorSample() {
  glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
  glBindTexture(GL_TEXTURE_2D, sceneSampleTex);
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, viewportWidth, viewportHeight);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

void Renderer::blitTonemapToScreen() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, viewportWidth, viewportHeight);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);

  tonemapShader.use();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, sceneColorTex);
  tonemapShader.setInt("uHdrColor", 0);

  glBindVertexArray(fullscreenVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  glEnable(GL_DEPTH_TEST);
}

void Renderer::render(Scene &scene) {
  OrbitCamera &cam = scene.getCamera();
  cam.setAspect((float)viewportWidth / (float)viewportHeight);
  cam.updateMatrices();

  const std::vector<Mesh *> &meshes = scene.getMeshes();
  std::vector<Mesh *> opaque;
  std::vector<Mesh *> transparent;
  opaque.reserve(meshes.size());
  transparent.reserve(meshes.size());

  for (size_t i = 0; i < meshes.size(); i++) {
    Mesh *mesh = meshes[i];
    if (!mesh) continue;
    if (isTransparentMesh(mesh)) {
      transparent.push_back(mesh);
    } else {
      opaque.push_back(mesh);
    }
  }

  Vec3f camPos = cam.getPosition();
  std::sort(transparent.begin(), transparent.end(),
    [&camPos](const Mesh *a, const Mesh *b) {
      return meshSortKey(a, camPos) > meshSortKey(b, camPos);
    });

  // 1) 天空盒 + 不透明 → 线性 HDR FBO
  glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
  glViewport(0, 0, viewportWidth, viewportHeight);

  Vec3f bg = scene.getBackgroundColor();
  glClearColor(bg.x(), bg.y(), bg.z(), 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float *viewPtr = cam.getViewMatrix().glGet();
  float *projPtr = cam.getProjectionMatrix().glGet();
  skyboxShader.use();
  skyboxShader.setBool("uOutputLinear", true);
  ibl.renderSkybox(skyboxShader, viewPtr, projPtr);
  delete [] viewPtr;
  delete [] projPtr;

  drawMeshes(scene, false, &opaque, NULL);

  // 2) 拷贝不透明场景供折射采样
  if (!transparent.empty()) {
    captureSceneColorSample();
    drawMeshes(scene, true, NULL, &transparent);
  }

  // 3) HDR → ACES + sRGB 到默认帧缓冲
  blitTonemapToScreen();
}

void Renderer::destroy() {
  if (fullscreenVAO) {
    glDeleteVertexArrays(1, &fullscreenVAO);
    fullscreenVAO = 0;
  }
  destroySceneTargets();
  ibl.destroy();
  tonemapShader.destroy();
  pbrShader.destroy();
  skyboxShader.destroy();
}
