# Simple Makefile for 6.837 raytracing assignment
# for g++ on linux / mingw platforms
# ===============================================================

CORE_DIR = Core
CAMERA_DIR = Camera
OBJECTS_DIR = Objects
IMAGE_DIR = ImageLibrary
LA_DIR = LinearAlgebraLibrary
PARSER_DIR = Parser
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
       $(IMAGE_DIR)/image.C \
       $(LA_DIR)/matrix.C \
       $(PARSER_DIR)/scene_parser.C

# ===============================================================

LIBS = -lm
CC = g++
CFLAGS = -I. -I$(CORE_DIR) -I$(CAMERA_DIR) -I$(OBJECTS_DIR) -I$(IMAGE_DIR) -I$(LA_DIR) -I$(PARSER_DIR)
EXE = raytracer
OBJS = $(SRCS:.C=.o)

# ===============================================================
# targets

.PHONY: all depend clean render

all: depend $(EXE)

depend:
	$(CC) $(CFLAGS) -E -M $(SRCS) > Makefile.depend

clean:
	rm -f *~ *bak *.o $(EXE) Makefile.depend
	rm -f $(CORE_DIR)/*.o $(CAMERA_DIR)/*.o $(OBJECTS_DIR)/*.o
	rm -f $(IMAGE_DIR)/*.o $(LA_DIR)/*.o $(PARSER_DIR)/*.o

render: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	$(EXE) -input $(INPUT_DIR)/scene2_01_diffuse.txt -size 200 200 -output $(OUTPUT_DIR)/output2_01.tga
	$(EXE) -input $(INPUT_DIR)/scene2_02_ambient.txt -size 200 200 -output $(OUTPUT_DIR)/output2_02.tga
	$(EXE) -input $(INPUT_DIR)/scene2_03_colored_lights.txt -size 200 200 -output $(OUTPUT_DIR)/output2_03.tga -normals $(OUTPUT_DIR)/normals2_03.tga
	$(EXE) -input $(INPUT_DIR)/scene2_04_perspective.txt -size 200 200 -output $(OUTPUT_DIR)/output2_04.tga -normals $(OUTPUT_DIR)/normals2_04.tga
	$(EXE) -input $(INPUT_DIR)/scene2_05_inside_sphere.txt -size 200 200 -output $(OUTPUT_DIR)/output2_05.tga -depth 9 11 $(OUTPUT_DIR)/depth2_05.tga -normals $(OUTPUT_DIR)/normals2_05.tga -shade_back
	$(EXE) -input $(INPUT_DIR)/scene2_05_inside_sphere.txt -size 200 200 -output $(OUTPUT_DIR)/output2_05_no_back.tga
	$(EXE) -input $(INPUT_DIR)/scene2_06_plane.txt -size 200 200 -output $(OUTPUT_DIR)/output2_06.tga -depth 8 20 $(OUTPUT_DIR)/depth2_06.tga -normals $(OUTPUT_DIR)/normals2_06.tga
	$(EXE) -input $(INPUT_DIR)/scene2_07_sphere_triangles.txt -size 200 200 -output $(OUTPUT_DIR)/output2_07.tga -depth 9 11 $(OUTPUT_DIR)/depth2_07.tga -normals $(OUTPUT_DIR)/normals2_07.tga -shade_back
	$(EXE) -input $(INPUT_DIR)/scene2_07_sphere_triangles.txt -size 200 200 -output $(OUTPUT_DIR)/output2_07_no_back.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene2_08_cube.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output2_08.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene2_09_bunny_200.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output2_09.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene2_10_bunny_1k.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output2_10.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene2_11_squashed_sphere.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output2_11.tga -normals ..\$(OUTPUT_DIR)/normals2_11.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene2_12_rotated_sphere.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output2_12.tga -normals ..\$(OUTPUT_DIR)/normals2_12.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene2_13_rotated_squashed_sphere.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output2_13.tga -normals ..\$(OUTPUT_DIR)/normals2_13.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene2_14_axes_cube.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output2_14.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene2_15_crazy_transforms.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output2_15.tga
	cd $(MESH_DIR) && ..\$(EXE) -input ..\$(INPUT_DIR)/scene2_16_t_scale.txt -size 200 200 -output ..\$(OUTPUT_DIR)/output2_16.tga -depth 2 7 ..\$(OUTPUT_DIR)/depth2_16.tga

# ===============================================================
# compilation rules

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.C.o:
	$(CC) $(CFLAGS) -c -o $@ $<

# ===============================================================

-include Makefile.depend
