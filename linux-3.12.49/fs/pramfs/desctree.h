/*
 * BRIEF DESCRIPTION
 *
 * Extended attributes block descriptors tree.
 *
 * Copyright 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/atomic.h>
#include <linux/slab.h>
#include "pram.h"

struct pram_xblock_desc {
#define FREEING (1UL << 1)
	unsigned long flags;	/* descriptor flags */
	atomic_t refcount;	/* users count of this descriptor */
	unsigned long blocknr;	/* absolute block number */
	struct mutex lock;	/* block lock */
	struct rb_node node;	/* node in the rb tree */
};

extern struct pram_xblock_desc *lookup_xblock_desc(struct pram_sb_info *sbi,
						   unsigned long blocknr,
						   struct kmem_cache *, int);
extern void insert_xblock_desc(struct pram_sb_info *sbi,
			       struct pram_xblock_desc *desc);
extern void mark_free_desc(struct pram_xblock_desc *desc);
extern int put_xblock_desc(struct pram_sb_info *sbi,
			   struct pram_xblock_desc *desc);
extern void xblock_desc_init_always(struct pram_xblock_desc *desc);
extern void xblock_desc_init_once(struct pram_xblock_desc *desc);
extern void erase_tree(struct pram_sb_info *sbi,
		       struct kmem_cache *);

static inline void xblock_desc_init(struct pram_xblock_desc *desc)
{
	xblock_desc_init_always(desc);
	xblock_desc_init_once(desc);
};

