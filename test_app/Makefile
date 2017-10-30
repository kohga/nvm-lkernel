PWD:=$(shell pwd)

all: mmpram mmtxtpram cp

cp: mmpram mmtxtpram
	cp mmpram /mnt/pram
	cp mmtxtpram /mnt/pram
	cp maptxt_ maptxt
	cp maptxt /mnt/pram

mmpram: mm_pram.c
	gcc -o mmpram -D_GNU_SOURCE mm_pram.c


mmtxtpram: mmtxt_pram.c
	gcc -o mmtxtpram -D_GNU_SOURCE mmtxt_pram.c


mmext: mm_ext.c

mmtxtext: mmtxt_ext.c


clean:
	rm -f mmpram mmtxtpram


