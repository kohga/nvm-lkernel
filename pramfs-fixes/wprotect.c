/*
 * BRIEF DESCRIPTION
 *
 * Write protection for the filesystem pages.
 *
 * Copyright 2009-2011 Marco Stornelli <marco.stornelli@gmail.com>
 * Copyright 2003 Sony Corporation
 * Copyright 2003 Matsushita Electric Industrial Co., Ltd.
 * 2003-2004 (c) MontaVista Software, Inc. , Steve Longerbeam
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/io.h>
#include "pram.h"

void pram_writeable(void *vaddr, unsigned long size, int rw)
{
	int ret = 0;
	unsigned long nrpages = size >> PAGE_SHIFT;
	unsigned long addr = (unsigned long)vaddr;

	/* Page aligned */
	addr &= PAGE_MASK;

	if (size & (PAGE_SIZE - 1))
		nrpages++;

	if (rw)
		ret = set_memory_rw(addr, nrpages);
	else
		ret = set_memory_ro(addr, nrpages);

	BUG_ON(ret);
}
