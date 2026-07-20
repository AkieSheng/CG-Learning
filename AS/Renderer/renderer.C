#include "renderer.h"
#include "ibl.h"
#include "gl_headers.h"
#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>

static float modelDeterminant3x3(Matrix const&m) {
  return
    m.Get(0, 0) * (m.Get(1, 1) * m.Get(2, 2) - m.Get(2, 1) * m.Get(1, 2)) -
    m.Get(1, 0) * (m.Get(0, 1) * m.Get(2, 2) - m.Get(2, 1) * m.Get(0, 2)) +
    m.Get(2, 0) * (m.Get(0, 1) * m.Get(1, 2) - m.Get(1, 1) * m.Get(0, 2));
}

Renderer::Renderer()
  : msaaFBO(0),
    msaaColorRbo(0),
    msaaDepthRbo(0),
    msaaSamples(0),
    resolveFBO(0),
    sceneColorTex(0),
    sceneSampleTex(0),
    sceneDepthRbo(0),
    shadowFBO(0),
    shadowDepthTex(0),
    shadowDepthRbo(0),
    shadowMapSize(DEFAULT_SHADOW_MAP_SIZE),
    shadowsEnabled(false),
    fullscreenVAO(0),
    viewportWidth(800),
    viewportHeight(600),
    renderWidth(1200),
    renderHeight(900),
    renderScaleMode(1),
    fxaaEnabled(false),
    lightDirection(0.54f, -0.50f, -0.68f),
    lightColor(1.0f, 1.0f, 1.0f),
    ambientColor(0.03f, 0.03f, 0.035f) {
  lightViewProjection.SetToIdentity();
}

Renderer::~Renderer() {
  destroy();
}

bool Renderer::initialize(int width, int height) {
  viewportWidth = width;
  viewportHeight = height;
  updateRenderSize();
  setupGLState();
  if (!loadShaders()) {
    return false;
  }
  if (!createSceneTargets(renderWidth, renderHeight)) {
    std::fprintf(stderr, "Renderer: failed to create scene targets\n");
    return false;
  }
  if (!createShadowMap()) {
    std::fprintf(stderr, "Renderer: failed to create shadow map\n");
    return false;
  }
  ::glGenVertexArrays(1, &fullscreenVAO);
  ::glViewport(0, 0, width, height);
  std::fprintf(stderr, "Renderer: supersampling %.1fx (%dx%d -> %dx%d), FXAA off\n",
          currentRenderScale(), viewportWidth, viewportHeight, renderWidth, renderHeight);
  return true;
}

void Renderer::resize(int width, int height) {
  if (width <= 0 || height <= 0) return;
  viewportWidth = width;
  viewportHeight = height;
  updateRenderSize();
  createSceneTargets(renderWidth, renderHeight);
  ::glViewport(0, 0, width, height);
}

float Renderer::currentRenderScale() const {
  if (renderScaleMode == 0) return 1.0f;
  if (renderScaleMode == 2) return 2.0f;
  return 1.5f;
}

void Renderer::updateRenderSize() {
  float scale = currentRenderScale();
  renderWidth = (int)((float)viewportWidth * scale + 0.5f);
  renderHeight = (int)((float)viewportHeight * scale + 0.5f);
}

float Renderer::cycleSupersampling() {
  renderScaleMode = (renderScaleMode + 1) % 3;
  updateRenderSize();
  if (!createSceneTargets(renderWidth, renderHeight)) {
    std::fprintf(stderr, "Renderer: failed to resize supersampling targets\n");
  }
  ::glViewport(0, 0, viewportWidth, viewportHeight);
  float scale = currentRenderScale();
  std::fprintf(stderr, "Supersampling: %.1fx (%dx%d -> %dx%d)\n",
          scale, viewportWidth, viewportHeight, renderWidth, renderHeight);
  return scale;
}

bool Renderer::toggleFXAA() {
  fxaaEnabled = !fxaaEnabled;
  return fxaaEnabled;
}

void Renderer::setupGLState() {
  ::glEnable(GL_DEPTH_TEST);
  ::glEnable(GL_CULL_FACE);
  ::glCullFace(GL_BACK);
  ::glFrontFace(GL_CCW);
}

bool Renderer::loadShaders() {
  if (!pbrShader.loadFromFiles("Shader/pbr.vert", "Shader/pbr.frag")) {
    std::fprintf(stderr, "Renderer: failed to load PBR shader\n");
    return false;
  }
  if (!skyboxShader.loadFromFiles("Shader/skybox.vert", "Shader/skybox.frag")) {
    std::fprintf(stderr, "Renderer: failed to load skybox shader\n");
    return false;
  }
  if (!tonemapShader.loadFromFiles("Shader/tonemap.vert", "Shader/tonemap.frag")) {
    std::fprintf(stderr, "Renderer: failed to load tonemap shader\n");
    return false;
  }
  if (!shadowShader.loadFromFiles("Shader/shadow_depth.vert", "Shader/shadow_depth.frag")) {
    std::fprintf(stderr, "Renderer: failed to load shadow depth shader\n");
    return false;
  }
  if (!ibl.initialize()) {
    std::fprintf(stderr, "Renderer: failed to initialize IBL\n");
    return false;
  }
  return true;
}

void Renderer::destroySceneTargets() {
  if (msaaFBO) { ::glDeleteFramebuffers(1, &msaaFBO); msaaFBO = 0; }
  if (msaaColorRbo) { ::glDeleteRenderbuffers(1, &msaaColorRbo); msaaColorRbo = 0; }
  if (msaaDepthRbo) { ::glDeleteRenderbuffers(1, &msaaDepthRbo); msaaDepthRbo = 0; }
  if (resolveFBO) { ::glDeleteFramebuffers(1, &resolveFBO); resolveFBO = 0; }
  if (sceneColorTex) { ::glDeleteTextures(1, &sceneColorTex); sceneColorTex = 0; }
  if (sceneSampleTex) { ::glDeleteTextures(1, &sceneSampleTex); sceneSampleTex = 0; }
  if (sceneDepthRbo) { ::glDeleteRenderbuffers(1, &sceneDepthRbo); sceneDepthRbo = 0; }
  msaaSamples = 0;
}

bool Renderer::createSceneTargets(int width, int height) {
  destroySceneTargets();
  if (width <= 0 || height <= 0) return false;

  ::glGenTextures(1, &sceneColorTex);
  ::glBindTexture(GL_TEXTURE_2D, sceneColorTex);
  ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  ::glGenTextures(1, &sceneSampleTex);
  ::glBindTexture(GL_TEXTURE_2D, sceneSampleTex);
  ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  ::glGenFramebuffers(1, &resolveFBO);
  ::glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
  ::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTex, 0);
  if (::glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::fprintf(stderr, "Renderer: resolve framebuffer incomplete\n");
    ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
    destroySceneTargets();
    return false;
  }

  GLint maxSamples = 0;
  ::glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
  int candidates[3] = {8, 4, 2};
  msaaSamples = 0;
  for (int i = 0; i < 3; i++) {
    if (candidates[i] > TARGET_MSAA_SAMPLES) continue;
    if (candidates[i] > maxSamples) continue;

    int trySamples = candidates[i];
    ::glGenRenderbuffers(1, &msaaColorRbo);
    ::glBindRenderbuffer(GL_RENDERBUFFER, msaaColorRbo);
    ::glRenderbufferStorageMultisample(GL_RENDERBUFFER, trySamples, GL_RGBA16F, width, height);

    ::glGenRenderbuffers(1, &msaaDepthRbo);
    ::glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthRbo);
    ::glRenderbufferStorageMultisample(GL_RENDERBUFFER, trySamples, GL_DEPTH_COMPONENT24, width, height);

    ::glGenFramebuffers(1, &msaaFBO);
    ::glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    ::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorRbo);
    ::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msaaDepthRbo);

    GLenum status = ::glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE) {
      msaaSamples = trySamples;
      static int loggedMsaa = -1;
      if (loggedMsaa != msaaSamples) {
        std::fprintf(stderr, "Renderer: MSAA %dx enabled\n", msaaSamples);
        loggedMsaa = msaaSamples;
      }
      break;
    }

    std::fprintf(stderr,
            "Renderer: MSAA %dx incomplete (0x%X), trying lower samples\n",
            trySamples, (unsigned int)status);
    if (msaaFBO) { ::glDeleteFramebuffers(1, &msaaFBO); msaaFBO = 0; }
    if (msaaColorRbo) { ::glDeleteRenderbuffers(1, &msaaColorRbo); msaaColorRbo = 0; }
    if (msaaDepthRbo) { ::glDeleteRenderbuffers(1, &msaaDepthRbo); msaaDepthRbo = 0; }
  }

  if (msaaSamples <= 0) {
    ::glGenRenderbuffers(1, &sceneDepthRbo);
    ::glBindRenderbuffer(GL_RENDERBUFFER, sceneDepthRbo);
    ::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

    ::glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    ::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneDepthRbo);
    GLenum status = ::glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      std::fprintf(stderr, "Renderer: single-sample framebuffer incomplete (0x%X)\n", (unsigned int)status);
      ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
      destroySceneTargets();
      return false;
    }
    std::fprintf(stderr, "Renderer: MSAA disabled (single-sample path)\n");
  }

  ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
  ::glBindTexture(GL_TEXTURE_2D, 0);
  ::glBindRenderbuffer(GL_RENDERBUFFER, 0);
  return true;
}

void Renderer::destroyShadowMap() {
  if (shadowFBO) { ::glDeleteFramebuffers(1, &shadowFBO); shadowFBO = 0; }
  if (shadowDepthTex) { ::glDeleteTextures(1, &shadowDepthTex); shadowDepthTex = 0; }
  if (shadowDepthRbo) { ::glDeleteRenderbuffers(1, &shadowDepthRbo); shadowDepthRbo = 0; }
  shadowsEnabled = false;
}

bool Renderer::createShadowMap() {
  destroyShadowMap();
  if (shadowMapSize <= 0) shadowMapSize = DEFAULT_SHADOW_MAP_SIZE;

  ::glGenTextures(1, &shadowDepthTex);
  ::glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
  ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
               shadowMapSize, shadowMapSize, 0,
               GL_RGBA, GL_FLOAT, nullptr);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  ::glBindTexture(GL_TEXTURE_2D, 0);

  ::glGenRenderbuffers(1, &shadowDepthRbo);
  ::glBindRenderbuffer(GL_RENDERBUFFER, shadowDepthRbo);
  ::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                        shadowMapSize, shadowMapSize);
  ::glBindRenderbuffer(GL_RENDERBUFFER, 0);

  ::glGenFramebuffers(1, &shadowFBO);
  ::glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
  ::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, shadowDepthTex, 0);
  ::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, shadowDepthRbo);

  GLenum status = ::glCheckFramebufferStatus(GL_FRAMEBUFFER);
  ::glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::fprintf(stderr,
            "Renderer: shadow framebuffer incomplete (0x%X), shadows disabled\n",
            (unsigned int)status);
    destroyShadowMap();
    return true;
  }

  shadowsEnabled = true;
  std::fprintf(stderr, "Renderer: shadow map enabled %dx%d\n", shadowMapSize, shadowMapSize);
  return true;
}

void Renderer::computeLightMatrix(Scene &scene) {
  Vec3f bmin, bmax;
  scene.getBounds(bmin, bmax);

  Vec3f center = (bmin + bmax) * 0.5f;
  Vec3f extent = bmax - bmin;
  float radius = 0.5f * extent.x();
  if (0.5f * extent.y() > radius) radius = 0.5f * extent.y();
  if (0.5f * extent.z() > radius) radius = 0.5f * extent.z();
  if (radius < 0.01f) radius = 1.0f;
  radius *= 1.15f;

  lightDirection.Normalize();
  Vec3f toLight = lightDirection * -1.0f;
  Vec3f lightPos = center + toLight * (radius * 2.0f);

  Vec3f front = center - lightPos;
  front.Normalize();
  Vec3f worldUp(0.0f, 1.0f, 0.0f);
  if (std::fabs(front.Dot3(worldUp)) > 0.95f) {
    worldUp = Vec3f(0.0f, 0.0f, 1.0f);
  }

  Vec3f right, up;
  Vec3f::Cross3(right, front, worldUp);
  right.Normalize();
  Vec3f::Cross3(up, right, front);
  up.Normalize();

  Matrix lightView;
  lightView.SetToIdentity();
  lightView.Set(0, 0, right.x());
  lightView.Set(1, 0, right.y());
  lightView.Set(2, 0, right.z());
  lightView.Set(0, 1, up.x());
  lightView.Set(1, 1, up.y());
  lightView.Set(2, 1, up.z());
  lightView.Set(0, 2, -front.x());
  lightView.Set(1, 2, -front.y());
  lightView.Set(2, 2, -front.z());
  lightView.Set(3, 0, -right.Dot3(lightPos));
  lightView.Set(3, 1, -up.Dot3(lightPos));
  lightView.Set(3, 2, front.Dot3(lightPos));

  float nearZ = 0.1f;
  float farZ = radius * 4.0f;
  if (farZ < nearZ + 1.0f) farZ = nearZ + 1.0f;

  Matrix lightProj;
  lightProj.Clear();
  lightProj.Set(0, 0, 1.0f / radius);
  lightProj.Set(1, 1, 1.0f / radius);
  lightProj.Set(2, 2, -2.0f / (farZ - nearZ));
  lightProj.Set(3, 2, -(farZ + nearZ) / (farZ - nearZ));
  lightProj.Set(3, 3, 1.0f);

  lightViewProjection = lightProj * lightView;
}

void Renderer::renderShadowMap(std::vector<Mesh *> const& opaque) {
  if (!shadowsEnabled || !shadowFBO) return;

  ::glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
  ::glViewport(0, 0, shadowMapSize, shadowMapSize);
  ::glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ::glEnable(GL_DEPTH_TEST);
  ::glDepthMask(GL_TRUE);
  ::glDisable(GL_BLEND);
  ::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  ::glEnable(GL_POLYGON_OFFSET_FILL);
  ::glPolygonOffset(1.1f, 4.0f);
  ::glEnable(GL_CULL_FACE);
  ::glCullFace(GL_FRONT);
  ::glFrontFace(GL_CCW);

  shadowShader.use();
  float *lvpPtr = lightViewProjection.glGet();
  shadowShader.setMat4("uLightViewProjection", lvpPtr);
  delete [] lvpPtr;

  for (size_t i = 0; i < opaque.size(); i++) {
    Mesh *mesh = opaque[i];
    if (!mesh) continue;

    float *modelPtr = mesh->getModelMatrix().glGet();
    shadowShader.setMat4("uModel", modelPtr);
    delete [] modelPtr;

    if (modelDeterminant3x3(mesh->getModelMatrix()) < 0.0f) {
      ::glFrontFace(GL_CW);
    } else {
      ::glFrontFace(GL_CCW);
    }
    mesh->draw();
  }

  ::glCullFace(GL_BACK);
  ::glFrontFace(GL_CCW);
  ::glDisable(GL_POLYGON_OFFSET_FILL);
  ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::setLightDirection(Vec3f const&dir) {
  lightDirection = dir;
  lightDirection.Normalize();
}

void Renderer::setLightColor(Vec3f const&color) {
  lightColor = color;
}

void Renderer::setAmbientColor(Vec3f const&color) {
  ambientColor = color;
}

bool Renderer::isTransparentMesh(Mesh const*mesh) {
  if (!mesh) return false;
  PBRMaterial const*mat = mesh->getMaterial();
  if (!mat) return false;
  if (mat->alphaMode == ALPHA_BLEND) return true;
  if (mat->hasTransmission && mat->transmissionFactor > 0.001f) return true;
  return false;
}

float Renderer::meshSortKey(Mesh const*mesh, Vec3f const&camPos) {
  Vec3f const&c = mesh->getWorldCenter();
  float dx = c.x() - camPos.x();
  float dy = c.y() - camPos.y();
  float dz = c.z() - camPos.z();
  return dx * dx + dy * dy + dz * dz;
}

void Renderer::drawSingleMesh(Mesh *mesh, bool transparentPass) {
  if (!mesh) return;

  PBRMaterial *mat = mesh->getMaterial();
  float *modelPtr = mesh->getModelMatrix().glGet();
  pbrShader.setMat4("uModel", modelPtr);

  Matrix normalMat = mesh->getModelMatrix();
  normalMat.Inverse();
  normalMat.Transpose();
  float nm[9] = {
    normalMat.Get(0, 0), normalMat.Get(0, 1), normalMat.Get(0, 2),
    normalMat.Get(1, 0), normalMat.Get(1, 1), normalMat.Get(1, 2),
    normalMat.Get(2, 0), normalMat.Get(2, 1), normalMat.Get(2, 2)
  };
  ::glUniformMatrix3fv(::glGetUniformLocation(pbrShader.programId(), "uNormalMatrix"),
                     1, GL_FALSE, nm);
  delete [] modelPtr;

  if (mat) {
    if (mat->doubleSided || transparentPass) {
      ::glDisable(GL_CULL_FACE);
    } else {
      ::glEnable(GL_CULL_FACE);
      if (modelDeterminant3x3(mesh->getModelMatrix()) < 0.0f) {
        ::glFrontFace(GL_CW);
      } else {
        ::glFrontFace(GL_CCW);
      }
    }

    if (transparentPass) {
      ::glEnable(GL_BLEND);
      ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      ::glDepthMask(GL_FALSE);
    } else {
      ::glDisable(GL_BLEND);
      ::glDepthMask(GL_TRUE);
    }

    mat->bindTextures(pbrShader);
    mat->setUniforms(pbrShader);
  } else if (transparentPass) {
    ::glEnable(GL_BLEND);
    ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ::glDepthMask(GL_FALSE);
  } else {
    ::glDisable(GL_BLEND);
    ::glDepthMask(GL_TRUE);
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
  lightDirection.Normalize();

  pbrShader.setVec3("uCameraPos", camPos.x(), camPos.y(), camPos.z());
  pbrShader.setVec3("uLightDir", lightDirection.x(), lightDirection.y(), lightDirection.z());
  pbrShader.setVec3("uLightColor", lightColor.x(), lightColor.y(), lightColor.z());
  pbrShader.setVec3("uAmbientColor", ambientColor.x(), ambientColor.y(), ambientColor.z());
  pbrShader.setFloat("uDirectLightScale", 2.45f);
  pbrShader.setFloat("uDiffuseEnvScale", 0.10f);
  pbrShader.setFloat("uSpecularEnvScale", 1.42f);
  pbrShader.setFloat("uHemiFillScale", 0.006f);
  pbrShader.setVec2("uFramebufferSize", (float)renderWidth, (float)renderHeight);

  pbrShader.setBool("uUseShadows", shadowsEnabled);
  pbrShader.setFloat("uShadowBias", 0.0025f);
  if (shadowsEnabled && shadowDepthTex) {
    float *lvpPtr = lightViewProjection.glGet();
    pbrShader.setMat4("uLightViewProjection", lvpPtr);
    delete [] lvpPtr;
    ::glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_UNIT);
    ::glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
    pbrShader.setInt("uShadowMap", SHADOW_MAP_UNIT);
  }

  bool useStrips = scene.hasLightStrips();
  pbrShader.setBool("uUseLightStrips", useStrips);
  pbrShader.setInt("uLightStripCount", useStrips ? LIGHT_STRIP_COUNT : 0);
  if (useStrips) {
    const LightStrip *strips = scene.getLightStrips();
    for (int i = 0; i < LIGHT_STRIP_COUNT; i++) {
      char name[64];
      sprintf(name, "uStripCenter[%d]", i);
      pbrShader.setVec3(name, strips[i].center.x(), strips[i].center.y(), strips[i].center.z());
      sprintf(name, "uStripHalfRight[%d]", i);
      pbrShader.setVec3(name, strips[i].halfRight.x(), strips[i].halfRight.y(), strips[i].halfRight.z());
      sprintf(name, "uStripHalfUp[%d]", i);
      pbrShader.setVec3(name, strips[i].halfUp.x(), strips[i].halfUp.y(), strips[i].halfUp.z());
      sprintf(name, "uStripNormal[%d]", i);
      pbrShader.setVec3(name, strips[i].normal.x(), strips[i].normal.y(), strips[i].normal.z());
      sprintf(name, "uStripColor[%d]", i);
      pbrShader.setVec3(name, strips[i].color.x(), strips[i].color.y(), strips[i].color.z());
    }
  }

  ibl.bindForPBR(pbrShader);
}

void Renderer::drawMeshes(Scene& scene, bool transparentPassOnly,
                          std::vector<Mesh*> const* opaque,
                          std::vector<Mesh*> const* transparent) {
  bindCommonPBRUniforms(scene);

  if (!transparentPassOnly && opaque) {
    pbrShader.setBool("uHasSceneColor", false);
    for (size_t i = 0; i < opaque->size(); i++) {
      drawSingleMesh((*opaque)[i], false);
    }
  }

  if (transparentPassOnly && transparent) {
    ::glActiveTexture(GL_TEXTURE0 + SCENE_SAMPLE_UNIT);
    ::glBindTexture(GL_TEXTURE_2D, sceneSampleTex);
    pbrShader.setInt("uSceneColorMap", SCENE_SAMPLE_UNIT);
    pbrShader.setBool("uHasSceneColor", true);

    for (size_t i = 0; i < transparent->size(); i++) {
      drawSingleMesh((*transparent)[i], true);
    }
  }

  ::glEnable(GL_CULL_FACE);
  ::glFrontFace(GL_CCW);
  ::glDisable(GL_BLEND);
  ::glDepthMask(GL_TRUE);
}

void Renderer::resolveMsaaToSceneColor() {
  if (msaaSamples <= 0 || !msaaFBO) return;

  ::glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
  ::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
  ::glBlitFramebuffer(0, 0, renderWidth, renderHeight,
                    0, 0, renderWidth, renderHeight,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);
  ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::captureSceneColorSample() {
  ::glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
  ::glBindTexture(GL_TEXTURE_2D, sceneSampleTex);
  ::glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, renderWidth, renderHeight);
  ::glGenerateMipmap(GL_TEXTURE_2D);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

void Renderer::blitTonemapToScreen() {
  ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
  ::glViewport(0, 0, viewportWidth, viewportHeight);
  ::glDisable(GL_DEPTH_TEST);
  ::glDisable(GL_BLEND);
  ::glDepthMask(GL_TRUE);

  tonemapShader.use();
  ::glActiveTexture(GL_TEXTURE0);
  ::glBindTexture(GL_TEXTURE_2D, sceneColorTex);
  tonemapShader.setInt("uHdrColor", 0);
  tonemapShader.setVec2("uFramebufferSize", (float)renderWidth, (float)renderHeight);
  tonemapShader.setBool("uEnableFXAA", fxaaEnabled);

  ::glBindVertexArray(fullscreenVAO);
  ::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  ::glBindVertexArray(0);

  ::glEnable(GL_DEPTH_TEST);
}

void Renderer::render(Scene &scene) {
  OrbitCamera &cam = scene.getCamera();
  cam.setAspect((float)viewportWidth / (float)viewportHeight);
  cam.updateMatrices();

  std::vector<Mesh*> const& meshes = scene.getMeshes();
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
    [&camPos](Mesh const*a, Mesh const*b) {
      return meshSortKey(a, camPos) > meshSortKey(b, camPos);
    });

  if (shadowsEnabled && !opaque.empty()) {
    computeLightMatrix(scene);
    renderShadowMap(opaque);
  }

  unsigned int drawFBO = (msaaSamples > 0 && msaaFBO) ? msaaFBO : resolveFBO;
  ::glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
  ::glViewport(0, 0, renderWidth, renderHeight);

  Vec3f bg = scene.getBackgroundColor();
  ::glClearColor(bg.x(), bg.y(), bg.z(), 1.0f);
  ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float *viewPtr = cam.getViewMatrix().glGet();
  float *projPtr = cam.getProjectionMatrix().glGet();
  skyboxShader.use();
  skyboxShader.setBool("uOutputLinear", true);
  ibl.renderSkybox(skyboxShader, viewPtr, projPtr);
  delete [] viewPtr;
  delete [] projPtr;

  drawMeshes(scene, false, &opaque, nullptr);

  if (!transparent.empty()) {
    resolveMsaaToSceneColor();
    captureSceneColorSample();
    ::glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
    drawMeshes(scene, true, nullptr, &transparent);
  }

  resolveMsaaToSceneColor();
  if (msaaSamples <= 0) {
  }

  blitTonemapToScreen();
}

void Renderer::destroy() {
  if (fullscreenVAO) {
    ::glDeleteVertexArrays(1, &fullscreenVAO);
    fullscreenVAO = 0;
  }
  destroyShadowMap();
  destroySceneTargets();
  ibl.destroy();
  shadowShader.destroy();
  tonemapShader.destroy();
  pbrShader.destroy();
  skyboxShader.destroy();
}
