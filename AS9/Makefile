# Makefile for 6.837 Assignment 9 (Particle Systems)
# g++ on Windows MinGW / Linux

LA_DIR = LinearAlgebraLibrary
GL_DIR = GL
PARSER_DIR = Parser
PARTICLE_DIR = Particle
CORE_DIR = Core
FF_DIR = ForceField
INT_DIR = Integrator
GEN_DIR = Generator
INPUT_DIR = InputFiles
OUTPUT_DIR = OutputFiles

SRCS = main.C \
       $(LA_DIR)/matrix.C \
       $(GL_DIR)/glCanvas.C \
       $(PARSER_DIR)/parser.C \
       $(PARTICLE_DIR)/particle.C \
       $(CORE_DIR)/system.C \
       $(FF_DIR)/force_field.C \
       $(FF_DIR)/gravity_force_field.C \
       $(FF_DIR)/constant_force_field.C \
       $(FF_DIR)/radial_force_field.C \
       $(FF_DIR)/vertical_force_field.C \
       $(FF_DIR)/wind_force_field.C \
       $(INT_DIR)/integrator.C \
       $(INT_DIR)/euler_integrator.C \
       $(INT_DIR)/midpoint_integrator.C \
       $(INT_DIR)/runge_kutta_integrator.C \
       $(GEN_DIR)/generator.C \
       $(GEN_DIR)/hose_generator.C \
       $(GEN_DIR)/ring_generator.C

LIBS = -lm -lopengl32 -lglu32 -lfreeglut
CC = g++
CFLAGS = -I. -I$(LA_DIR) -I$(GL_DIR) -I$(PARSER_DIR) -I$(PARTICLE_DIR) \
         -I$(CORE_DIR) -I$(FF_DIR) -I$(INT_DIR) -I$(GEN_DIR)
EXE = particle_system
OBJS = $(SRCS:.C=.o)

.PHONY: all depend clean

all: depend $(EXE)

depend:
	$(CC) $(CFLAGS) -E -M $(SRCS) > Makefile.depend

clean:
	-del /Q *.o $(EXE) $(EXE).exe Makefile.depend 2>nul
	-del /Q $(LA_DIR)\*.o $(GL_DIR)\*.o $(PARSER_DIR)\*.o $(PARTICLE_DIR)\*.o $(CORE_DIR)\*.o 2>nul
	-del /Q $(FF_DIR)\*.o $(INT_DIR)\*.o $(GEN_DIR)\*.o 2>nul

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.C.o:
	$(CC) $(CFLAGS) -c -o $@ $<

-include Makefile.depend
