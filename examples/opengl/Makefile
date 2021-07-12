ifneq ($(OS),Windows_NT)
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        CC_FLAGS += -framework OpenGL
    endif
    ifeq ($(UNAME_S),Linux)
        CX_FLAGS += -DGL_GLEXT_PROTOTYPES
        CC_FLAGS += $$(pkg-config --libs gl) -no-pie
    endif
endif

all:
	cx -c *.cx -DGLFW_INCLUDE_GLCOREARB $$(pkg-config --cflags glfw3) $(CX_FLAGS)
	cc output.o $$(pkg-config --libs glfw3) $(CC_FLAGS)
	rm output.o
