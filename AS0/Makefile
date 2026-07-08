# Simple Makefile for 6.837 assignment 0
# for g++ on linux / mingw platforms
# ===============================================================

IMAGE_DIR = ImageLibrary
LA_DIR = LinearAlgebraLibrary

SRCS = main.C \
       ifs.C \
       $(IMAGE_DIR)/image.C \
       $(LA_DIR)/matrix.C

# ===============================================================

LIBS = -lm
CC = g++
CFLAGS = -I$(IMAGE_DIR) -I$(LA_DIR)
EXE = ifs
OBJS = $(SRCS:.C=.o)

# ===============================================================
# targets

.PHONY: all depend clean

all: depend $(EXE)

depend:
	$(CC) $(CFLAGS) -E -M $(SRCS) > Makefile.depend

clean:
	rm -f *~ *bak *.o $(EXE) Makefile.depend
	rm -f $(IMAGE_DIR)/*.o $(LA_DIR)/*.o

# ===============================================================
# compilation rules

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.C.o:
	$(CC) $(CFLAGS) -c -o $@ $<

# ===============================================================

-include Makefile.depend

# DO NOT DELETE
