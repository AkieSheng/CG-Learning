#include "gl_loader.h"
#include <stdio.h>

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = NULL;
PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORM1FPROC glUniform1f = NULL;
PFNGLUNIFORM2FPROC glUniform2f = NULL;
PFNGLUNIFORM3FPROC glUniform3f = NULL;
PFNGLUNIFORM4FPROC glUniform4f = NULL;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = NULL;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = NULL;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = NULL;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = NULL;

#ifdef _WIN32
static void *getProc(const char *name) {
  void *p = (void *)wglGetProcAddress(name);
  if (p == NULL || p == (void *)0x1 || p == (void *)0x2 || p == (void *)0x3) {
    HMODULE mod = GetModuleHandleA("opengl32.dll");
    p = (void *)GetProcAddress(mod, name);
  }
  return p;
}
#else
#include <GL/glx.h>
static void *getProc(const char *name) {
  return (void *)glXGetProcAddress((const GLubyte *)name);
}
#endif

#define LOAD_FN(var, type, name) \
  var = (type)getProc(name); \
  if (!var) { fprintf(stderr, "Failed to load %s\n", name); return false; }

bool loadOpenGLFunctions() {
  LOAD_FN(glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC, "glGenVertexArrays");
  LOAD_FN(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC, "glBindVertexArray");
  LOAD_FN(glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC, "glDeleteVertexArrays");
  LOAD_FN(glGenBuffers, PFNGLGENBUFFERSPROC, "glGenBuffers");
  LOAD_FN(glBindBuffer, PFNGLBINDBUFFERPROC, "glBindBuffer");
  LOAD_FN(glBufferData, PFNGLBUFFERDATAPROC, "glBufferData");
  LOAD_FN(glDeleteBuffers, PFNGLDELETEBUFFERSPROC, "glDeleteBuffers");
  LOAD_FN(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC, "glEnableVertexAttribArray");
  LOAD_FN(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC, "glVertexAttribPointer");
  LOAD_FN(glActiveTexture, PFNGLACTIVETEXTUREPROC, "glActiveTexture");
  LOAD_FN(glGenerateMipmap, PFNGLGENERATEMIPMAPPROC, "glGenerateMipmap");
  LOAD_FN(glCreateShader, PFNGLCREATESHADERPROC, "glCreateShader");
  LOAD_FN(glShaderSource, PFNGLSHADERSOURCEPROC, "glShaderSource");
  LOAD_FN(glCompileShader, PFNGLCOMPILESHADERPROC, "glCompileShader");
  LOAD_FN(glGetShaderiv, PFNGLGETSHADERIVPROC, "glGetShaderiv");
  LOAD_FN(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC, "glGetShaderInfoLog");
  LOAD_FN(glDeleteShader, PFNGLDELETESHADERPROC, "glDeleteShader");
  LOAD_FN(glCreateProgram, PFNGLCREATEPROGRAMPROC, "glCreateProgram");
  LOAD_FN(glAttachShader, PFNGLATTACHSHADERPROC, "glAttachShader");
  LOAD_FN(glLinkProgram, PFNGLLINKPROGRAMPROC, "glLinkProgram");
  LOAD_FN(glGetProgramiv, PFNGLGETPROGRAMIVPROC, "glGetProgramiv");
  LOAD_FN(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC, "glGetProgramInfoLog");
  LOAD_FN(glDeleteProgram, PFNGLDELETEPROGRAMPROC, "glDeleteProgram");
  LOAD_FN(glUseProgram, PFNGLUSEPROGRAMPROC, "glUseProgram");
  LOAD_FN(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC, "glGetUniformLocation");
  LOAD_FN(glUniform1i, PFNGLUNIFORM1IPROC, "glUniform1i");
  LOAD_FN(glUniform1f, PFNGLUNIFORM1FPROC, "glUniform1f");
  LOAD_FN(glUniform2f, PFNGLUNIFORM2FPROC, "glUniform2f");
  LOAD_FN(glUniform3f, PFNGLUNIFORM3FPROC, "glUniform3f");
  LOAD_FN(glUniform4f, PFNGLUNIFORM4FPROC, "glUniform4f");
  LOAD_FN(glUniformMatrix3fv, PFNGLUNIFORMMATRIX3FVPROC, "glUniformMatrix3fv");
  LOAD_FN(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC, "glUniformMatrix4fv");
  LOAD_FN(glGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC, "glGenFramebuffers");
  LOAD_FN(glDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC, "glDeleteFramebuffers");
  LOAD_FN(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC, "glBindFramebuffer");
  LOAD_FN(glFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC, "glFramebufferTexture2D");
  LOAD_FN(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC, "glCheckFramebufferStatus");
  LOAD_FN(glGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC, "glGenRenderbuffers");
  LOAD_FN(glDeleteRenderbuffers, PFNGLDELETERENDERBUFFERSPROC, "glDeleteRenderbuffers");
  LOAD_FN(glBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC, "glBindRenderbuffer");
  LOAD_FN(glRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC, "glRenderbufferStorage");
  LOAD_FN(glFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC, "glFramebufferRenderbuffer");
  return true;
}

#undef LOAD_FN
