/*
 * BRIEF DESCRIPTION
 *
 * Handler for storing security labels as extended attributes.
 *
 * Copyright 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/pram_fs.h>
#include <linux/security.h>
#include "xattr.h"

static size_t pram_xattr_security_list(struct dentry *dentry, char *list,
				       size_t list_size, const char *name,
				       size_t name_len, int type)
{
	pram_info("xattr_security.c\n");
	const int prefix_len = XATTR_SECURITY_PREFIX_LEN;
	const size_t total_len = prefix_len + name_len + 1;

	if (list && total_len <= list_size) {
		memcpy(list, XATTR_SECURITY_PREFIX, prefix_len);
		memcpy(list+prefix_len, name, name_len);
		list[prefix_len + name_len] = '\0';
	}
	return total_len;
}

static int pram_xattr_security_get(struct dentry *dentry, const char *name,
		       void *buffer, size_t size, int type)
{
	pram_info("xattr_security.c\n");
	if (strcmp(name, "") == 0)
		return -EINVAL;
	return pram_xattr_get(dentry->d_inode, PRAM_XATTR_INDEX_SECURITY, name,
			      buffer, size);
}

static int pram_xattr_security_set(struct dentry *dentry, const char *name,
		const void *value, size_t size, int flags, int type)
{
	pram_info("xattr_security.c\n");
	if (strcmp(name, "") == 0)
		return -EINVAL;
	return pram_xattr_set(dentry->d_inode, PRAM_XATTR_INDEX_SECURITY, name,
			      value, size, flags);
}

int pram_initxattrs(struct inode *inode, const struct xattr *xattr_array,
		    void *fs_info)
{
	pram_info("xattr_security.c\n");
	const struct xattr *xattr;
	int err = 0;
	for (xattr = xattr_array; xattr->name != NULL; xattr++) {
		err = pram_xattr_set(inode, PRAM_XATTR_INDEX_SECURITY,
				     xattr->name, xattr->value,
				     xattr->value_len, 0);
		if (err < 0)
			break;
	}
	return err;
}

int pram_init_security(struct inode *inode, struct inode *dir,
		       const struct qstr *qstr)
{
	pram_info("xattr_security.c\n");
	return security_inode_init_security(inode, dir, qstr,
					    &pram_initxattrs, NULL);
}

const struct xattr_handler pram_xattr_security_handler = {
	.prefix	= XATTR_SECURITY_PREFIX,
	.list	= pram_xattr_security_list,
	.get	= pram_xattr_security_get,
	.set	= pram_xattr_security_set,
};
