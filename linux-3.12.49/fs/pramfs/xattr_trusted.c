/*
 * BRIEF DESCRIPTION
 *
 * Handler for trusted extended attributes.
 *
 * Copyright 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
 *
 * based on fs/ext2/xattr_trusted.c with the following copyright:
 *
 * Copyright (C) 2003 by Andreas Gruenbacher, <a.gruenbacher@computer.org>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/string.h>
#include <linux/capability.h>
#include <linux/fs.h>
#include <linux/pram_fs.h>
#include "xattr.h"

static size_t pram_xattr_trusted_list(struct dentry *dentry, char *list,
				size_t list_size, const char *name,
				size_t name_len, int type)
{
	const int prefix_len = XATTR_TRUSTED_PREFIX_LEN;
	const size_t total_len = prefix_len + name_len + 1;

	if (!capable(CAP_SYS_ADMIN))
		return 0;

	if (list && total_len <= list_size) {
		memcpy(list, XATTR_TRUSTED_PREFIX, prefix_len);
		memcpy(list+prefix_len, name, name_len);
		list[prefix_len + name_len] = '\0';
	}
	return total_len;
}

static int pram_xattr_trusted_get(struct dentry *dentry, const char *name,
				void *buffer, size_t size, int type)
{
	if (strcmp(name, "") == 0)
		return -EINVAL;
	return pram_xattr_get(dentry->d_inode, PRAM_XATTR_INDEX_TRUSTED, name,
			      buffer, size);
}

static int pram_xattr_trusted_set(struct dentry *dentry, const char *name,
		const void *value, size_t size, int flags, int type)
{
	if (strcmp(name, "") == 0)
		return -EINVAL;
	return pram_xattr_set(dentry->d_inode, PRAM_XATTR_INDEX_TRUSTED, name,
			      value, size, flags);
}

const struct xattr_handler pram_xattr_trusted_handler = {
	.prefix	= XATTR_TRUSTED_PREFIX,
	.list	= pram_xattr_trusted_list,
	.get	= pram_xattr_trusted_get,
	.set	= pram_xattr_trusted_set,
};
