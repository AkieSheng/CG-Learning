# Simple Makefile for 6.837 raycasting assignment
# for g++ on linux / mingw platforms
# ===============================================================

IMAGE_DIR = ImageLibrary
LA_DIR = LinearAlgebraLibrary
PARSER_DIR = Parser
INPUT_DIR = InputFiles
OUTPUT_DIR = OutputFiles

SRCS = main.C \
       camera.C \
       group.C \
       sphere.C \
       $(IMAGE_DIR)/image.C \
       $(LA_DIR)/matrix.C \
       $(PARSER_DIR)/scene_parser.C

# ===============================================================

LIBS = -lm
CC = g++
CFLAGS = -I. -I$(IMAGE_DIR) -I$(LA_DIR) -I$(PARSER_DIR)
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
	rm -f $(IMAGE_DIR)/*.o $(LA_DIR)/*.o $(PARSER_DIR)/*.o

# 渲染全部测试场景
render: $(EXE)
	$(EXE) -input $(INPUT_DIR)/scene1_01.txt -size 200 200 -output $(OUTPUT_DIR)/output1_01.tga -depth 9 10 $(OUTPUT_DIR)/depth1_01.tga
	$(EXE) -input $(INPUT_DIR)/scene1_02.txt -size 200 200 -output $(OUTPUT_DIR)/output1_02.tga -depth 8 12 $(OUTPUT_DIR)/depth1_02.tga
	$(EXE) -input $(INPUT_DIR)/scene1_03.txt -size 200 200 -output $(OUTPUT_DIR)/output1_03.tga -depth 8 12 $(OUTPUT_DIR)/depth1_03.tga
	$(EXE) -input $(INPUT_DIR)/scene1_04.txt -size 200 200 -output $(OUTPUT_DIR)/output1_04.tga -depth 12 17 $(OUTPUT_DIR)/depth1_04.tga
	$(EXE) -input $(INPUT_DIR)/scene1_05.txt -size 200 200 -output $(OUTPUT_DIR)/output1_05.tga -depth 14.5 19.5 $(OUTPUT_DIR)/depth1_05.tga
	$(EXE) -input $(INPUT_DIR)/scene1_06.txt -size 200 200 -output $(OUTPUT_DIR)/output1_06.tga -depth 3 7 $(OUTPUT_DIR)/depth1_06.tga
	$(EXE) -input $(INPUT_DIR)/scene1_07.txt -size 200 200 -output $(OUTPUT_DIR)/output1_07.tga -depth -2 2 $(OUTPUT_DIR)/depth1_07.tga

# ===============================================================
# compilation rules

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.C.o:
	$(CC) $(CFLAGS) -c -o $@ $<

# ===============================================================

-include Makefile.depend
