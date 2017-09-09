/*
 * BRIEF DESCRIPTION
 *
 * Memory protection definitions for the PRAMFS filesystem.
 *
 * Copyright 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __WPROTECT_H
#define __WPROTECT_H

#include <linux/pram_fs.h>

/* pram_memunlock_super() before calling! */
static inline void pram_sync_super(struct pram_super_block *ps)
{
	u32 crc = 0;
	ps->s_wtime = cpu_to_be32(get_seconds());
	ps->s_sum = 0;
	crc = crc32(~0, (__u8 *)ps + sizeof(__be32), PRAM_SB_SIZE -
								sizeof(__be32));
	ps->s_sum = cpu_to_be32(crc);
	/* Keep sync redundant super block */
	memcpy((void *)ps + PRAM_SB_SIZE, (void *)ps, PRAM_SB_SIZE);
}

/* pram_memunlock_inode() before calling! */
static inline void pram_sync_inode(struct pram_inode *pi)
{
	u32 crc = 0;
	pi->i_sum = 0;
	crc = crc32(~0, (__u8 *)pi + sizeof(__be32), PRAM_INODE_SIZE -
								sizeof(__be32));
	pi->i_sum = cpu_to_be32(crc);
}

#ifdef CONFIG_PRAMFS_WRITE_PROTECT
extern void pram_writeable(void *vaddr, unsigned long size, int rw);

static inline int pram_is_protected(struct super_block *sb)
{
	struct pram_sb_info *sbi = (struct pram_sb_info *)sb->s_fs_info;
	return sbi->s_mount_opt & PRAM_MOUNT_PROTECT;
}

static inline void __pram_memunlock_range(void *p, unsigned long len)
{
	pram_writeable(p, len, 1);
}

static inline void __pram_memlock_range(void *p, unsigned long len)
{
	pram_writeable(p, len, 0);
}

static inline void pram_memunlock_range(struct super_block *sb, void *p,
					unsigned long len)
{
	if (pram_is_protected(sb))
		__pram_memunlock_range(p, len);
}

static inline void pram_memlock_range(struct super_block *sb, void *p,
					unsigned long len)
{
	if (pram_is_protected(sb))
		__pram_memlock_range(p, len);
}

static inline void pram_memunlock_super(struct super_block *sb,
					struct pram_super_block *ps)
{
	if (pram_is_protected(sb))
		__pram_memunlock_range(ps, PRAM_SB_SIZE);
}

static inline void pram_memlock_super(struct super_block *sb,
					struct pram_super_block *ps)
{
	pram_sync_super(ps);
	if (pram_is_protected(sb))
		__pram_memlock_range(ps, PRAM_SB_SIZE);
}

static inline void pram_memunlock_inode(struct super_block *sb,
					struct pram_inode *pi)
{
	if (pram_is_protected(sb))
		__pram_memunlock_range(pi, PRAM_SB_SIZE);
}

static inline void pram_memlock_inode(struct super_block *sb,
					struct pram_inode *pi)
{
	pram_sync_inode(pi);
	if (pram_is_protected(sb))
		__pram_memlock_range(pi, PRAM_SB_SIZE);
}

static inline void pram_memunlock_block(struct super_block *sb,
					void *bp)
{
	if (pram_is_protected(sb))
		__pram_memunlock_range(bp, sb->s_blocksize);
}

static inline void pram_memlock_block(struct super_block *sb,
					void *bp)
{
	if (pram_is_protected(sb))
		__pram_memlock_range(bp, sb->s_blocksize);
}

#else
#define pram_is_protected(sb)	0
#define pram_writeable(vaddr, size, rw) do {} while (0)
static inline void pram_memunlock_range(struct super_block *sb, void *p,
					unsigned long len) {}
static inline void pram_memlock_range(struct super_block *sb, void *p,
					unsigned long len) {}
static inline void pram_memunlock_super(struct super_block *sb,
					struct pram_super_block *ps) {}
static inline void pram_memlock_super(struct super_block *sb,
					struct pram_super_block *ps)
{
	pram_sync_super(ps);
}
static inline void pram_memunlock_inode(struct super_block *sb,
					struct pram_inode *pi) {}
static inline void pram_memlock_inode(struct super_block *sb,
					struct pram_inode *pi)
{
	pram_sync_inode(pi);
}
static inline void pram_memunlock_block(struct super_block *sb,
					void *bp) {}
static inline void pram_memlock_block(struct super_block *sb,
					void *bp) {}

#endif /* CONFIG PRAMFS_WRITE_PROTECT */
#endif
