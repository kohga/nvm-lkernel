PWD:=$(shell pwd)

all: mmext mmtxtext mluext cp workspace/pramwork

cp: cleanwork mmext mmtxtext
	cp mmext workspace/
	cp mmtxtext workspace/
	cp maptxt_ maptxt
	cp maptxt workspace/
	cp mluext workspace/

workspace/pramwork:
	ln -sv /mnt/pram workspace/pramwork

mmext: mm_ext.c
	gcc -o mmext -D_GNU_SOURCE mm_ext.c

mmtxtext: mmtxt_ext.c
	gcc -o mmtxtext -D_GNU_SOURCE mmtxt_ext.c

mluext: mluti_ext.c
	gcc -o mluext -D_GNU_SOURCE mluti_ext.c

clean:
	rm -f mmext mmtxtext

cleanwork:
	rm workspace/*
