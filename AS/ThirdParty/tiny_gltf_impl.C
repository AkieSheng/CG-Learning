// tinygltf 编译单元：禁用内置图像 IO，由 AS/Texture 负责加载
#include <string>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE

namespace tinygltf {
class Image;
struct FsCallbacks;
struct URICallbacks;

bool LoadImageData(Image *image, const int image_idx, std::string *err,
                   std::string *warn, int req_width, int req_height,
                   const unsigned char *bytes, int size, void *);

bool WriteImageData(const std::string *basepath, const std::string *filename,
                    const Image *image, bool embedImages,
                    const FsCallbacks *fs_cb, const URICallbacks *uri_cb,
                    std::string *out_uri, void *);
}

namespace tinygltf {

bool LoadImageData(Image *image, const int image_idx, std::string *err,
                   std::string *warn, int req_width, int req_height,
                   const unsigned char *bytes, int size, void *) {
  (void)image;
  (void)image_idx;
  (void)err;
  (void)warn;
  (void)req_width;
  (void)req_height;
  (void)bytes;
  (void)size;
  return true;
}

bool WriteImageData(const std::string *basepath, const std::string *filename,
                    const Image *image, bool embedImages,
                    const FsCallbacks *fs_cb, const URICallbacks *uri_cb,
                    std::string *out_uri, void *) {
  (void)basepath;
  (void)filename;
  (void)image;
  (void)embedImages;
  (void)fs_cb;
  (void)uri_cb;
  (void)out_uri;
  return true;
}

}

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
