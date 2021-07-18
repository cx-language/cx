all:
	@cx -c *.cx `pkg-config sdl2 --cflags`
	@cc output.o `pkg-config sdl2 --libs --static`
	@rm output.o
