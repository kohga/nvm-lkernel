#!/bin/sh

gcc -D_GNU_SOURCE -w -o fwrite fwrite.c
gcc -D_GNU_SOURCE -w -o fmmap fmmap.c
gcc -D_GNU_SOURCE -w -o mapwrite mapwrite.c
gcc -D_GNU_SOURCE -w -o syswrite syswrite.c
gcc -D_GNU_SOURCE -w -o mapread mapread.c
gcc -w -o mapc mapc.c
gcc -D_GNU_SOURCE -w -o txtmap txtmap.c
gcc -D_GNU_SOURCE -w -o txtio txtio.c
gcc -w -o cowtest cowtest.c
gcc -w -o maprw maprw.c
