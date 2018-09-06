CC = g++
CFLAGS = -Wall -g
PROG = sample3D

SRCS = sample3D.cpp imageloader.cpp vec3f.cpp
LIBS = -lglut -lGL -lGLU 

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)
