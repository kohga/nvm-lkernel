PWD:=$(shell pwd)

all: mmpram mmtxtpram


mmpram: mm_pram.c
	gcc -o mmpram -D_GNU_SOURCE mm_pram.c


mmtxtpram: mmtxt_pram.c
	gcc -o mtxtpram -D_GNU_SOURCE mmtxt_pram.c


mmext: mm_ext.c

mmtxtext: mmtxt_ext.c


clean:
	rm -f mmpram mmtxtpram


