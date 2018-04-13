LD      := gcc-6
LDFLAGS := -O3 -std=gnu11 -flto -lrt -lpthread -lnuma -libverbs -lmemcached  -lrdmacm#-lpapi
CC = gcc-6
CFLAGS =  -O3 -std=gnu11 -I ../libhrd/ -I../../include/mica -I../../include/libhrd
APPS    := test

all: ${APPS}

test: ../libhrd/hrd_conn.o ../libhrd/hrd_util.o mica.o city.o test.o
	${LD} -o $@ $^ ${LDFLAGS}
	rm -rd test.o test

PHONY: clean
clean:
	rm -f *.o ../libhrd/*.o ${APPS}
