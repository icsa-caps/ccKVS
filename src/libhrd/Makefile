LD      := gcc-6
LDFLAGS := -03 -std=gnu11 -flto -libverbs -lrt -lpthread -lmemcached -lnuma -lrdmacm
CC = gcc-6
CFLAGS = -O3   -I../../include/libhrd

APPS    := main

all: ${APPS}

main: hrd_conn.o hrd_util.o main.o
	${LD} -o $@ $^ ${LDFLAGS}
	rm -rf main main.o

PHONY: clean
clean:
	rm -f *.o ${APPS}
