all:
	@delta -c *.delta `pkg-config sdl2 --cflags`
	@cc output.o -no-pie `pkg-config sdl2 --libs --static`
	@rm output.o
