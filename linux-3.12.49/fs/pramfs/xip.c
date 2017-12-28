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

int pram_find_and_alloc_blocks_atomic(struct inode *inode, sector_t iblock, sector_t *data_block)
{
	//pram_info("xip.c / pram_find_and_alloc_blocks_atomic\n");
	int err = -EIO;
	u64 block;

	if( inode->inode_pram_flags & PRAM_ATOMIC ){
		//pram_info("PRAM_ATOMIC:2\n");

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


inline int __pram_get_block_atomic(struct inode *inode, pgoff_t pgoff, sector_t *result)
{
	//pram_info("xip.c / __pram_get_block_atomic\n");
	int rc = 0;

	rc = pram_find_and_alloc_blocks_atomic(inode, (sector_t)pgoff, result);

	if (rc == -ENODATA)
		pram_info("ERROR: __pram_get_block_atomic\n");

	return rc;
}


struct pram_atomic_inode *pas_create_inode(struct inode *create_inode){
	pram_info("pas_create_inode\n");
	struct pram_atomic_inode *pai;

	pai = (struct pram_atomic_inode *)kmalloc(sizeof(struct pram_atomic_inode), GFP_HIGHUSER);
	pai->i_next = NULL;
	pai->b_cnt = 0;
	pai->i_address = create_inode;
	pai->b_start = NULL;
	pram_j.pad.i_cnt += 1;

	return pai;
}


struct pram_atomic_inode *pas_search_inode(struct inode *search_inode)
{
	pram_info("pas_search_inode\n");
	struct pram_atomic_inode *pai = pram_j.pad.i_start;

	while(pai != NULL){
		pram_info("while\n");
		if(pai->i_address == search_inode)
			return pai;

		pai = pai->i_next;
	}
	return pai;
}


struct pram_atomic_block *pas_create_block(struct inode *ino_p, pgoff_t pgoff, sector_t block)
{
	pram_info("pas_create_block\n");
	struct pram_atomic_block *pab;
	sector_t s_block;
	int offset = 1024;
	int rc = 0;

	pab = (struct pram_atomic_block *)kmalloc(sizeof(struct pram_atomic_block), GFP_HIGHUSER);
	pab->b_next = NULL;
	pab->origin_pgoff = pgoff;
	pab->origin_block = block;

	pab->shadow_pgoff = pgoff + offset;

	rc = __pram_get_block_atomic(ino_p, pab->shadow_pgoff, &(pab->shadow_block) );
	if (rc){
		pram_info("ERROR: pas_create_block\n");
	}

	pram_info("\n---check status---\n");
	pram_info("shadow_pgoff = %d\n", pab->shadow_pgoff);
	pram_info("shadow_block = %x\n", pab->shadow_block);

	return pab;
}

/*
sector_t _pram_atomic_system(struct inode *ino_p, pgoff_t pgoff, sector_t block)
{
	pram_info("pram_atomic_system\n");
	struct pram_atomic_inode *pai = pas_search_inode(ino_p);

	if(pai == NULL){
		pram_info("create inode\n");
		pai = pas_create_inode(ino_p);
		if(pram_j.pad.i_cnt == 1){
			pram_j.pad.i_start = pai;
		}
	}

	struct pram_atomic_block *bp = pai->b_start;
	while(bp != NULL){
		pram_info("while\n");
		if(bp->origin_pgoff == pgoff){
			pram_info("Match pgoff");
			return bp->shadow_block;
		}
		bp = bp->b_next;
	}

	bp = pas_create_block(ino_p, pgoff, block);
	pai->b_cnt += 1;

	if(pai->b_cnt == 1){
		pai->b_start = bp;
	}

	pram_info("memcpy before\n");
	memcpy(pram_get_block(pai->i_address->i_sb, bp->shadow_block), 
			pram_get_block(pai->i_address->i_sb, bp->origin_block), pai->i_address->i_sb->s_blocksize);
	pram_info("memcpy after\n");

	return bp->shadow_block;
}
*/

sector_t pram_atomic_system(struct inode *ino_p, pgoff_t pgoff, sector_t block)
{
	pram_info("pram_atomic_system\n");
	sector_t shadow_block;
	int offset = 1024;
	pgoff_t  shadow_pgoff = pgoff + offset;
	int rc = 0;

	rc = __pram_get_block_atomic(ino_p, shadow_pgoff, &shadow_block );
	if (rc){
		pram_info("ERROR: pas_create_block\n");
	}

	memcpy(pram_get_block(ino_p->i_sb, shadow_block), pram_get_block(ino_p->i_sb, block), ino_p->i_sb->s_blocksize);

	return shadow_block;
}

sector_t pas_commit_block(struct inode *ino_p, pgoff_t pgoff)
{
	sector_t block = 0;

	return block;
}


int pram_get_xip_mem(struct address_space *mapping, pgoff_t pgoff, int create,
		     void **kmem, unsigned long *pfn)
{
	pram_info("xip.c / pram_get_xip_mem\n");
	int rc;
	sector_t block = 0;
	pram_info("mapping->host = %x\n", mapping->host);
	pram_info("pgoff = %x\n", pgoff);

	/* first, retrieve the block */
	rc = __pram_get_block(mapping->host, pgoff, create, &block);
	if (rc){
		pram_info("goto exit;\n");
		goto exit;
	}


	// kohga add
	if( mapping->host->inode_pram_flags & PRAM_ATOMIC ){
		//pram_info("*** PRAM_ATOMIC ***\n");
		//sector_t shadow_block;
		//shadow_block= pram_atomic_system(mapping->host, pgoff, block);

		sector_t shadow_block;
		int offset = 1024;
		pgoff_t shadow_pgoff = pgoff + offset;
		int rc = 0;

		rc = __pram_get_block_atomic(mapping->host, shadow_pgoff, &shadow_block );
		if (rc){
			pram_info("ERROR: pas_create_block\n");
		}

		*kmem = pram_get_block(mapping->host->i_sb, shadow_block);
		memcpy(*kmem, pram_get_block(mapping->host->i_sb, block), mapping->host->i_sb->s_blocksize);
		//*kmem = pram_get_block(mapping->host->i_sb, shadow_block);
		*pfn =  pram_get_pfn(mapping->host->i_sb, shadow_block);
		block = shadow_block;

		}else if( mapping->host->inode_pram_flags & PRAM_COMMIT ){
		pram_info("*** PRAM_COMMIT ***\n");
		sector_t commit_block;

		commit_block = pas_commit_block(mapping->host, pgoff);

		*kmem = pram_get_block(mapping->host->i_sb, commit_block);
		*pfn =  pram_get_pfn(mapping->host->i_sb, commit_block);

		block = commit_block;

	}else{
		pram_info("*** ELSE ***\n");
		*kmem = pram_get_block(mapping->host->i_sb, block);
		*pfn =  pram_get_pfn(mapping->host->i_sb, block);
	}

	mapping->host->inode_pram_flags &= ~PRAM_ATOMIC;

	pram_info("\n---check status---\n");
	pram_info("block = %x\n", block);
	pram_info("*kmem = %lu\n", *kmem);
	pram_info("*pfn = %lu\n", *pfn);


exit:
	return rc;
}
