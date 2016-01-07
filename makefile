crekub_lib: crekub_lib.o
crekub_lib.o: crekub_lib.c text.h memory.h error.h
	cc -c crekub_lib.c
