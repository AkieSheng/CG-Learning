# Makefile for 6.837 Assignment 4 (Recursive Ray Tracer)
# g++ on Windows MinGW / Linux

CORE_DIR = Core
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
       $(IMAGE_DIR)/image.C \
       $(LA_DIR)/matrix.C \
       $(PARSER_DIR)/scene_parser.C \
       $(GL_DIR)/glCanvas.C

LIBS = -lm -lopengl32 -lglu32 -lfreeglut
CC = g++
CFLAGS = -I. -I$(CORE_DIR) -I$(CAMERA_DIR) -I$(OBJECTS_DIR) \
         -I$(IMAGE_DIR) -I$(LA_DIR) -I$(PARSER_DIR) -I$(GL_DIR)
OBJS = $(SRCS:.C=.o)

.PHONY: all depend clean render test

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

# Assignment 4 Sample Results（全部 TGA 输出，不含 -gui）
render: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	@echo === scene4_01 - scene4_04 ===
	$(RT) -input $(INPUT_DIR)/scene4_01_sphere_shadow.txt -output $(OUTPUT_DIR)/output4_01.tga -shadows
	$(RT) -input $(INPUT_DIR)/scene4_02_colored_shadows.txt -output $(OUTPUT_DIR)/output4_02.tga -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_03_mirrored_floor.txt -output ..\\$(OUTPUT_DIR)/output4_03.tga -shadows -bounces 1 -weight 0.01
	$(RT) -input $(INPUT_DIR)/scene4_04_reflective_sphere.txt -output $(OUTPUT_DIR)/output4_04a.tga -shadows -bounces 0 -weight 0.01
	$(RT) -input $(INPUT_DIR)/scene4_04_reflective_sphere.txt -output $(OUTPUT_DIR)/output4_04b.tga -shadows -bounces 1 -weight 0.01
	$(RT) -input $(INPUT_DIR)/scene4_04_reflective_sphere.txt -output $(OUTPUT_DIR)/output4_04c.tga -shadows -bounces 2 -weight 0.01
	$(RT) -input $(INPUT_DIR)/scene4_04_reflective_sphere.txt -output $(OUTPUT_DIR)/output4_04d.tga -shadows -bounces 3 -weight 0.01
	@echo === scene4_05 - scene4_09 ===
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_05_transparent_bar.txt -output ..\\$(OUTPUT_DIR)/output4_05.tga -shadows -bounces 10 -weight 0.01 -shade_back
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_06_transparent_bars.txt -output ..\\$(OUTPUT_DIR)/output4_06a.tga -shadows -bounces 0 -weight 0.01 -shade_back
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_06_transparent_bars.txt -output ..\\$(OUTPUT_DIR)/output4_06b.tga -shadows -bounces 1 -weight 0.01 -shade_back
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_06_transparent_bars.txt -output ..\\$(OUTPUT_DIR)/output4_06c.tga -shadows -bounces 2 -weight 0.01 -shade_back
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_06_transparent_bars.txt -output ..\\$(OUTPUT_DIR)/output4_06d.tga -shadows -bounces 3 -weight 0.01 -shade_back
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_06_transparent_bars.txt -output ..\\$(OUTPUT_DIR)/output4_06e.tga -shadows -bounces 4 -weight 0.01 -shade_back
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_06_transparent_bars.txt -output ..\\$(OUTPUT_DIR)/output4_06f.tga -shadows -bounces 5 -weight 0.01 -shade_back
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_07_transparent_sphere_1.0.txt -output ..\\$(OUTPUT_DIR)/output4_07.tga -shadows -bounces 5 -weight 0.01 -shade_back
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_08_transparent_sphere_1.1.txt -output ..\\$(OUTPUT_DIR)/output4_08.tga -shadows -bounces 5 -weight 0.01 -shade_back
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_09_transparent_sphere_2.0.txt -output ..\\$(OUTPUT_DIR)/output4_09.tga -shadows -bounces 5 -weight 0.01 -shade_back
	@echo === scene4_10 - scene4_14 ===
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_10_point_light_distance.txt -output ..\\$(OUTPUT_DIR)/output4_10.tga -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_11_point_light_circle.txt -output ..\\$(OUTPUT_DIR)/output4_11.tga -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_12_point_light_circle_d_attenuation.txt -output ..\\$(OUTPUT_DIR)/output4_12.tga -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_13_point_light_circle_d2_attenuation.txt -output ..\\$(OUTPUT_DIR)/output4_13.tga -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_14_faceted_gem.txt -output ..\\$(OUTPUT_DIR)/output4_14a.tga -shadows -shade_back -bounces 0 -weight 0.01
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_14_faceted_gem.txt -output ..\\$(OUTPUT_DIR)/output4_14b.tga -shadows -shade_back -bounces 1 -weight 0.01
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_14_faceted_gem.txt -output ..\\$(OUTPUT_DIR)/output4_14c.tga -shadows -shade_back -bounces 2 -weight 0.01
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_14_faceted_gem.txt -output ..\\$(OUTPUT_DIR)/output4_14d.tga -shadows -shade_back -bounces 3 -weight 0.01
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_14_faceted_gem.txt -output ..\\$(OUTPUT_DIR)/output4_14e.tga -shadows -shade_back -bounces 4 -weight 0.01
	$(MRT) -input ..\\$(INPUT_DIR)/scene4_14_faceted_gem.txt -output ..\\$(OUTPUT_DIR)/output4_14f.tga -shadows -shade_back -bounces 5 -weight 0.01
	@echo === All Sample Results rendered to $(OUTPUT_DIR)/ ===

test: $(EXE)
	@if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
	$(RT) -input $(INPUT_DIR)/scene4_01_sphere_shadow.txt -output $(OUTPUT_DIR)/output4_01.tga -shadows

-include Makefile.depend
