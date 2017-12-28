PWD:=$(shell pwd)

all: mmpram mmtxtpram mlupram mlu1 mlu2 mtxt mtxt_cow cp workspace/extwork

cp: mmpram mmtxtpram
	cp mmpram workspace/
	cp mmtxtpram workspace/
	cp maptxt_ maptxt
	cp maptxt workspace/
	cp mlupram workspace/
	cp mlu1 workspace/
	cp mlu2 workspace/
	cp mtxt workspace/
	cp mtxt_cow workspace/

workspace/extwork:
	ln -sv /usr/src/nvm-lkernel/test_app/ext/workspace workspace/extwork

mmpram: mm_pram.c
	gcc -o mmpram mm_pram.c

mmtxtpram: mmtxt_pram.c
	gcc -o mmtxtpram mmtxt_pram.c

mlupram: mluti_pram.c
	gcc -o mlupram mluti_pram.c

mlu1: mluti_1.c
	gcc -o mlu1 mluti_1.c

mlu2: mluti_2.c
	gcc -o mlu2 mluti_2.c

mtxt: mtxt.c
	gcc -o mtxt mtxt.c

mtxt_cow: mtxt_cow.c
	gcc -o mtxt_cow mtxt_cow.c

clean:
	rm -f mmpram mmtxtpram

cleanwork:
	rm workspace/*
