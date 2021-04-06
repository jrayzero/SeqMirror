CC=clang
CFLAGS=-I. -g -O0

test: read_yuv.o main.o
	$(CC) $(CFLAGS) -DDEBUG -o test main.c read_yuv.c
clean:
	rm -rf *.o yuv.out test test.dSYM
