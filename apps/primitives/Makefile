# C

CC=clang
CFLAGS_TEST=-I. -g -O0
CFLAGS_TIME=-I. -O3

CFILES=read_yuv.c main.c intra.c common.c control.c transform.c

test: $(CFILES)
	$(CC) $(CFLAGS_TEST) -DDEBUG -o test $(CFILES)

time: $(CFILES)
	$(CC) $(CFLAGS_TIME) -DTIME -o time_c $(CFILES)

fastll: $(CFILES)
	$(CC) $(CFLAGS_TIME)  -emit-llvm -S  $(CFILES)

timetest: $(CFILES)
	$(CC) $(CFLAGS_TIME) -DTIME -DDEBUG -o timetest_c $(CFILES)

clean:
	rm -rf *.o yuv.out test test.dSYM time_c timetest_c main

# SEQ

cola_test: read_yuv.seq main.seq
	SEQ_DEBUG=t colac run main.seq

cola_time: read_yuv.seq main.seq
	SEQ_DEBUG=t colac run -release main.seq
