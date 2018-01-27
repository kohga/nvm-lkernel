#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include "pram.h"


/*
 * Since it does not cache, it is not necessary to have a memory barrier
 */

//#define mb() asm volatile ("mfence":::"memory")


unsigned long cur_j_num = 0;

// need spi_lock();

int pjournal_srecord_sync(struct inode *inode){
	struct pj_srecord pjs;
	struct pj_footer pj_f;
	pram_info("pj_srecord_sync; i_ino = %lu\n", inode->i_ino);

	cur_j_num += 1;
	pjs.j_num = cur_j_num;
	pjs.status = 1;
	memcpy(&pjs.inode, inode, sizeof(struct inode));
	pj_f.type = 1;

	memcpy(pj_super.cur_addr, &pjs, sizeof(struct pj_srecord));
	pj_super.cur_addr += sizeof(struct pj_srecord);

	memcpy(pj_super.cur_addr, &pj_f, sizeof(struct pj_footer));
	pj_super.cur_addr += sizeof(struct pj_footer);

	//mb();
	memcpy(pj_super.start_addr, &pj_super, sizeof(struct pj_super));
	//clflush_cache_range(pj_super.start_addr, sizeof(struct pj_super));
	//mb();

	return 0;
}

int pjournal_srecord_commit(struct inode *inode){
	struct pj_srecord pjs;
	struct pj_footer pj_f;
	pram_info("pj_srecord_commit; i_ino = %lu\n", inode->i_ino);

	cur_j_num += 1;
	pjs.j_num = cur_j_num;
	pjs.status = 2;
	memcpy(&pjs.inode, inode, sizeof(struct inode));
	pj_f.type = 1;

	memcpy(pj_super.cur_addr, &pjs, sizeof(struct pj_srecord));
	pj_super.cur_addr += sizeof(struct pj_srecord);

	memcpy(pj_super.cur_addr, &pj_f, sizeof(struct pj_footer));
	pj_super.cur_addr += sizeof(struct pj_footer);

	//mb();
	memcpy(pj_super.start_addr, &pj_super, sizeof(struct pj_super));
	//clflush_cache_range(pj_super.start_addr, sizeof(struct pj_super));
	//mb();

	return 0;
}

int pjournal_crecord(struct inode *inode, pgoff_t aside_pgoff, pgoff_t bside_pgoff , void *aside_mem, void *bside_mem){
	struct pj_crecord pjc;
	struct pj_footer pj_f;
	pram_info("pj_crecord; i_ino = %lu\n", inode->i_ino);

	cur_j_num += 1;
	pjc.j_num = cur_j_num;
	pjc.aside_pgoff = aside_pgoff;
	pjc.bside_pgoff = bside_pgoff;
	pjc.aside_mem = aside_mem;
	pjc.bside_mem = bside_mem;
	memcpy(&pjc.inode, inode, sizeof(struct inode));
	pj_f.type = 2;

	memcpy(pj_super.cur_addr, &pjc, sizeof(struct pj_crecord));
	pj_super.cur_addr += sizeof(struct pj_crecord);

	memcpy(pj_super.cur_addr, &pj_f, sizeof(struct pj_footer));
	pj_super.cur_addr += sizeof(struct pj_footer);

	//mb();
	memcpy(pj_super.start_addr, &pj_super, sizeof(struct pj_super));
	//clflush_cache_range(pj_super.start_addr, sizeof(struct pj_super));
	//mb();

	return 0;
}
