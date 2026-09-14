SHELL := bash

all:
	cx build $$(pkg-config sdl3 --cflags --libs-only-l --libs-only-L) -lm $(CXFLAGS)
