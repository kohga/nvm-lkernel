/*
 * BRIEF DESCRIPTION
 *
 * File operations for files.
 *
 * Copyright 2009-2011 Marco Stornelli <marco.stornelli@gmail.com>
 * Copyright 2003 Sony Corporation
 * Copyright 2003 Matsushita Electric Industrial Co., Ltd.
 * 2003-2004 (c) MontaVista Software, Inc. , Steve Longerbeam
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/fs.h>
#include <linux/aio.h>
#include <linux/slab.h>
#include <linux/uio.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/falloc.h>
#include "pram.h"
#include "acl.h"
#include "xip.h"
#include "xattr.h"

#include <linux/buffer_head.h>
#define VM_SHARED       0x00000008

/*
 * The following functions are helper routines to copy to/from
 * user space and iter over io vectors (mainly for readv/writev).
 * They are used in the direct IO path.
 */
static size_t __pram_iov_copy_from(char *vaddr,
			const struct iovec *iov, size_t base, size_t bytes)
{
	size_t copied = 0, left = 0;

	while (bytes) {
		char __user *buf = iov->iov_base + base;
		int copy = min(bytes, iov->iov_len - base);

		base = 0;
		left = __copy_from_user(vaddr, buf, copy);
		copied += copy;
		bytes -= copy;
		vaddr += copy;
		iov++;

		if (unlikely(left))
			break;
	}
	return copied - left;
}

static size_t __pram_iov_copy_to(char *vaddr,
			const struct iovec *iov, size_t base, size_t bytes)
{
	size_t copied = 0, left = 0;

	while (bytes) {
		char __user *buf = iov->iov_base + base;
		int copy = min(bytes, iov->iov_len - base);

		base = 0;
		left = __copy_to_user(buf, vaddr, copy);
		copied += copy;
		bytes -= copy;
		vaddr += copy;
		iov++;

		if (unlikely(left))
			break;
	}
	return copied - left;
}

static size_t pram_iov_copy_from(void *to, struct iov_iter *i, size_t bytes)
{
	size_t copied;

	if (likely(i->nr_segs == 1)) {
		int left;
		char __user *buf = i->iov->iov_base + i->iov_offset;
		left = __copy_from_user(to, buf, bytes);
		copied = bytes - left;
	} else {
		copied = __pram_iov_copy_from(to, i->iov, i->iov_offset, bytes);
	}

	return copied;
}

static size_t pram_iov_copy_to(void *from, struct iov_iter *i, size_t bytes)
{
	size_t copied;

	if (likely(i->nr_segs == 1)) {
		int left;
		char __user *buf = i->iov->iov_base + i->iov_offset;
		left = __copy_to_user(buf, from, bytes);
		copied = bytes - left;
	} else {
		copied = __pram_iov_copy_to(from, i->iov, i->iov_offset, bytes);
	}

	return copied;
}

static size_t __pram_clear_user(const struct iovec *iov, size_t base,
				size_t bytes)
{
	size_t claened = 0, left = 0;

	while (bytes) {
		char __user *buf = iov->iov_base + base;
		int clear = min(bytes, iov->iov_len - base);

		base = 0;
		left = __clear_user(buf, clear);
		claened += clear;
		bytes -= clear;
		iov++;

		if (unlikely(left))
			break;
	}
	return claened - left;
}

static size_t pram_clear_user(struct iov_iter *i, size_t bytes)
{
	size_t clear;

	if (likely(i->nr_segs == 1)) {
		int left;
		char __user *buf = i->iov->iov_base + i->iov_offset;
		left = __clear_user(buf, bytes);
		clear = bytes - left;
	} else {
		clear = __pram_clear_user(i->iov, i->iov_offset, bytes);
	}

	return clear;
}

static int pram_open_file(struct inode *inode, struct file *filp)
{
	filp->f_flags |= O_DIRECT;
	return generic_file_open(inode, filp);
}

ssize_t pram_direct_IO(int rw, struct kiocb *iocb,
		   const struct iovec *iov,
		   loff_t offset, unsigned long nr_segs)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file->f_mapping->host;
	struct super_block *sb = inode->i_sb;
	int progress = 0, hole = 0, alloc_once = 1;
	ssize_t retval = 0;
	unsigned long blocknr, blockoff, blocknr_start;
	struct iov_iter iter;
	unsigned int num_blocks;
	size_t length = iov_length(iov, nr_segs);
	loff_t size;

	printk(KERN_DEBUG "pram_direct_IO\n");

	/*
	 * If we are in the write path we are under i_mutex but no lock held
	 * in the read, so we need to be sync with truncate to avoid race
	 * conditions.
	 */
	if (rw == READ)
		rcu_read_lock();

	size = i_size_read(inode);

	if (length < 0) {
		retval = -EINVAL;
		goto out;
	}
	if ((rw == READ) && (offset + length > size))
		length = size - offset;
	if (!length)
		goto out;

	/* find starting block number to access */
	blocknr = offset >> sb->s_blocksize_bits;
	/* find starting offset within starting block */
	blockoff = offset & (sb->s_blocksize - 1);
	/* find number of blocks to access */
	num_blocks = (blockoff + length + sb->s_blocksize - 1) >>
							sb->s_blocksize_bits;
	blocknr_start = blocknr;

	iov_iter_init(&iter, iov, nr_segs, length, 0);

	while (length) {
		int count;
		u8 *bp = NULL;
		u64 block = pram_find_data_block(inode, blocknr);
		if (!block) {
			if (alloc_once && rw == WRITE) {
				/*
				 * Allocate the data blocks starting from
				 * blocknr to the end.
				 */
				retval = pram_alloc_blocks(inode, blocknr,
							num_blocks - (blocknr -
								blocknr_start));
				if (retval)
					goto out;
				/* retry....*/
				block = pram_find_data_block(inode, blocknr);
				BUG_ON(!block);
				alloc_once = 0;
			} else if (unlikely(rw == READ)) {
				/* We are falling in a hole */
				hole = 1;
				goto hole;
			}
		}
		bp = (u8 *)pram_get_block(sb, block);
		if (!bp) {
			retval = -EACCES;
			goto out;
		}
 hole:
		++blocknr;

		count = blockoff + length > sb->s_blocksize ?
			sb->s_blocksize - blockoff : length;

		if (rw == READ) {
			if (unlikely(hole)) {
				retval = pram_clear_user(&iter, count);
				if (retval != count) {
					retval = -EFAULT;
					goto out;
				}
			} else {
				retval = pram_iov_copy_to(&bp[blockoff], &iter,
							  count);
				if (retval != count) {
					retval = -EFAULT;
					goto out;
				}
			}
		} else {
			pram_memunlock_block(sb, bp);
			retval = pram_iov_copy_from(&bp[blockoff], &iter,
							count);
			if (retval != count) {
				retval = -EFAULT;
				pram_memlock_block(sb, bp);
				goto out;
			}
			pram_memlock_block(sb, bp);
		}

		progress += count;
		iov_iter_advance(&iter, count);
		length -= count;
		blockoff = 0;
		hole = 0;
	}

	retval = progress;
	/*
	 * Check for the flag EOFBLOCKS is still valid after the extending
	 * write.
	 */
	if (rw == WRITE && (offset + length >= size))
		check_eof_blocks(inode, size + retval);
 out:
	if (rw == READ)
		rcu_read_unlock();
	return retval;
}

static int pram_check_flags(int flags)
{
	if (!(flags & O_DIRECT))
		return -EINVAL;

	return 0;
}

static long pram_fallocate(struct file *file, int mode, loff_t offset,
			   loff_t len)
{
	struct inode *inode = file_inode(file);
	long ret = 0;
	unsigned long blocknr, blockoff;
	int num_blocks, blocksize_mask;
	struct pram_inode *pi;
	loff_t new_size;

	/* We only support the FALLOC_FL_KEEP_SIZE mode */
	if (mode & ~FALLOC_FL_KEEP_SIZE)
		return -EOPNOTSUPP;

	if (S_ISDIR(inode->i_mode))
		return -ENODEV;

	mutex_lock(&inode->i_mutex);

	new_size = len + offset;
	if (!(mode & FALLOC_FL_KEEP_SIZE) && new_size > inode->i_size) {
		ret = inode_newsize_ok(inode, new_size);
		if (ret)
			goto out;
	}

	blocksize_mask = (1 << inode->i_sb->s_blocksize_bits) - 1;
	blocknr = offset >> inode->i_sb->s_blocksize_bits;
	blockoff = offset & blocksize_mask;
	num_blocks = (blockoff + len + blocksize_mask) >>
						inode->i_sb->s_blocksize_bits;
	ret = pram_alloc_blocks(inode, blocknr, num_blocks);
	if (ret)
		goto out;

	if (mode & FALLOC_FL_KEEP_SIZE) {
		pi = pram_get_inode(inode->i_sb, inode->i_ino);
		if (!pi) {
			ret = -EACCES;
			goto out;
		}
		pram_memunlock_inode(inode->i_sb, pi);
		pi->i_flags |= cpu_to_be32(PRAM_EOFBLOCKS_FL);
		pram_memlock_inode(inode->i_sb, pi);
	}

	inode->i_mtime = inode->i_ctime = CURRENT_TIME_SEC;
	if (!(mode & FALLOC_FL_KEEP_SIZE) && new_size > inode->i_size)
		inode->i_size = new_size;
	ret = pram_update_inode(inode);
 out:
	mutex_unlock(&inode->i_mutex);
	return ret;
}

loff_t pram_llseek(struct file *file, loff_t offset, int origin)
{
	struct inode *inode = file->f_mapping->host;
	int retval;

	if (origin != SEEK_DATA && origin != SEEK_HOLE)
		return generic_file_llseek(file, offset, origin);

	mutex_lock(&inode->i_mutex);
	switch (origin) {
	case SEEK_DATA:
		retval = pram_find_region(inode, &offset, 0);
		if (retval) {
			mutex_unlock(&inode->i_mutex);
			return retval;
		}
		break;
	case SEEK_HOLE:
		retval = pram_find_region(inode, &offset, 1);
		if (retval) {
			mutex_unlock(&inode->i_mutex);
			return retval;
		}
		break;
	}

	offset = vfs_setpos(file, offset, inode->i_sb->s_maxbytes);

	mutex_unlock(&inode->i_mutex);
	return offset;
}

//
int pram_file_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
        int ret = 0;
        printk(KERN_DEBUG "pram_file_fault\n");
        //vma->vm_flags &= ~VM_SHARED;
        ret = filemap_fault(vma, vmf);
        //printk(KERN_DEBUG ";%x \n",ret);
        return ret;
}

int pram_mkwrite(struct vm_area_struct *vma, struct vm_fault *vmf)
{
        int ret;
        printk(KERN_DEBUG "pram_mkwrite\n");
        ret = filemap_page_mkwrite(vma,vmf);
        //printk(KERN_DEBUG ";%x \n",ret);
        return ret;
}

int pram_remap(struct vm_area_struct *vma, unsigned long addr,
                             unsigned long size, pgoff_t pgoff)
{
        int ret;
        printk(KERN_DEBUG "pram_remap\n");
        ret = generic_file_remap_pages(vma,addr,size,pgoff);
        //printk(KERN_DEBUG ";%x \n",ret);
        return ret;
}

static const struct vm_operations_struct pram_file_vm_ops = {
        .fault  = pram_file_fault,
        .page_mkwrite = pram_mkwrite,
        .remap_pages = pram_remap,
};

int pram_file_mmap(struct file * file, struct vm_area_struct * vma)
{
        struct address_space *mapping = file->f_mapping;
        printk(KERN_DEBUG "pram_file_mmap\n");

        if (!mapping->a_ops->readpage)
               return -ENOEXEC;
        file_accessed(file);
        vma->vm_ops = &pram_file_vm_ops;
        return 0;
}
//

const struct file_operations pram_file_operations = {
	.llseek		= pram_llseek,
	.read		= do_sync_read,
	.write		= do_sync_write,
	.aio_read	= generic_file_aio_read,
	.aio_write	= generic_file_aio_write,
//	.mmap		= generic_file_mmap,
	.mmap		= pram_file_mmap,
	.open		= pram_open_file,
	.fsync		= noop_fsync,
	.check_flags	= pram_check_flags,
	.unlocked_ioctl	= pram_ioctl,
	.splice_read	= generic_file_splice_read,
	.fallocate	= pram_fallocate,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= pram_compat_ioctl,
#endif
};

#ifdef CONFIG_PRAMFS_XIP
const struct file_operations pram_xip_file_operations = {
	.llseek		= pram_llseek,
	.read		= pram_xip_file_read,
	.write		= xip_file_write,
	.mmap		= pram_xip_file_mmap,
	.open		= generic_file_open,
	.fsync		= noop_fsync,
	.unlocked_ioctl	= pram_ioctl,
	.fallocate	= pram_fallocate,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= pram_compat_ioctl,
#endif
};
#endif

const struct inode_operations pram_file_inode_operations = {
#ifdef CONFIG_PRAMFS_XATTR
	.setxattr	= generic_setxattr,
	.getxattr	= generic_getxattr,
	.listxattr	= pram_listxattr,
	.removexattr	= generic_removexattr,
#endif
	.setattr	= pram_notify_change,
	.get_acl	= pram_get_acl,
};
