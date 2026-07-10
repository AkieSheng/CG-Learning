# Makefile for 6.837 Assignment 7 (Supersampling and Antialiasing)
# g++ on Windows MinGW / Linux

CORE_DIR = Core
SPATIAL_DIR = Spatial
CAMERA_DIR = Camera
OBJECTS_DIR = Objects
IMAGE_DIR = ImageLibrary
LA_DIR = LinearAlgebraLibrary
PARSER_DIR = Parser
MATERIALS_DIR = Materials
PROCEDURAL_DIR = Procedural
STATS_DIR = Stats
FILM_DIR = Film
SAMPLER_DIR = Sampler
FILTER_DIR = Filter
INPUT_DIR = InputFiles
MESH_DIR = Meshes
OUTPUT_DIR = OutputFiles
EXE = raytracer

# 无 mesh：在项目根目录运行
RT = $(EXE)
# 有 mesh：在 Meshes/ 下运行
MRT = cd $(MESH_DIR) && ..\\$(EXE)

SRCS = main.C \
       $(CORE_DIR)/object3d.C \
       $(CAMERA_DIR)/camera.C \
       $(OBJECTS_DIR)/group.C \
       $(OBJECTS_DIR)/sphere.C \
       $(OBJECTS_DIR)/plane.C \
       $(OBJECTS_DIR)/triangle.C \
       $(OBJECTS_DIR)/transform.C \
       $(CORE_DIR)/material.C \
       $(CORE_DIR)/rayTracer.C \
       $(SPATIAL_DIR)/boundingbox.C \
       $(SPATIAL_DIR)/grid.C \
       $(SPATIAL_DIR)/marchinginfo.C \
       $(IMAGE_DIR)/image.C \
       $(LA_DIR)/matrix.C \
       $(PARSER_DIR)/scene_parser.C \
       $(MATERIALS_DIR)/checkerboard.C \
       $(MATERIALS_DIR)/noise.C \
       $(MATERIALS_DIR)/marble.C \
       $(MATERIALS_DIR)/wood.C \
       $(PROCEDURAL_DIR)/perlin_noise.C \
       $(STATS_DIR)/raytracing_stats.C \
       $(FILM_DIR)/film.C \
       $(SAMPLER_DIR)/random_sampler.C \
       $(SAMPLER_DIR)/uniform_sampler.C \
       $(SAMPLER_DIR)/jittered_sampler.C \
       $(FILTER_DIR)/filter.C \
       $(FILTER_DIR)/box_filter.C \
       $(FILTER_DIR)/tent_filter.C \
       $(FILTER_DIR)/gaussian_filter.C

LIBS = -lm
CC = g++
CFLAGS = -I. -I$(CORE_DIR) -I$(SPATIAL_DIR) -I$(CAMERA_DIR) -I$(OBJECTS_DIR) \
         -I$(IMAGE_DIR) -I$(LA_DIR) -I$(PARSER_DIR) \
         -I$(MATERIALS_DIR) -I$(PROCEDURAL_DIR) -I$(STATS_DIR) \
         -I$(FILM_DIR) -I$(SAMPLER_DIR) -I$(FILTER_DIR)
OBJS = $(SRCS:.C=.o)

.PHONY: all depend clean render render01 render02 render03 render04 render05 render06

all: depend $(EXE)

depend:
	$(CC) $(CFLAGS) -E -M $(SRCS) > Makefile.depend

clean:
	-del /Q *.o $(EXE) Makefile.depend 2>nul
	-del /Q $(CORE_DIR)\*.o $(SPATIAL_DIR)\*.o $(CAMERA_DIR)\*.o $(OBJECTS_DIR)\*.o 2>nul
	-del /Q $(IMAGE_DIR)\*.o $(LA_DIR)\*.o $(PARSER_DIR)\*.o 2>nul
	-del /Q $(MATERIALS_DIR)\*.o $(PROCEDURAL_DIR)\*.o $(STATS_DIR)\*.o 2>nul
	-del /Q $(FILM_DIR)\*.o $(SAMPLER_DIR)\*.o $(FILTER_DIR)\*.o 2>nul

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.C.o:
	$(CC) $(CFLAGS) -c -o $@ $<

# OutputFiles/scene7_XX/{outputs,samples,filters}/
OUT01 = $(OUTPUT_DIR)/scene7_01
OUT02 = $(OUTPUT_DIR)/scene7_02
OUT03 = $(OUTPUT_DIR)/scene7_03
OUT04 = $(OUTPUT_DIR)/scene7_04
OUT05 = $(OUTPUT_DIR)/scene7_05
OUT06 = $(OUTPUT_DIR)/scene7_06

# Sample Results（说明见 OutputFiles/output_catalog.txt）
render: render01 render02 render03 render04 render05 render06
	@echo === All Sample Results rendered to $(OUTPUT_DIR)/ ===

# --- scene7_01 ---
render01: $(EXE)
	@if not exist $(OUT01)\outputs mkdir $(OUT01)\outputs
	@if not exist $(OUT01)\samples mkdir $(OUT01)\samples
	@if not exist $(OUT01)\filters mkdir $(OUT01)\filters
	@echo === scene7_01 sphere_triangle ===
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 180 180 -output $(OUT01)/outputs/output7_01.tga
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_samples $(OUT01)/samples/samples7_01a.tga 20 -random_samples 4
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_samples $(OUT01)/samples/samples7_01b.tga 20 -uniform_samples 4
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_samples $(OUT01)/samples/samples7_01c.tga 20 -jittered_samples 4
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_samples $(OUT01)/samples/samples7_01d.tga 20 -random_samples 9
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_samples $(OUT01)/samples/samples7_01e.tga 20 -uniform_samples 9
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_samples $(OUT01)/samples/samples7_01f.tga 20 -jittered_samples 9
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_samples $(OUT01)/samples/samples7_01g.tga 20 -random_samples 36
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_samples $(OUT01)/samples/samples7_01h.tga 20 -uniform_samples 36
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_samples $(OUT01)/samples/samples7_01i.tga 20 -jittered_samples 36
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_filter $(OUT01)/filters/filter7_01a.tga 20 -box_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_filter $(OUT01)/filters/filter7_01b.tga 20 -tent_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_filter $(OUT01)/filters/filter7_01c.tga 20 -gaussian_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_filter $(OUT01)/filters/filter7_01d.tga 20 -box_filter 1.7
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_filter $(OUT01)/filters/filter7_01e.tga 20 -tent_filter 1.7
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_filter $(OUT01)/filters/filter7_01f.tga 20 -gaussian_filter 1.7
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_filter $(OUT01)/filters/filter7_01g.tga 20 -box_filter 2.3
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_filter $(OUT01)/filters/filter7_01h.tga 20 -tent_filter 2.3
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 9 9 -render_filter $(OUT01)/filters/filter7_01i.tga 20 -gaussian_filter 2.3
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -output $(OUT01)/outputs/output7_01_low_res.tga
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -render_samples $(OUT01)/samples/samples7_01a_low_res.tga 15 -random_samples 9
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -render_samples $(OUT01)/samples/samples7_01b_low_res.tga 15 -uniform_samples 9
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -render_samples $(OUT01)/samples/samples7_01c_low_res.tga 15 -jittered_samples 9
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -render_filter $(OUT01)/filters/filter7_01a_low_res.tga 15 -box_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -render_filter $(OUT01)/filters/filter7_01b_low_res.tga 15 -tent_filter 1.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -render_filter $(OUT01)/filters/filter7_01c_low_res.tga 15 -gaussian_filter 1.0
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -output $(OUT01)/outputs/output7_01a_low_res.tga -random_samples 9 -box_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -output $(OUT01)/outputs/output7_01b_low_res.tga -random_samples 9 -tent_filter 1.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -output $(OUT01)/outputs/output7_01c_low_res.tga -random_samples 9 -gaussian_filter 1.0
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -output $(OUT01)/outputs/output7_01d_low_res.tga -uniform_samples 9 -box_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -output $(OUT01)/outputs/output7_01e_low_res.tga -uniform_samples 9 -tent_filter 1.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -output $(OUT01)/outputs/output7_01f_low_res.tga -uniform_samples 9 -gaussian_filter 1.0
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -output $(OUT01)/outputs/output7_01g_low_res.tga -jittered_samples 9 -box_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -output $(OUT01)/outputs/output7_01h_low_res.tga -jittered_samples 9 -tent_filter 1.5
	$(RT) -input $(INPUT_DIR)/scene7_01_sphere_triangle.txt -size 12 12 -output $(OUT01)/outputs/output7_01i_low_res.tga -jittered_samples 9 -gaussian_filter 1.0

# --- scene7_02 ---
render02: $(EXE)
	@if not exist $(OUT02)\outputs mkdir $(OUT02)\outputs
	@if not exist $(OUT02)\samples mkdir $(OUT02)\samples
	@if not exist $(OUT02)\filters mkdir $(OUT02)\filters
	@echo === scene7_02 checkerboard ===
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 180 180 -output $(OUT02)/outputs/output7_02.tga
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 9 9 -render_samples $(OUT02)/samples/samples7_02a.tga 20 -random_samples 16
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 9 9 -render_samples $(OUT02)/samples/samples7_02b.tga 20 -uniform_samples 16
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 9 9 -render_samples $(OUT02)/samples/samples7_02c.tga 20 -jittered_samples 16
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 9 9 -render_filter $(OUT02)/filters/filter7_02a.tga 20 -box_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 9 9 -render_filter $(OUT02)/filters/filter7_02b.tga 20 -tent_filter 1.5
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 9 9 -render_filter $(OUT02)/filters/filter7_02c.tga 20 -gaussian_filter 0.6
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 180 180 -output $(OUT02)/outputs/output7_02a.tga -random_samples 16 -box_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 180 180 -output $(OUT02)/outputs/output7_02b.tga -random_samples 16 -tent_filter 1.5
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 180 180 -output $(OUT02)/outputs/output7_02c.tga -random_samples 16 -gaussian_filter 0.6
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 180 180 -output $(OUT02)/outputs/output7_02d.tga -uniform_samples 16 -box_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 180 180 -output $(OUT02)/outputs/output7_02e.tga -uniform_samples 16 -tent_filter 1.5
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 180 180 -output $(OUT02)/outputs/output7_02f.tga -uniform_samples 16 -gaussian_filter 0.6
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 180 180 -output $(OUT02)/outputs/output7_02g.tga -jittered_samples 16 -box_filter 0.5
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 180 180 -output $(OUT02)/outputs/output7_02h.tga -jittered_samples 16 -tent_filter 1.5
	$(RT) -input $(INPUT_DIR)/scene7_02_checkerboard.txt -size 180 180 -output $(OUT02)/outputs/output7_02i.tga -jittered_samples 16 -gaussian_filter 0.6

# --- scene7_03 (vase.obj) ---
render03: $(EXE)
	@if not exist $(OUT03)\outputs mkdir $(OUT03)\outputs
	@echo === scene7_03 marble_vase ===
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_03_marble_vase.txt -size 200 200 -output ..\\$(OUT03)/outputs/output7_03a.tga -grid 15 30 15 -shadows
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_03_marble_vase.txt -size 200 200 -output ..\\$(OUT03)/outputs/output7_03b.tga -grid 15 30 15 -shadows -jittered_samples 4 -gaussian_filter 0.4
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_03_marble_vase.txt -size 200 200 -output ..\\$(OUT03)/outputs/output7_03c.tga -grid 15 30 15 -shadows -jittered_samples 9 -gaussian_filter 0.4
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_03_marble_vase.txt -size 200 200 -output ..\\$(OUT03)/outputs/output7_03d.tga -grid 15 30 15 -shadows -jittered_samples 36 -gaussian_filter 0.4

# --- scene7_04 (6.837.obj) ---
render04: $(EXE)
	@if not exist $(OUT04)\outputs mkdir $(OUT04)\outputs
	@echo === scene7_04 6.837_logo ===
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_04_6.837_logo.txt -size 400 200 -output ..\\$(OUT04)/outputs/output7_04a.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 80 30 3
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_04_6.837_logo.txt -size 400 200 -output ..\\$(OUT04)/outputs/output7_04b.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 80 30 3 -jittered_samples 9 -gaussian_filter 0.4

# --- scene7_05 (cube.obj) ---
render05: $(EXE)
	@if not exist $(OUT05)\outputs mkdir $(OUT05)\outputs
	@echo === scene7_05 glass_sphere ===
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_05_glass_sphere.txt -size 300 300 -output ..\\$(OUT05)/outputs/output7_05a.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_05_glass_sphere.txt -size 300 300 -output ..\\$(OUT05)/outputs/output7_05b.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20 -jittered_samples 4 -gaussian_filter 0.4
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_05_glass_sphere.txt -size 300 300 -output ..\\$(OUT05)/outputs/output7_05c.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20 -jittered_samples 16 -gaussian_filter 0.4

# --- scene7_06 (diamond.obj) ---
render06: $(EXE)
	@if not exist $(OUT06)\outputs mkdir $(OUT06)\outputs
	@echo === scene7_06 faceted_gem ===
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_06_faceted_gem.txt -size 200 200 -output ..\\$(OUT06)/outputs/output7_06a.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20
	$(MRT) -input ..\\$(INPUT_DIR)/scene7_06_faceted_gem.txt -size 200 200 -output ..\\$(OUT06)/outputs/output7_06b.tga -shadows -shade_back -bounces 5 -weight 0.01 -grid 20 20 20 -jittered_samples 9 -gaussian_filter 0.4

-include Makefile.depend
