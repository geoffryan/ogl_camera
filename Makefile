APP = paint

default: $(APP)

LD_SDL := $(shell sdl2-config --libs)
CC_SDL := $(shell sdl2-config --cflags)

LD_GL = -framework OpenGL
CC_GL = 

LDFLAGS = $(LD_GL) $(LD_SDL)
CCFLAGS = $(CC_GL) $(CC_SDL) -g -Wall

.PHONY: clean

$(APP): main.c
	gcc -o $(APP) main.c $(CCFLAGS) $(LDFLAGS)

clean:
	rm -f $(APP)
