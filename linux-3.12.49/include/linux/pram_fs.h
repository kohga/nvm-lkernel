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

extern int pram_get_xip_mem(struct address_space *mapping, pgoff_t pgoff, int create,
					void **kmem, unsigned long *pfn);
extern int pram_xip_file_fault(struct vm_area_struct *vma, struct vm_fault *vmf);

extern struct pram_journal pram_j;

#define PRAM_COMMIT 0x01
#define PRAM_ATOMIC 0x02

#define PRAM_NONE 0x01
#define PRAM_ORIGIN 0x02
#define PRAM_SHADOW 0x04

extern unsigned long pp_offset;
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
	phys_addr_t jbd_phys_addr;
	unsigned long jbd_size;

	void *jbd_virt_addr;
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

extern struct pram_atomic_file *paf_start;
extern struct pram_atomic_file **paf_now;

struct pram_page{
	char flags;
};

struct pram_atomic_block{
	void *origin_mem;
	void *shadow_mem;
	unsigned long origin_pfn;
	unsigned long shadow_pfn;
};

struct pram_atomic_file{
	unsigned long i_ino;
	unsigned long div;
	struct pram_atomic_block pab[128];
	struct pram_atomic_file *next;
	int flags;
};

/*
struct pram_atomic_block{
	struct pram_atomic_block *b_next;
	sector_t origin_block;
	pgoff_t origin_pgoff;
	sector_t shadow_block;
	pgoff_t shadow_pgoff;
};
*/

struct pram_atomic_inode{
	struct pram_atomic_inode *i_next;
	int b_cnt;
	struct inode *i_address;
	struct pram_atomic_block *b_start;
};

struct pram_atomic_data{
	int i_cnt;
	struct pram_atomic_inode *i_start;
};

struct pram_journal{
	int status;  // 0:init, 1:start, 2:create, 4,match, 5:delete, 6:commit, 7:full commit
	char func[32];
	int line;
	struct pram_atomic_data pad;
	struct pram_sb_info *psi;
};

#endif	/* _LINUX_PRAM_FS_H */
