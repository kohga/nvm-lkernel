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
#include <linux/slab.h>
#include <linux/buffer_head.h>
#include "pram.h"
#include "xip.h"


/* use it now ?  */
unsigned long pram_xip_process_status;
unsigned long pram_xip_process_count;


/*
 * Wrappers. We need to use the rcu read lock to avoid
 * concurrent truncate operation. No problem for write because we held
 * i_mutex.
 */
ssize_t pram_xip_file_read(struct file *filp, char __user *buf,
					size_t len, loff_t *ppos)
{
	pram_info("xip.c / (strct file_operations)pram_xip_file_operations..read = pram_xip_file_read\n");
	ssize_t res;
	rcu_read_lock();
	res = xip_file_read(filp, buf, len, ppos);
	rcu_read_unlock();
	return res;
}

//static int pram_xip_file_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
int pram_xip_file_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	pram_info("xip.c / (strct vm_operations_struct)pram_xip_vm_ops..fault = pram_xip_file_fault\n");
	int ret = 0;
	rcu_read_lock();
	ret = xip_file_fault(vma, vmf);
	rcu_read_unlock();
	return ret;
}

int pram_xip_filemap_page_mkwrite(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	pram_info("xip.c / (strct vm_operations_struct)pram_xip_vm_ops..page_mkwrite = pram_xip_filemap_page_mkwrite\n");
	int ret;
	ret = filemap_page_mkwrite(vma,vmf);
	return ret;
}

int pram_xip_file_remap_pages(struct vm_area_struct *vma, unsigned long addr,
				unsigned long size, pgoff_t pgoff)
{
	pram_info("xip.c / (strct vm_operations_struct)pram_xip_vm_ops..remap_pages = pram_xip_file_remap_pages\n");
	int ret;
	ret = generic_file_remap_pages(vma, addr, size, pgoff);
	return ret;
}

static const struct vm_operations_struct pram_xip_vm_ops = {
	.fault = pram_xip_file_fault,
	.page_mkwrite = pram_xip_filemap_page_mkwrite,
	.remap_pages = pram_xip_file_remap_pages,
};

int pram_xip_file_mmap(struct file * file, struct vm_area_struct * vma)
{
	pram_info("xip.c / pram_xip_file_mmap\n");
	BUG_ON(!file->f_mapping->a_ops->get_xip_mem);

	file_accessed(file);
	vma->vm_ops = &pram_xip_vm_ops;
	vma->vm_flags |= VM_MIXEDMAP;
	return 0;
}

//static int pram_find_and_alloc_blocks(struct inode *inode, sector_t iblock,
int pram_find_and_alloc_blocks(struct inode *inode, sector_t iblock,
				     sector_t *data_block, int create)
{
	pram_info("xip.c / pram_find_and_alloc_blocks\n");
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

//static inline int __pram_get_block(struct inode *inode, pgoff_t pgoff,
inline int __pram_get_block(struct inode *inode, pgoff_t pgoff,
				   int create, sector_t *result)
{
	pram_info("xip.c / __pram_get_block\n");
	int rc = 0;

	rc = pram_find_and_alloc_blocks(inode, (sector_t)pgoff, result, create);

	if (rc == -ENODATA)
		BUG_ON(create);

	return rc;
}






int pram_find_and_alloc_blocks_atomic(struct inode *inode, sector_t iblock,
				     sector_t *data_block, int create)
{
	pram_info("xip.c / pram_find_and_alloc_blocks_atomic\n");
	int err = -EIO;
	u64 block;

	if( inode->inode_pram_flags & PRAM_ATOMIC ){
		pram_info("PRAM_ATOMIC:2\n");

		/* NEW  */
		err = pram_alloc_blocks(inode, iblock, 1);
		if (err){
			pram_info("PRAM_ATOMIC:2: ERR1\n");
			goto err;
		}

		block = pram_find_data_block(inode, iblock);
		if (!block) {
			pram_info("PRAM_ATOMIC:2: ERR2\n");
			err = -ENODATA;
			goto err;
		}
	}

	*data_block = block;
	err = 0;

 err:
	return err;
}

inline int __pram_get_block_atomic(struct inode *inode, pgoff_t pgoff,
				   int create, sector_t *result)
{
	pram_info("xip.c / __pram_get_block_atomic\n");
	int rc = 0;

	rc = pram_find_and_alloc_blocks_atomic(inode, (sector_t)pgoff, result, create);

	if (rc == -ENODATA)
		BUG_ON(create);

	return rc;
}


int pram_get_xip_mem(struct address_space *mapping, pgoff_t pgoff, int create,
		     void **kmem, unsigned long *pfn)
{
	pram_info("xip.c / pram_get_xip_mem\n");
	int rc;
	sector_t block = 0;
	pram_info("mapping->host = %x\n", mapping->host);
	pram_info("pgoff = %x\n", pgoff);
	pram_info("create = %d\n", create);
	//pram_info("**kmem = %x\n", **kmem);
	pram_info("*pfn = %lu\n", *pfn);

	/* first, retrieve the block */
	rc = __pram_get_block(mapping->host, pgoff, create, &block);
	if (rc){
		pram_info("goto exit;\n");
		goto exit;
	}

	// kohga add
	pram_info("mapping->host->inode_pram_flags = %lu\n", mapping->host->inode_pram_flags);
	if( mapping->host->inode_pram_flags & PRAM_ATOMIC ){
		if(pgoff == 0){
			pram_info("create\n");
			pad_p->i_cnt++;
			pram_info("create2\n");
			pad_p->start =(struct pram_atomic_inode *)kmalloc(sizeof(struct pram_atomic_inode), GFP_HIGHUSER);
			pram_info("create3\n");
			pad_p->now =(struct pram_atomic_inode *)kmalloc(sizeof(struct pram_atomic_inode), GFP_HIGHUSER);
			pram_info("create4\n");
			pad_p->end =(struct pram_atomic_inode *)kmalloc(sizeof(struct pram_atomic_inode), GFP_HIGHUSER);
			pram_info("create5\n");
			pad_p->now->now =(struct pram_atomic_block *)kmalloc(sizeof(struct pram_atomic_block), GFP_HIGHUSER);
		}

		sector_t old_block = block;
		pgoff_t old_pgoff = pgoff;
		void **old_kmem;
		struct super_block *sb = mapping->host->i_sb;
		pgoff += 1000;
		pram_info("PRAM_ATOMIC:1\n");
		//*kmem = pram_get_block(mapping->host->i_sb, block);
		pram_info("PRAM_ATOMIC:1-1\n");

		rc = __pram_get_block_atomic(mapping->host, pgoff, create, &block);
		if (rc){
			pram_info("goto exit;\n");
			goto exit;
		}

		pram_info("old pgoff = %x\n", old_pgoff);
		pram_info("new pgoff = %x\n", pgoff);
		pram_info("old block = %x\n", old_block);
		pram_info("new block = %x\n", block);

		*kmem = pram_get_block(mapping->host->i_sb, block);

		
		pram_info("memcpy; before\n");
		if( mapping->host->inode_pram_flags & PRAM_COMMIT ){
			pram_info("memcpy; PRAM_COMMIT\n");
			memcpy(*kmem, pram_get_block(mapping->host->i_sb, old_block), sb->s_blocksize);
			mapping->host->inode_pram_flags &= ~PRAM_COMMIT;
			//pram_free_block(mapping->host->i_sb,old_pgoff);
			//pram_info("free!!\n");
			//mapping->host->inode_pram_flags &= ~PRAM_ATOMIC;
		}
		pram_info("memcpy; after\n");
		

		*pfn =  pram_get_pfn(mapping->host->i_sb, block);

	}else{
		*kmem = pram_get_block(mapping->host->i_sb, block);
		*pfn =  pram_get_pfn(mapping->host->i_sb, block);
	}

	//mapping->host->inode_pram_flags &= ~PRAM_ATOMIC;

	pram_info("NOW block = %x\n", block);

	pram_info("after: *kmem = %lu\n", *kmem);
	pram_info("after: *pfn = %lu\n", *pfn);

exit:
	return rc;
}
