/*
 * BRIEF DESCRIPTION
 *
 * Extended attributes for the pram filesystem.
 *
 * Copyright 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
 *
 * based on fs/ext2/xattr.h with the following copyright:
 *
 *(C) 2001 Andreas Gruenbacher, <a.gruenbacher@computer.org>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/init.h>
#include <linux/xattr.h>

#ifdef CONFIG_PRAMFS_XATTR

extern const struct xattr_handler pram_xattr_user_handler;
extern const struct xattr_handler pram_xattr_trusted_handler;
extern const struct xattr_handler pram_xattr_acl_access_handler;
extern const struct xattr_handler pram_xattr_acl_default_handler;
extern const struct xattr_handler pram_xattr_security_handler;

extern ssize_t pram_listxattr(struct dentry *, char *, size_t);

extern int pram_xattr_get(struct inode *, int, const char *, void *, size_t);
extern int pram_xattr_set(struct inode *, int, const char *, const void *,
			  size_t, int);

extern void pram_xattr_delete_inode(struct inode *);
extern void pram_xattr_put_super(struct super_block *);

extern int init_pram_xattr(void) __init;
extern void exit_pram_xattr(void);

extern const struct xattr_handler *pram_xattr_handlers[];

# else  /* CONFIG_PRAMFS_XATTR */

static inline int
pram_xattr_get(struct inode *inode, int name_index,
	       const char *name, void *buffer, size_t size)
{
	return -EOPNOTSUPP;
}

static inline int
pram_xattr_set(struct inode *inode, int name_index, const char *name,
	       const void *value, size_t size, int flags)
{
	return -EOPNOTSUPP;
}

static inline void
pram_xattr_delete_inode(struct inode *inode)
{
}

static inline void
pram_xattr_put_super(struct super_block *sb)
{
}

static inline int
init_pram_xattr(void)
{
	return 0;
}

static inline void
exit_pram_xattr(void)
{
}

#define pram_xattr_handlers NULL

# endif  /* CONFIG_PRAMFS_XATTR */

#ifdef CONFIG_PRAMFS_SECURITY
extern int pram_init_security(struct inode *inode, struct inode *dir,
				const struct qstr *qstr);
#else
static inline int pram_init_security(struct inode *inode, struct inode *dir,
					const struct qstr *qstr)
{
	return 0;
}
#endif
