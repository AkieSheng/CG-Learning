# Makefile for 6.837 Assignment 3 (OpenGL + Phong Shading)
# g++ on Windows MinGW / Linux

CORE_DIR = Core
CAMERA_DIR = Camera
OBJECTS_DIR = Objects
IMAGE_DIR = ImageLibrary
LA_DIR = LinearAlgebraLibrary
PARSER_DIR = Parser
GL_DIR = GL
INPUT_DIR = InputFiles
MESH_DIR = TriangleMeshes
OUTPUT_DIR = OutputFiles

SRCS = main.C \
       $(CAMERA_DIR)/camera.C \
       $(OBJECTS_DIR)/group.C \
       $(OBJECTS_DIR)/sphere.C \
       $(OBJECTS_DIR)/plane.C \
       $(OBJECTS_DIR)/triangle.C \
       $(OBJECTS_DIR)/transform.C \
       $(CORE_DIR)/material.C \
       $(CORE_DIR)/light.C \
       $(IMAGE_DIR)/image.C \
       $(LA_DIR)/matrix.C \
       $(PARSER_DIR)/scene_parser.C \
       $(GL_DIR)/glCanvas.C

LIBS = -lm -lopengl32 -lglu32 -lfreeglut
CC = g++
CFLAGS = -I. -I$(CORE_DIR) -I$(CAMERA_DIR) -I$(OBJECTS_DIR) \
         -I$(IMAGE_DIR) -I$(LA_DIR) -I$(PARSER_DIR) -I$(GL_DIR)
EXE = raytracer
OBJS = $(SRCS:.C=.o)

.PHONY: all depend clean render render-variants test

all: depend $(EXE)

depend:
	$(CC) $(CFLAGS) -E -M $(SRCS) > Makefile.depend

clean:
	-del /Q *.o $(EXE) Makefile.depend 2>nul
	-del /Q $(CORE_DIR)\*.o $(CAMERA_DIR)\*.o $(OBJECTS_DIR)\*.o 2>nul
	-del /Q $(IMAGE_DIR)\*.o $(LA_DIR)\*.o $(PARSER_DIR)\*.o $(GL_DIR)\*.o 2>nul

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.C.o:
	$(CC) $(CFLAGS) -c -o $@ $<

render: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene3_01_cube_orthographic.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output3_01.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene3_02_cube_perspective.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output3_02.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene3_03_bunny_mesh_200.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output3_03.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene3_04_bunny_mesh_1k.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output3_04.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene3_05_axes_cube.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output3_05.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene3_06_crazy_transforms.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output3_06.tga
	$(EXE) -input $(INPUT_DIR)/scene3_07_plane.txt -size 200 200 -output $(OUTPUT_DIR)/output3_07.tga -tessellation 10 5
	$(EXE) -input $(INPUT_DIR)/scene3_08_sphere.txt -size 200 200 -output $(OUTPUT_DIR)/output3_08.tga -tessellation 10 5
	$(EXE) -input $(INPUT_DIR)/scene3_09_exponent_variations.txt -size 300 300 -output $(OUTPUT_DIR)/output3_09.tga -tessellation 100 50 -gouraud
	$(EXE) -input $(INPUT_DIR)/scene3_10_exponent_variations_back.txt -size 300 300 -output $(OUTPUT_DIR)/output3_10.tga -tessellation 100 50 -gouraud
	$(EXE) -input $(INPUT_DIR)/scene3_11_weird_lighting_diffuse.txt -size 200 200 -output $(OUTPUT_DIR)/output3_11.tga -tessellation 100 50 -gouraud
	$(EXE) -input $(INPUT_DIR)/scene3_12_weird_lighting_specular.txt -size 200 200 -output $(OUTPUT_DIR)/output3_12.tga -tessellation 100 50 -gouraud

render-variants: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	$(EXE) -input $(INPUT_DIR)/scene3_08_sphere.txt -size 200 200 -output $(OUTPUT_DIR)/output3_08_t10_p5.tga -tessellation 10 5
	$(EXE) -input $(INPUT_DIR)/scene3_08_sphere.txt -size 200 200 -output $(OUTPUT_DIR)/output3_08_t20_p10.tga -tessellation 20 10
	$(EXE) -input $(INPUT_DIR)/scene3_08_sphere.txt -size 200 200 -output $(OUTPUT_DIR)/output3_08_t10_p5_gouraud.tga -tessellation 10 5 -gouraud
	$(EXE) -input $(INPUT_DIR)/scene3_08_sphere.txt -size 200 200 -output $(OUTPUT_DIR)/output3_08_t20_p10_gouraud.tga -tessellation 20 10 -gouraud
	$(EXE) -input $(INPUT_DIR)/scene3_09_exponent_variations.txt -size 300 300 -output $(OUTPUT_DIR)/output3_09_specfix.tga -tessellation 100 50 -gouraud -specular_fix
	$(EXE) -input $(INPUT_DIR)/scene3_10_exponent_variations_back.txt -size 300 300 -output $(OUTPUT_DIR)/output3_10_specfix.tga -tessellation 100 50 -gouraud -specular_fix
	$(EXE) -input $(INPUT_DIR)/scene3_12_weird_lighting_specular.txt -size 200 200 -output $(OUTPUT_DIR)/output3_12_specfix.tga -tessellation 100 50 -gouraud -specular_fix

test: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene3_01_cube_orthographic.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output3_01.tga
	$(EXE) -input $(INPUT_DIR)/scene3_08_sphere.txt -size 200 200 -output $(OUTPUT_DIR)/output3_08.tga -tessellation 10 5

-include Makefile.depend
