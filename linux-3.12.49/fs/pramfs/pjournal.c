#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include "pram.h"

int pjournal_srecord_sync(struct inode *inode){
	pram_info("pj_srecord_sync; i_ino = %lu\n", inode->i_ino);
	return 0;
}

int pjournal_srecord_commit(struct inode *inode){
	pram_info("pj_srecord_commit; i_ino = %lu\n", inode->i_ino);
	return 0;
}

int pjournal_crecord(struct inode *inode, pgoff_t pgoff) {
	pram_info("pj_crecord; i_ino = %lu\n", inode->i_ino);
	return 0;
}
