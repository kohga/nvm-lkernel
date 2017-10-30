PWD:=$(shell pwd)

all: mmpram mmtxtpram mlupram cp workspace/extwork

cp: mmpram mmtxtpram
	cp mmpram workspace/
	cp mmtxtpram workspace/
	cp maptxt_ maptxt
	cp maptxt workspace/
	cp mlupram workspace/

workspace/extwork:
	ln -sv /usr/src/nvm-lkernel/test_app/ext/workspace workspace/extwork

mmpram: mm_pram.c
	gcc -o mmpram -D_GNU_SOURCE mm_pram.c

mmtxtpram: mmtxt_pram.c
	gcc -o mmtxtpram -D_GNU_SOURCE mmtxt_pram.c

mlupram: mluti_pram.c
	gcc -o mlupram -D_GNU_SOURCE mluti_pram.c

clean:
	rm -f mmpram mmtxtpram

cleanwork:
	rm workspace/*
