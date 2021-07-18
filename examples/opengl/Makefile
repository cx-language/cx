SHELL := bash

ifneq ($(OS),Windows_NT)
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        FLAGS += -framework OpenGL
    else ifeq ($(UNAME_S),Linux)
        FLAGS += -DGL_GLEXT_PROTOTYPES
        FLAGS += $$(pkg-config --libs gl)
    endif
endif

all:
	cx build -DGLFW_INCLUDE_GLCOREARB $$(pkg-config --cflags --libs glfw3) $(FLAGS)
