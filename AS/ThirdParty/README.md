# ThirdParty 依赖

## tinygltf
- 来源: https://github.com/syoyo/tinygltf (MIT)
- 路径: `ThirdParty/tinygltf/`
- 用途: glTF 2.0 解析（`GltfLoader` 后端）
- 集成: `ThirdParty/tiny_gltf_impl.C` 中 `#define TINYGLTF_IMPLEMENTATION`
- 图像加载: 禁用 tinygltf 内置 stb_image，由 `Material/texture.C` 负责 OpenGL 纹理上传

## stb_image
- 来源: https://github.com/nothings/stb (public domain)
- 路径: `ThirdParty/stb_image.h`
- 用途: PNG/JPEG 纹理解码

## gl_loader
- 自研轻量 OpenGL 3.3 函数加载器（替代 GLEW）
- 路径: `ThirdParty/gl_loader.h` / `gl_loader.C`
