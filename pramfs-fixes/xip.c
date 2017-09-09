/*
 * BRIEF DESCRIPTION
 *
 * XIP operations.
 *
 * Copyright 2009-2011 Marco Stornelli <marco.stornelli@gmail.com>
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include "pram.h"
#include "xip.h"

/*
 * Wrappers. We need to use the rcu read lock to avoid
 * concurrent truncate operation. No problem for write because we held
 * i_mutex.
 */
ssize_t pram_xip_file_read(struct file *filp, char __user *buf,
					size_t len, loff_t *ppos)
{
	ssize_t res;
	rcu_read_lock();
	res = xip_file_read(filp, buf, len, ppos);
	rcu_read_unlock();
	return res;
}

static int pram_xip_file_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	int ret = 0;
	rcu_read_lock();
	ret = xip_file_fault(vma, vmf);
	rcu_read_unlock();
	return ret;
}

static const struct vm_operations_struct pram_xip_vm_ops = {
	.fault	= pram_xip_file_fault,
	.page_mkwrite = filemap_page_mkwrite,
        .remap_pages = generic_file_remap_pages,
};

int pram_xip_file_mmap(struct file * file, struct vm_area_struct * vma)
{
	BUG_ON(!file->f_mapping->a_ops->get_xip_mem);

	file_accessed(file);
	vma->vm_ops = &pram_xip_vm_ops;
	vma->vm_flags |= VM_MIXEDMAP;
	return 0;
}

static int pram_find_and_alloc_blocks(struct inode *inode, sector_t iblock,
				     sector_t *data_block, int create)
{
	int err = -EIO;
	u64 block;

	block = pram_find_data_block(inode, iblock);

	if (!block) {
		if (!create) {
			err = -ENODATA;
			goto err;
		}

		err = pram_alloc_blocks(inode, iblock, 1);
		if (err)
			goto err;

		block = pram_find_data_block(inode, iblock);
		if (!block) {
			err = -ENODATA;
			goto err;
		}
	}

	*data_block = block;
	err = 0;

 err:
	return err;
}

static inline int __pram_get_block(struct inode *inode, pgoff_t pgoff,
				   int create, sector_t *result)
{
	int rc = 0;

	rc = pram_find_and_alloc_blocks(inode, (sector_t)pgoff, result, create);

	if (rc == -ENODATA)
		BUG_ON(create);

	return rc;
}

int pram_get_xip_mem(struct address_space *mapping, pgoff_t pgoff, int create,
		     void **kmem, unsigned long *pfn)
{
	int rc;
	sector_t block = 0;

	/* first, retrieve the block */
	rc = __pram_get_block(mapping->host, pgoff, create, &block);
	if (rc)
		goto exit;

	*kmem = pram_get_block(mapping->host->i_sb, block);
	*pfn =  pram_get_pfn(mapping->host->i_sb, block);

exit:
	return rc;
}
