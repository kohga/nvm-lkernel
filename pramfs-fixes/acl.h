/*
 * BRIEF DESCRIPTION
 *
 * POSIX ACL operations
 *
 * Copyright 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 *
 * Based on fs/ext2/acl.h with the following copyright:
 *
 * Copyright (C) 2001-2003 Andreas Gruenbacher, <agruen@suse.de>
 *
 */

#include <linux/posix_acl_xattr.h>

#define PRAM_ACL_VERSION	0x0001

struct pram_acl_entry {
	__be16		e_tag;
	__be16		e_perm;
	__be32		e_id;
};

struct pram_acl_entry_short {
	__be16		e_tag;
	__be16		e_perm;
};

struct pram_acl_header {
	__be32		a_version;
};

static inline size_t pram_acl_size(int count)
{
	if (count <= 4) {
		return sizeof(struct pram_acl_header) +
		       count * sizeof(struct pram_acl_entry_short);
	} else {
		return sizeof(struct pram_acl_header) +
		       4 * sizeof(struct pram_acl_entry_short) +
		       (count - 4) * sizeof(struct pram_acl_entry);
	}
}

static inline int pram_acl_count(size_t size)
{
	ssize_t s;
	size -= sizeof(struct pram_acl_header);
	s = size - 4 * sizeof(struct pram_acl_entry_short);
	if (s < 0) {
		if (size % sizeof(struct pram_acl_entry_short))
			return -1;
		return size / sizeof(struct pram_acl_entry_short);
	} else {
		if (s % sizeof(struct pram_acl_entry))
			return -1;
		return s / sizeof(struct pram_acl_entry) + 4;
	}
}

#ifdef CONFIG_PRAMFS_POSIX_ACL

/* acl.c */
extern struct posix_acl *pram_get_acl(struct inode *inode, int type);
extern int pram_acl_chmod(struct inode *);
extern int pram_init_acl(struct inode *, struct inode *);

#else
#include <linux/sched.h>
#define pram_get_acl	NULL
#define pram_set_acl	NULL

static inline int pram_acl_chmod(struct inode *inode)
{
	return 0;
}

static inline int pram_init_acl(struct inode *inode, struct inode *dir)
{
	return 0;
}
#endif
