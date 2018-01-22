/*
 * BRIEF DESCRIPTION
 *
 * Definitions for the PRAM filesystem.
 *
 * Copyright 2009-2011 Marco Stornelli <marco.stornelli@gmail.com>
 * Copyright 2003 Sony Corporation
 * Copyright 2003 Matsushita Electric Industrial Co., Ltd.
 * 2003-2004 (c) MontaVista Software, Inc. , Steve Longerbeam
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _LINUX_PRAM_FS_H
#define _LINUX_PRAM_FS_H

#include <uapi/linux/pram_fs.h>

/* kohga add for filemap_xip */
extern int pram_get_xip_mem(struct address_space *mapping, pgoff_t pgoff, int create,
					void **kmem, unsigned long *pfn);
extern int pram_xip_file_fault(struct vm_area_struct *vma, struct vm_fault *vmf);
extern unsigned long pp_offset;
extern struct pram_page *pp_address;
extern int pram_pgoff;
extern int pjournal_srecord_sync(struct inode *inode);
extern int pjournal_srecord_commit(struct inode *inode);
extern int pjournal_crecord(struct inode *inode, pgoff_t pgoff);


// kohga add (mm/mmap.c -> intarnal.h)
extern struct inode *get_pram_inode(unsigned long fd);

#define PRAM_PAGE_SIZE 4096

/* inode flags (kohga add) */
#define PRAM_INODE_NONE 0x0001
#define PRAM_INODE_SYNC 0x0002

/* pram page flags (kohga add) */
#define PRAM_PAGE_NONE 0x01
#define PRAM_PAGE_COPY 0x02

struct pram_page{
	char flags;
};

/*
 * PRAM filesystem super-block data in memory
 */
struct pram_sb_info {
	/*
	 * base physical and virtual address of PRAMFS (which is also
	 * the pointer to the super block)
	 */
	phys_addr_t phys_addr;
	void *virt_addr;

	/* Mount options */
	unsigned long bpi;
	unsigned long num_inodes;
	unsigned long blocksize;
	unsigned long initsize;
	unsigned long s_mount_opt;
	unsigned long s_mount_state;
	kuid_t uid;		    /* Mount uid for root directory */
	kgid_t gid;		    /* Mount gid for root directory */
	umode_t mode;		    /* Mount mode for root directory */
	atomic_t next_generation;
#ifdef CONFIG_PRAMFS_XATTR
	struct rb_root desc_tree;
	spinlock_t desc_tree_lock;
#endif
	struct mutex s_lock;

	/* Journaling */
	phys_addr_t j_phys_addr;
	unsigned long j_size;

	void *j_virt_addr;
	struct inode * s_journal_inode;
	struct journal_s * s_journal;
	struct list_head s_orphan;
	struct mutex s_orphan_lock;
	struct mutex s_resize_lock;
	unsigned long s_commit_interval;
	struct block_device *journal_bdev;
#ifdef CONFIG_QUOTA
	char *s_qf_names[MAXQUOTAS];		/* Names of quota files with journalled quota */
	int s_jquota_fmt;			/* Format of quota to use */
#endif

};
#endif	/* _LINUX_PRAM_FS_H */
