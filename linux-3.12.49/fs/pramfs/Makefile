#
# Makefile for the linux pram-filesystem routines.
#

obj-$(CONFIG_PRAMFS) += pramfs.o
obj-$(CONFIG_PRAMFS_TEST_MODULE) += pramfs_test.o

pramfs-y := balloc.o dir.o file.o inode.o namei.o super.o symlink.o ioctl.o

pramfs-$(CONFIG_PRAMFS_WRITE_PROTECT) += wprotect.o
pramfs-$(CONFIG_PRAMFS_XIP) += xip.o
pramfs-$(CONFIG_PRAMFS_XATTR) += xattr.o xattr_user.o xattr_trusted.o desctree.o
pramfs-$(CONFIG_PRAMFS_POSIX_ACL) += acl.o
pramfs-$(CONFIG_PRAMFS_SECURITY) += xattr_security.o
