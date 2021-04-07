# C

CC=clang
CFLAGS_TEST=-I. -g -O0
CFLAGS_TIME=-I. -O3

test: read_yuv.c main.c
	$(CC) $(CFLAGS_TEST) -DDEBUG -o test main.c read_yuv.c

time: read_yuv.c main.c
	$(CC) $(CFLAGS_TIME) -DTIME -o time_c main.c read_yuv.c

timetest: read_yuv.c main.c
	$(CC) $(CFLAGS_TIME) -DTIME -DDEBUG -o timetest_c main.c read_yuv.c

clean:
	rm -rf *.o yuv.out test test.dSYM time_c timetest_c

# SEQ

cola_test: read_yuv.seq main.seq
	SEQ_DEBUG=t colac run main.seq

cola_time: read_yuv.seq main.seq
	SEQ_DEBUG=t colac run -release main.seq
