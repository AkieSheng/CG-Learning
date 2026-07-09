# Makefile for 6.837 Assignment 6 (Grid Acceleration & Solid Textures)
# g++ on Windows MinGW / Linux

CORE_DIR = Core
SPATIAL_DIR = Spatial
CAMERA_DIR = Camera
OBJECTS_DIR = Objects
IMAGE_DIR = ImageLibrary
LA_DIR = LinearAlgebraLibrary
PARSER_DIR = Parser
GL_DIR = GL
MATERIALS_DIR = Materials
PROCEDURAL_DIR = Procedural
STATS_DIR = Stats
INPUT_DIR = InputFiles
MESH_DIR = Meshes
OUTPUT_DIR = OutputFiles
EXE = raytracer
RT = $(EXE) -size 200 200
MRT = cd $(MESH_DIR) && ..\\$(EXE) -size 200 200
MRT300 = cd $(MESH_DIR) && ..\\$(EXE) -size 300 300
MRT400 = cd $(MESH_DIR) && ..\\$(EXE) -size 400 200

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
       $(GL_DIR)/glCanvas.C \
       $(MATERIALS_DIR)/checkerboard.C \
       $(MATERIALS_DIR)/noise.C \
       $(MATERIALS_DIR)/marble.C \
       $(MATERIALS_DIR)/wood.C \
       $(PROCEDURAL_DIR)/perlin_noise.C \
       $(STATS_DIR)/raytracing_stats.C

LIBS = -lm -lopengl32 -lglu32 -lfreeglut
CC = g++
CFLAGS = -I. -I$(CORE_DIR) -I$(SPATIAL_DIR) -I$(CAMERA_DIR) -I$(OBJECTS_DIR) \
         -I$(IMAGE_DIR) -I$(LA_DIR) -I$(PARSER_DIR) -I$(GL_DIR) \
         -I$(MATERIALS_DIR) -I$(PROCEDURAL_DIR) -I$(STATS_DIR)
OBJS = $(SRCS:.C=.o)

.PHONY: all depend clean render test stats

all: depend $(EXE)

depend:
	$(CC) $(CFLAGS) -E -M $(SRCS) > Makefile.depend

clean:
	-del /Q *.o $(EXE) Makefile.depend 2>nul
	-del /Q $(CORE_DIR)\*.o $(SPATIAL_DIR)\*.o $(CAMERA_DIR)\*.o $(OBJECTS_DIR)\*.o 2>nul
	-del /Q $(IMAGE_DIR)\*.o $(LA_DIR)\*.o $(PARSER_DIR)\*.o $(GL_DIR)\*.o 2>nul
	-del /Q $(MATERIALS_DIR)\*.o $(PROCEDURAL_DIR)\*.o $(STATS_DIR)\*.o 2>nul

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.C.o:
	$(CC) $(CFLAGS) -c -o $@ $<

# Assignment 6 — all Sample Results from the handout
render: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	@echo === scene6_01 ===
	$(RT) -input $(INPUT_DIR)/scene6_01_sphere.txt -output $(OUTPUT_DIR)/output6_01a.tga -stats
	$(RT) -input $(INPUT_DIR)/scene6_01_sphere.txt -output $(OUTPUT_DIR)/output6_01b.tga -grid 10 10 10 -stats
	$(RT) -input $(INPUT_DIR)/scene6_01_sphere.txt -output $(OUTPUT_DIR)/output6_01c.tga -grid 10 10 10 -visualize_grid
	@echo === scene6_02 ===
	$(RT) -input $(INPUT_DIR)/scene6_02_sphere_triangles.txt -output $(OUTPUT_DIR)/output6_02a.tga -stats
	$(RT) -input $(INPUT_DIR)/scene6_02_sphere_triangles.txt -output $(OUTPUT_DIR)/output6_02b.tga -grid 10 10 10 -stats
	$(RT) -input $(INPUT_DIR)/scene6_02_sphere_triangles.txt -output $(OUTPUT_DIR)/output6_02c.tga -stats -shadows
	$(RT) -input $(INPUT_DIR)/scene6_02_sphere_triangles.txt -output $(OUTPUT_DIR)/output6_02d.tga -grid 10 10 10 -stats -shadows
	$(RT) -input $(INPUT_DIR)/scene6_02_sphere_triangles.txt -output $(OUTPUT_DIR)/output6_02e.tga -grid 10 10 10 -visualize_grid
	@echo === scene6_03 ===
	$(RT) -input $(INPUT_DIR)/scene6_03_sphere_plane.txt -output $(OUTPUT_DIR)/output6_03a.tga -stats
	$(RT) -input $(INPUT_DIR)/scene6_03_sphere_plane.txt -output $(OUTPUT_DIR)/output6_03b.tga -grid 10 10 10 -stats
	$(RT) -input $(INPUT_DIR)/scene6_03_sphere_plane.txt -output $(OUTPUT_DIR)/output6_03c.tga -stats -shadows
	$(RT) -input $(INPUT_DIR)/scene6_03_sphere_plane.txt -output $(OUTPUT_DIR)/output6_03d.tga -grid 10 10 10 -stats -shadows
	$(RT) -input $(INPUT_DIR)/scene6_03_sphere_plane.txt -output $(OUTPUT_DIR)/output6_03e.tga -grid 10 10 10 -visualize_grid
	@echo === scene6_04 - scene6_07 (bunny meshes) ===
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_04_bunny_mesh_200.txt -output ..\\$(OUTPUT_DIR)/output6_04a.tga -stats
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_04_bunny_mesh_200.txt -output ..\\$(OUTPUT_DIR)/output6_04b.tga -grid 10 10 7 -stats
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_04_bunny_mesh_200.txt -output ..\\$(OUTPUT_DIR)/output6_04c.tga -stats -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_04_bunny_mesh_200.txt -output ..\\$(OUTPUT_DIR)/output6_04d.tga -grid 10 10 7 -stats -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_04_bunny_mesh_200.txt -output ..\\$(OUTPUT_DIR)/output6_04e.tga -grid 10 10 7 -visualize_grid
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_05_bunny_mesh_1k.txt -output ..\\$(OUTPUT_DIR)/output6_05.tga -grid 15 15 12 -stats -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_06_bunny_mesh_5k.txt -output ..\\$(OUTPUT_DIR)/output6_06.tga -grid 20 20 15 -stats -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_07_bunny_mesh_40k.txt -output ..\\$(OUTPUT_DIR)/output6_07.tga -grid 40 40 33 -stats -shadows
	@echo === scene6_08 - scene6_10 ===
	$(RT) -input $(INPUT_DIR)/scene6_08_scale_translate.txt -output $(OUTPUT_DIR)/output6_08a.tga
	$(RT) -input $(INPUT_DIR)/scene6_08_scale_translate.txt -output $(OUTPUT_DIR)/output6_08b.tga -grid 15 15 15
	$(RT) -input $(INPUT_DIR)/scene6_08_scale_translate.txt -output $(OUTPUT_DIR)/output6_08c.tga -grid 15 15 15 -visualize_grid
	$(RT) -input $(INPUT_DIR)/scene6_09_rotated_triangles.txt -output $(OUTPUT_DIR)/output6_09a.tga
	$(RT) -input $(INPUT_DIR)/scene6_09_rotated_triangles.txt -output $(OUTPUT_DIR)/output6_09b.tga -grid 15 15 9
	$(RT) -input $(INPUT_DIR)/scene6_09_rotated_triangles.txt -output $(OUTPUT_DIR)/output6_09c.tga -grid 15 15 9 -visualize_grid
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_10_nested_transformations.txt -output ..\\$(OUTPUT_DIR)/output6_10a.tga
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_10_nested_transformations.txt -output ..\\$(OUTPUT_DIR)/output6_10b.tga -grid 30 30 30
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_10_nested_transformations.txt -output ..\\$(OUTPUT_DIR)/output6_10c.tga -grid 30 30 30 -visualize_grid
	@echo === scene6_11 - scene6_12 ===
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_11_mirrored_floor.txt -output ..\\$(OUTPUT_DIR)/output6_11a.tga -shadows -bounces 1 -weight 0.01 -stats
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_11_mirrored_floor.txt -output ..\\$(OUTPUT_DIR)/output6_11b.tga -shadows -bounces 1 -weight 0.01 -grid 40 10 40 -stats
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_11_mirrored_floor.txt -output ..\\$(OUTPUT_DIR)/output6_11c.tga -grid 40 10 40 -visualize_grid
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_12_faceted_gem.txt -output ..\\$(OUTPUT_DIR)/output6_12a.tga -shadows -shade_back -bounces 5 -weight 0.01 -stats
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_12_faceted_gem.txt -output ..\\$(OUTPUT_DIR)/output6_12b.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20 -stats
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_12_faceted_gem.txt -output ..\\$(OUTPUT_DIR)/output6_12c.tga -grid 20 20 20 -visualize_grid
	@echo === scene6_13 - scene6_14 ===
	$(RT) -input $(INPUT_DIR)/scene6_13_checkerboard.txt -output $(OUTPUT_DIR)/output6_13.tga -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene6_14_glass_sphere.txt -output ..\\$(OUTPUT_DIR)/output6_14.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20
	@echo === scene6_15 - scene6_18 ===
	$(MRT300) -input ..\\$(INPUT_DIR)/scene6_15_marble_cubes.txt -output ..\\$(OUTPUT_DIR)/output6_15.tga
	$(MRT300) -input ..\\$(INPUT_DIR)/scene6_16_wood_cubes.txt -output ..\\$(OUTPUT_DIR)/output6_16.tga
	$(MRT300) -input ..\\$(INPUT_DIR)/scene6_17_marble_vase.txt -output ..\\$(OUTPUT_DIR)/output6_17a.tga -grid 15 30 15 -bounces 1 -shadows
	$(MRT300) -input ..\\$(INPUT_DIR)/scene6_17_marble_vase.txt -output ..\\$(OUTPUT_DIR)/output6_17b.tga -grid 15 30 15 -visualize_grid
	$(MRT400) -input ..\\$(INPUT_DIR)/scene6_18_6.837_logo.txt -output ..\\$(OUTPUT_DIR)/output6_18a.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 80 30 3
	$(MRT400) -input ..\\$(INPUT_DIR)/scene6_18_6.837_logo.txt -output ..\\$(OUTPUT_DIR)/output6_18b.tga -grid 80 30 3 -visualize_grid
	@echo === All Sample Results rendered to $(OUTPUT_DIR)/ ===

test: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	$(RT) -input $(INPUT_DIR)/scene6_01_sphere.txt -output $(OUTPUT_DIR)/output6_01a.tga -stats

stats: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	$(RT) -input $(INPUT_DIR)/scene6_01_sphere.txt -output $(OUTPUT_DIR)/output6_01a.tga -stats
	$(RT) -input $(INPUT_DIR)/scene6_01_sphere.txt -output $(OUTPUT_DIR)/output6_01b.tga -grid 10 10 10 -stats
	$(RT) -input $(INPUT_DIR)/scene6_02_sphere_triangles.txt -output $(OUTPUT_DIR)/output6_02a.tga -stats
	$(RT) -input $(INPUT_DIR)/scene6_02_sphere_triangles.txt -output $(OUTPUT_DIR)/output6_02c.tga -stats -shadows

-include Makefile.depend
