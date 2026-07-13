# Makefile for 6.837 Assignment 8 (Curve Editor)
# g++ on Windows MinGW / Linux

LA_DIR = LinearAlgebraLibrary
ARG_DIR = ArgParser
GL_DIR = GL
PARSER_DIR = Parser
MESH_DIR = Mesh
SPLINE_DIR = Spline
INPUT_DIR = InputFiles
OUTPUT_DIR = OutputFiles

SRCS = main.C \
       $(LA_DIR)/matrix.C \
       $(GL_DIR)/glCanvas.C \
       $(PARSER_DIR)/spline_parser.C \
       $(MESH_DIR)/triangle_mesh.C \
       $(MESH_DIR)/triangle_net.C \
       $(SPLINE_DIR)/spline.C \
       $(SPLINE_DIR)/curve.C \
       $(SPLINE_DIR)/bezier_curve.C \
       $(SPLINE_DIR)/bspline_curve.C \
       $(SPLINE_DIR)/surface.C \
       $(SPLINE_DIR)/surface_of_revolution.C \
       $(SPLINE_DIR)/bezier_patch.C

LIBS = -lm -lopengl32 -lglu32 -lfreeglut
CC = g++
CFLAGS = -I. -I$(LA_DIR) -I$(ARG_DIR) -I$(GL_DIR) -I$(PARSER_DIR) -I$(MESH_DIR) -I$(SPLINE_DIR)
EXE = curve_editor
OBJS = $(SRCS:.C=.o)

.PHONY: all depend clean

all: depend $(EXE)

depend:
	$(CC) $(CFLAGS) -E -M $(SRCS) > Makefile.depend

clean:
	-del /Q *.o $(EXE) Makefile.depend 2>nul
	-del /Q $(LA_DIR)\*.o $(GL_DIR)\*.o $(PARSER_DIR)\*.o $(MESH_DIR)\*.o $(SPLINE_DIR)\*.o 2>nul

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.C.o:
	$(CC) $(CFLAGS) -c -o $@ $<

-include Makefile.depend
