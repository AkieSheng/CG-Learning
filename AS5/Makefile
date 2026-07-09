# Makefile for 6.837 Assignment 5 (Voxel Rendering / Uniform Grid)
# g++ on Windows MinGW / Linux

CORE_DIR = Core
SPATIAL_DIR = Spatial
CAMERA_DIR = Camera
OBJECTS_DIR = Objects
IMAGE_DIR = ImageLibrary
LA_DIR = LinearAlgebraLibrary
PARSER_DIR = Parser
GL_DIR = GL
INPUT_DIR = InputFiles
MESH_DIR = Meshes
OUTPUT_DIR = OutputFiles
EXE = raytracer
RT = $(EXE) -size 200 200
MRT = cd $(MESH_DIR) && ..\\$(EXE) -size 200 200

SRCS = main.C \
       $(CORE_DIR)/object3d.C \
       $(CAMERA_DIR)/camera.C \
       $(OBJECTS_DIR)/group.C \
       $(OBJECTS_DIR)/sphere.C \
       $(OBJECTS_DIR)/plane.C \
       $(OBJECTS_DIR)/triangle.C \
       $(OBJECTS_DIR)/transform.C \
       $(CORE_DIR)/material.C \
       $(CORE_DIR)/light.C \
       $(CORE_DIR)/rayTracer.C \
       $(CORE_DIR)/rayTree.C \
       $(SPATIAL_DIR)/boundingbox.C \
       $(SPATIAL_DIR)/grid.C \
       $(SPATIAL_DIR)/marchinginfo.C \
       $(IMAGE_DIR)/image.C \
       $(LA_DIR)/matrix.C \
       $(PARSER_DIR)/scene_parser.C \
       $(GL_DIR)/glCanvas.C

LIBS = -lm -lopengl32 -lglu32 -lfreeglut
CC = g++
CFLAGS = -I. -I$(CORE_DIR) -I$(SPATIAL_DIR) -I$(CAMERA_DIR) -I$(OBJECTS_DIR) \
         -I$(IMAGE_DIR) -I$(LA_DIR) -I$(PARSER_DIR) -I$(GL_DIR)
OBJS = $(SRCS:.C=.o)

.PHONY: all depend clean render test

all: depend $(EXE)

depend:
	$(CC) $(CFLAGS) -E -M $(SRCS) > Makefile.depend

clean:
	-del /Q *.o $(EXE) Makefile.depend 2>nul
	-del /Q $(CORE_DIR)\*.o $(SPATIAL_DIR)\*.o $(CAMERA_DIR)\*.o $(OBJECTS_DIR)\*.o 2>nul
	-del /Q $(IMAGE_DIR)\*.o $(LA_DIR)\*.o $(PARSER_DIR)\*.o $(GL_DIR)\*.o 2>nul

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.C.o:
	$(CC) $(CFLAGS) -c -o $@ $<

# Assignment 5 Sample Results
render: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	@echo === scene5_01 - scene5_03 ===
	$(RT) -input $(INPUT_DIR)/scene5_01_sphere.txt -output $(OUTPUT_DIR)/output5_01a.tga -grid 1 1 1 -visualize_grid
	$(RT) -input $(INPUT_DIR)/scene5_01_sphere.txt -output $(OUTPUT_DIR)/output5_01b.tga -grid 5 5 5 -visualize_grid
	$(RT) -input $(INPUT_DIR)/scene5_01_sphere.txt -output $(OUTPUT_DIR)/output5_01c.tga -grid 15 15 15 -visualize_grid
	$(RT) -input $(INPUT_DIR)/scene5_02_spheres.txt -output $(OUTPUT_DIR)/output5_02a.tga -grid 15 15 15 -visualize_grid
	$(RT) -input $(INPUT_DIR)/scene5_02_spheres.txt -output $(OUTPUT_DIR)/output5_02b.tga -grid 15 15 3 -visualize_grid
	$(RT) -input $(INPUT_DIR)/scene5_03_offcenter_spheres.txt -output $(OUTPUT_DIR)/output5_03.tga -grid 20 20 20 -visualize_grid
	@echo === scene5_04 - scene5_09 ===
	$(RT) -input $(INPUT_DIR)/scene5_04_plane_test.txt -output $(OUTPUT_DIR)/output5_04.tga -grid 15 15 15 -visualize_grid
	$(RT) -input $(INPUT_DIR)/scene5_05_sphere_triangles.txt -output $(OUTPUT_DIR)/output5_05.tga -grid 20 20 10 -visualize_grid
	$(MRT) -input ..\\$(INPUT_DIR)/scene5_06_bunny_mesh_200.txt -output ..\\$(OUTPUT_DIR)/output5_06.tga -grid 10 10 7 -visualize_grid
	$(MRT) -input ..\\$(INPUT_DIR)/scene5_07_bunny_mesh_1k.txt -output ..\\$(OUTPUT_DIR)/output5_07.tga -grid 15 15 12 -visualize_grid
	$(MRT) -input ..\\$(INPUT_DIR)/scene5_08_bunny_mesh_5k.txt -output ..\\$(OUTPUT_DIR)/output5_08.tga -grid 20 20 15 -visualize_grid
	$(MRT) -input ..\\$(INPUT_DIR)/scene5_09_bunny_mesh_40k.txt -output ..\\$(OUTPUT_DIR)/output5_09.tga -grid 40 40 33 -visualize_grid
	@echo === scene5_10 - scene5_12 ===
	$(RT) -input $(INPUT_DIR)/scene5_10_scale_translate.txt -output $(OUTPUT_DIR)/output5_10.tga -grid 15 15 15 -visualize_grid
	$(RT) -input $(INPUT_DIR)/scene5_11_rotated_triangles.txt -output $(OUTPUT_DIR)/output5_11.tga -grid 15 15 9 -visualize_grid
	$(MRT) -input ..\\$(INPUT_DIR)/scene5_12_nested_transformations.txt -output ..\\$(OUTPUT_DIR)/output5_12.tga -grid 30 30 30 -visualize_grid
	@echo === All Sample Results rendered to $(OUTPUT_DIR)/ ===

test: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	$(RT) -input $(INPUT_DIR)/scene5_01_sphere.txt -output $(OUTPUT_DIR)/output5_01a.tga -grid 1 1 1 -visualize_grid

-include Makefile.depend
