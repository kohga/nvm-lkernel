#!/bin/bash
#
# PRAMFS kernel patch script
#
# Copyright (C) 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# Patch PRAMFS into the kernel
#

LINUXDIR=../linux-3.12.49

# Display usage of this script
usage () {
	echo "usage:  $0 kernelpath"
	exit 1
}

if [ -z $LINUXDIR ]
then
    usage;
fi

# Check if kerneldir contains a Makefile
if [ ! -f $LINUXDIR/Makefile ]
then
	echo "Directory $LINUXDIR does not exist or is not a kernel source directory";
	exit 1;
fi

KCONFIG=$LINUXDIR/fs/Kconfig
KCONFIGARCH=$LINUXDIR/arch/Kconfig
KCONFIGX86ARCH=$LINUXDIR/arch/x86/Kconfig
KCONFIGTMP=$LINUXDIR/fs/Kconfig.tmp
KCONFIGARCHOLD=$LINUXDIR/arch/Kconfig.pre.pramfs
KCONFIGX86ARCHOLD=$LINUXDIR/arch/x86/Kconfig.pre.pramfs
KCONFIGOLD=$LINUXDIR/fs/Kconfig.pre.pramfs
KBUILDUAPI=$LINUXDIR/include/uapi/linux/Kbuild
MAGIC=$LINUXDIR/include/uapi/linux/magic.h
MAGICOLD=$LINUXDIR/include/uapi/linux/magic.pre.pramfs
XIP=$LINUXDIR/Documentation/filesystems/xip.txt
XIPOLD=$LINUXDIR/Documentation/filesystems/xip.pre.pramfs
XIPFILEMAP=$LINUXDIR/mm/filemap_xip.c
XIPFILEMAPOLD=$LINUXDIR/mm/filemap_xip.c.pramfs
FS=$LINUXDIR/include/linux/fs.h
FSTMP=$LINUXDIR/include/linux/fs.h.tmp
FSOLD=$LINUXDIR/include/linux/fs.h.pramfs
MAINT=$LINUXDIR/MAINTAINERS
MAINTOLD=$LINUXDIR/MAINTAINERS.pre.pramfs
PRAMFS_PATCHED_STRING=`grep -s pramfs <$KCONFIG | head -n 1`

MAKEFILE=$LINUXDIR/fs/Makefile

if [ ! -z "$PRAMFS_PATCHED_STRING" ]
then
    echo "$KCONFIG already mentions PRAMFS, so we will not change it"
else
    echo "Updating $KCONFIG"
    mv -f $KCONFIG  $KCONFIGOLD
    sed -i "s/depends on EXT2_FS_XIP/depends on EXT2_FS_XIP || PRAMFS_XIP/g" $KCONFIGOLD
    sed -i "s/default m if EXT2_FS_XATTR || EXT3_FS_XATTR || EXT4_FS/default m if EXT2_FS_XATTR || EXT3_FS_XATTR || EXT4_FS || PRAMFS_XATTR/g" $KCONFIGOLD

    sed -n -e "/EXT2_FS_XATTR || EXT3_FS_XATTR/,99999 ! p" $KCONFIGOLD > $KCONFIGTMP
    echo -e "\tdefault y if PRAMFS=y && PRAMFS_XATTR" >> $KCONFIGTMP
    sed -n -e "/EXT2_FS_XATTR || EXT3_FS_XATTR/,99999 p" $KCONFIGOLD >> $KCONFIGTMP

    mv -f $KCONFIGTMP $KCONFIGOLD

    sed -n -e "/[Ee][Xx][Oo][Ff][Ss]/,99999 ! p" $KCONFIGOLD > $KCONFIG
    echo "source \"fs/pramfs/Kconfig\"" >> $KCONFIG
    sed -n -e "/[Ee][Xx][Oo][Ff][Ss]/,99999 p" $KCONFIGOLD >> $KCONFIG

    rm -rf $KCONFIGOLD

    echo "Updating $KCONFIGARCH"
    mv -f $KCONFIGARCH  $KCONFIGARCHOLD
    sed -n -e "/config HAVE_KPROBES/,99999 ! p" $KCONFIGARCHOLD > $KCONFIGARCH
    echo "config HAVE_SET_MEMORY_RO" >> $KCONFIGARCH
    echo -e "\tbool" >> $KCONFIGARCH
    echo "" >> $KCONFIGARCH
    sed -n -e "/config HAVE_KPROBES/,99999 p" $KCONFIGARCHOLD >> $KCONFIGARCH
    rm -rf $KCONFIGARCHOLD

    echo "Updating $KCONFIGX86ARCH"
    mv -f $KCONFIGX86ARCH $KCONFIGX86ARCHOLD
    sed -n -e "/select HAVE_IOREMAP_PROT/,99999 ! p" $KCONFIGX86ARCHOLD > $KCONFIGX86ARCH
    echo -e "\tselect HAVE_SET_MEMORY_RO" >> $KCONFIGX86ARCH
    sed -n -e "/select HAVE_IOREMAP_PROT/,99999 p" $KCONFIGX86ARCHOLD >> $KCONFIGX86ARCH
    rm -rf $KCONFIGX86ARCHOLD

    echo "Updating $KBUILDUAPI"
    echo "header-y += pram_fs.h" >> $KBUILDUAPI

    echo "Updating $XIPFILEMAP"
    mv -f $XIPFILEMAP $XIPFILEMAPOLD
    sed -i "s/static int xip_file_fault/int xip_file_fault/g" $XIPFILEMAPOLD
    sed -n -e "/static const struct vm_operations_struct xip_file_vm_ops = {/,99999 ! p" $XIPFILEMAPOLD > $XIPFILEMAP
    echo -e "EXPORT_SYMBOL_GPL(xip_file_fault);" >> $XIPFILEMAP
    sed -n -e "/static const struct vm_operations_struct xip_file_vm_ops = {/,99999 p" $XIPFILEMAPOLD >> $XIPFILEMAP
    rm -f $XIPFILEMAPOLD

    echo "Updating $FS"
    mv -f $FS $FSOLD
    sed -n -e "/struct vm_area_struct;/,99999 ! p" $FSOLD > $FSTMP
    echo -e "struct vm_fault;" >> $FSTMP
    sed -n -e "/struct vm_area_struct;/,99999 p" $FSOLD >> $FSTMP
    mv -f $FSTMP $FSOLD
    sed -n -e "/extern int xip_file_mmap/,99999 ! p" $FSOLD > $FS
    echo -e "extern int xip_file_fault(struct vm_area_struct *vma, struct vm_fault *vmf);" >> $FS
    sed -n -e "/extern int xip_file_mmap/,99999 p" $FSOLD >> $FS
    rm -f $FSOLD

    echo "Updating $XIP"
    mv -f $XIP $XIPOLD
    sed -n -e "/- ext2: the second extended filesystem, see Documentation\/filesystems\/ext2.txt/,99999 ! p" $XIPOLD > $XIP
    echo -e "- pramfs: persistent and protected RAM filesystem, see\n  Documentation/filesystems/pramfs.txt" >> $XIP
    sed -n -e "/- ext2: the second extended filesystem, see Documentation\/filesystems\/ext2.txt/,99999 p" $XIPOLD >> $XIP
    rm -rf $XIPOLD

    echo "Updating $MAINT"
    mv -f $MAINT $MAINTOLD
    sed -n -e "/PREEMPTIBLE KERNEL/,99999 ! p" $MAINTOLD > $MAINT
    echo -e "PRAM FILE SYSTEM" >> $MAINT
    echo -e "W:\thttp://pramfs.sourceforge.net" >> $MAINT
    echo -e "L:\tpramfs-devel@lists.sourceforge.net (subscribers-only)" >> $MAINT
    echo -e "S:\tMaintained" >> $MAINT
    echo -e "F:\tDocumentation/filesystems/pramfs.txt" >> $MAINT
    echo -e "F:\tfs/pramfs/" >> $MAINT
    echo -e "F:\tinclude/linux/pram*" >> $MAINT
    echo -e "" >> $MAINT
    sed -n -e "/PREEMPTIBLE KERNEL/,99999 p" $MAINTOLD >> $MAINT
    rm -rf $MAINTOLD

    #now do fs/Makefile -- simply add the target at the end
    echo "Updating $MAKEFILE"
    echo -e "obj-\$(CONFIG_PRAMFS)\t\t+= pramfs/" >>$MAKEFILE

    echo "Updating $MAGIC"
    mv -f $MAGIC $MAGICOLD
    sed -n -e "/QNX4_SUPER_MAGIC/,99999 ! p" $MAGICOLD > $MAGIC
    echo -e "#define PRAM_SUPER_MAGIC\t0xEFFA" >> $MAGIC
    sed -n -e "/QNX4_SUPER_MAGIC/,99999 p" $MAGICOLD >> $MAGIC
    rm -rf $MAGICOLD
fi

PRAMFSDIR=$LINUXDIR/fs/pramfs

if [ -e $PRAMFSDIR ]
then
   echo "$PRAMFSDIR exists, so not patching. If you want to replace what is"
   echo "already there then delete $PRAMFSDIR and re-run this script"
   echo " eg.  \"rm -rf $PRAMFSDIR\" "
else
   mkdir $LINUXDIR/fs/pramfs
   cp $PWD/Makefile $LINUXDIR/fs/pramfs/Makefile
   cp $PWD/Kconfig $LINUXDIR/fs/pramfs
   cp $PWD/pramfs.txt $LINUXDIR/Documentation/filesystems/pramfs.txt
   cp $PWD/*.c $LINUXDIR/fs/pramfs
   cp $PWD/acl.h $PWD/xattr.h $PWD/desctree.h $PWD/pram.h $PWD/wprotect.h $PWD/xip.h $LINUXDIR/fs/pramfs
   cp $PWD/pram_fs.h $LINUXDIR/include/linux
   cp $PWD/pram_fs_uapi.h $LINUXDIR/include/uapi/linux/pram_fs.h
fi
