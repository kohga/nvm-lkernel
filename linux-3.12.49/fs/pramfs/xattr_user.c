/*
 * BRIEF DESCRIPTION
 *
 * Handler for extended user attributes.
 *
 * Copyright 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
 *
 * based on fs/ext2/xattr_user.c with the following copyright:
 *
 * Copyright (C) 2001 by Andreas Gruenbacher, <a.gruenbacher@computer.org>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include "pram.h"
#include "xattr.h"

static size_t pram_xattr_user_list(struct dentry *dentry, char *list,
				   size_t list_size, const char *name,
				   size_t name_len, int type)
{
	const size_t prefix_len = XATTR_USER_PREFIX_LEN;
	const size_t total_len = prefix_len + name_len + 1;

	if (!test_opt(dentry->d_sb, XATTR_USER))
		return 0;

	if (list && total_len <= list_size) {
		memcpy(list, XATTR_USER_PREFIX, prefix_len);
		memcpy(list+prefix_len, name, name_len);
		list[prefix_len + name_len] = '\0';
	}
	return total_len;
}

static int pram_xattr_user_get(struct dentry *dentry, const char *name,
		void *buffer, size_t size, int type)
{
	if (strcmp(name, "") == 0)
		return -EINVAL;
	if (!test_opt(dentry->d_sb, XATTR_USER))
		return -EOPNOTSUPP;
	return pram_xattr_get(dentry->d_inode, PRAM_XATTR_INDEX_USER,
			      name, buffer, size);
}

static int pram_xattr_user_set(struct dentry *dentry, const char *name,
		const void *value, size_t size, int flags, int type)
{
	if (strcmp(name, "") == 0)
		return -EINVAL;
	if (!test_opt(dentry->d_sb, XATTR_USER))
		return -EOPNOTSUPP;

	return pram_xattr_set(dentry->d_inode, PRAM_XATTR_INDEX_USER,
			      name, value, size, flags);
}

const struct xattr_handler pram_xattr_user_handler = {
	.prefix	= XATTR_USER_PREFIX,
	.list	= pram_xattr_user_list,
	.get	= pram_xattr_user_get,
	.set	= pram_xattr_user_set,
};
