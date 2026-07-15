# Makefile for AS - OpenGL PBR glTF Viewer
# 依赖: g++, MinGW, freeglut, tinygltf (ThirdParty/tinygltf)

CORE_DIR     = Core
RENDERER_DIR = Renderer
SHADER_DIR   = Shader
GLTF_DIR     = GLTF
MESH_DIR     = Mesh
MATERIAL_DIR = Material
GL_DIR       = GL
LA_DIR       = LinearAlgebraLibrary
THIRD_DIR    = ThirdParty
TINYGLTF_DIR = $(THIRD_DIR)/tinygltf
MODELS_DIR   = Models
EXE          = pbr_viewer

SRCS = main.C \
       $(CORE_DIR)/application.C \
       $(CORE_DIR)/orbit_camera.C \
       $(CORE_DIR)/scene.C \
       $(RENDERER_DIR)/renderer.C \
       $(RENDERER_DIR)/ibl.C \
       $(SHADER_DIR)/shader_program.C \
       $(GLTF_DIR)/gltf_loader.C \
       $(MESH_DIR)/mesh.C \
       $(MATERIAL_DIR)/texture.C \
       $(MATERIAL_DIR)/pbr_material.C \
       $(LA_DIR)/matrix.C \
       $(THIRD_DIR)/gl_loader.C \
       $(THIRD_DIR)/tiny_gltf_impl.C

LIBS = -lm -lopengl32 -lglu32 -lfreeglut
CC   = g++
CFLAGS = -std=c++11 -I. \
         -I$(CORE_DIR) -I$(RENDERER_DIR) -I$(SHADER_DIR) \
         -I$(GLTF_DIR) -I$(MESH_DIR) -I$(MATERIAL_DIR) \
         -I$(GL_DIR) -I$(LA_DIR) -I$(THIRD_DIR) -I$(TINYGLTF_DIR)

OBJS = $(SRCS:.C=.o)

.PHONY: all depend clean run

all: depend $(EXE)

depend:
	$(CC) $(CFLAGS) -E -M $(SRCS) > Makefile.depend

clean:
	-del /Q *.o $(EXE) Makefile.depend 2>nul
	-del /Q $(CORE_DIR)\*.o $(RENDERER_DIR)\*.o $(SHADER_DIR)\*.o 2>nul
	-del /Q $(GLTF_DIR)\*.o $(MESH_DIR)\*.o $(MATERIAL_DIR)\*.o 2>nul
	-del /Q $(LA_DIR)\*.o $(THIRD_DIR)\*.o 2>nul

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.C.o:
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(EXE)
	$(EXE) -model $(MODELS_DIR)/macbook_air_notebook_pbr/scene.gltf

-include Makefile.depend
