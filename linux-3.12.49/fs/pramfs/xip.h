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

#ifdef CONFIG_PRAMFS_XIP
int pram_get_xip_mem(struct address_space *, pgoff_t, int, void **,
							      unsigned long *);
ssize_t pram_xip_file_read(struct file *filp, char __user *buf,
					size_t len, loff_t *ppos);
int pram_xip_file_mmap(struct file * file, struct vm_area_struct * vma);
static inline int pram_use_xip(struct super_block *sb)
{
	struct pram_sb_info *sbi = PRAM_SB(sb);
	return sbi->s_mount_opt & PRAM_MOUNT_XIP;
}
#define mapping_is_xip(map) (map->a_ops->get_xip_mem)

#else

#define mapping_is_xip(map)	0
#define pram_use_xip(sb)	0
#define pram_get_xip_mem	NULL
#define pram_xip_file_read	NULL
#define pram_xip_file_mmap	NULL

#endif
