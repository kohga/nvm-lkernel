/*
 * BRIEF DESCRIPTION
 *
 * Definitions for the PRAMFS filesystem.
 *
 * Copyright 2009-2011 Marco Stornelli <marco.stornelli@gmail.com>
 * Copyright 2003 Sony Corporation
 * Copyright 2003 Matsushita Electric Industrial Co., Ltd.
 * 2003-2004 (c) MontaVista Software, Inc. , Steve Longerbeam
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#ifndef _UAPI_LINUX_PRAM_FS_H
#define _UAPI_LINUX_PRAM_FS_H

#include <linux/types.h>
#include <linux/magic.h>
#include <linux/fs.h>

/*
 * The PRAM filesystem constants/structures
 */

/*
 * Mount flags
 */
#define PRAM_MOUNT_PROTECT		0x000001  /* Use memory protection */
#define PRAM_MOUNT_XATTR_USER		0x000002  /* Extended user attributes */
#define PRAM_MOUNT_POSIX_ACL		0x000004  /* POSIX ACL */
#define PRAM_MOUNT_XIP			0x000008  /* Execute in place */
#define PRAM_MOUNT_ERRORS_CONT		0x000010  /* Continue on errors */
#define PRAM_MOUNT_ERRORS_RO		0x000020  /* Remount fs ro on errors */
#define PRAM_MOUNT_ERRORS_PANIC		0x000040  /* Panic on errors */
#define PRAM_MOUNT_ERRORS_PANIC		0x000040  /* Panic on errors */

//kohga hacked
#define PRAM_ERRORS_KOHGA		0x000080  /* Kohga ops errors */
#define PRAM_MOUNT_ABORT		0x000100  /* Kohga ops errors */

/*
 * Pram inode flags
 *
 * PRAM_EOFBLOCKS_FL	There are blocks allocated beyond eof
 */
#define PRAM_EOFBLOCKS_FL	0x20000000
/* Flags that should be inherited by new inodes from their parent. */
#define PRAM_FL_INHERITED (FS_SECRM_FL | FS_UNRM_FL | FS_COMPR_FL |\
			   FS_SYNC_FL | FS_NODUMP_FL | FS_NOATIME_FL | \
			   FS_COMPRBLK_FL | FS_NOCOMP_FL | FS_JOURNAL_DATA_FL |\
			   FS_NOTAIL_FL | FS_DIRSYNC_FL)
/* Flags that are appropriate for regular files (all but dir-specific ones). */
#define PRAM_REG_FLMASK (~(FS_DIRSYNC_FL | FS_TOPDIR_FL))
/* Flags that are appropriate for non-directories/regular files. */
#define PRAM_OTHER_FLMASK (FS_NODUMP_FL | FS_NOATIME_FL)
#define PRAM_FL_USER_VISIBLE (FS_FL_USER_VISIBLE | PRAM_EOFBLOCKS_FL)

/*
 * Maximal count of links to a file
 */
#define PRAM_LINK_MAX		32000

#define PRAM_MIN_BLOCK_SIZE 512
#define PRAM_MAX_BLOCK_SIZE 4096
#define PRAM_DEF_BLOCK_SIZE 2048

#define PRAM_INODE_SIZE 128 /* must be power of two */
#define PRAM_INODE_BITS   7

/*
 * Structure of a directory entry in PRAMFS.
 * Offsets are to the inode that holds the referenced dentry.
 */
struct pram_dentry {
	__be64	d_next;     /* next dentry in this directory */
	__be64	d_prev;     /* previous dentry in this directory */
	__be64	d_parent;   /* parent directory */
	char	d_name[0];
};


/*
 * Structure of an inode in PRAMFS
 */
struct pram_inode {
	__be32	i_sum;          /* checksum of this inode */
	__be32	i_uid;		/* Owner Uid */
	__be32	i_gid;		/* Group Id */
	__be16	i_mode;		/* File mode */
	__be16	i_links_count;	/* Links count */
	__be32	i_blocks;	/* Blocks count */
	__be32	i_size;		/* Size of data in bytes */
	__be32	i_atime;	/* Access time */
	__be32	i_ctime;	/* Creation time */
	__be32	i_mtime;	/* Modification time */
	__be32	i_dtime;	/* Deletion Time */
	__be64	i_xattr;	/* Extended attribute block */
	__be32	i_generation;	/* File version (for NFS) */
	__be32	i_flags;	/* Inode flags */

	union {
		struct {
			/*
			 * ptr to row block of 2D block pointer array,
			 * file block #'s 0 to (blocksize/8)^2 - 1.
			 */
			__be64 row_block;
		} reg;   /* regular file or symlink inode */
		struct {
			__be64 head; /* first entry in this directory */
			__be64 tail; /* last entry in this directory */
		} dir;
		struct {
			__be32 rdev; /* major/minor # */
		} dev;   /* device inode */
	} i_type;

	struct pram_dentry i_d;
};

#define PRAM_NAME_LEN \
	(PRAM_INODE_SIZE - offsetof(struct pram_inode, i_d.d_name) - 1)


//#define PRAM_SB_SIZE 128 /* must be power of two */
#define PRAM_SB_SIZE 1024 /* must be power of two */

/*
 * Structure of the super block in PRAMFS
 */
struct pram_super_block {
	__be32	s_sum;          /* checksum of this sb, including padding */
	__be64	s_size;         /* total size of fs in bytes */
	__be32	s_blocksize;    /* blocksize in bytes */
	__be32	s_inodes_count;	/* total inodes count (used or free) */
	__be32	s_free_inodes_count;/* free inodes count */
	__be32	s_free_inode_hint;  /* start hint for locating free inodes */
	__be32	s_blocks_count;	/* total data blocks count (used or free) */
	__be32	s_free_blocks_count;/* free data blocks count */
	__be32	s_free_blocknr_hint;/* free data blocks count */
	__be64	s_bitmap_start; /* data block in-use bitmap location */
	__be32	s_bitmap_blocks;/* size of bitmap in number of blocks */
	__be32	s_mtime;	/* Mount time */
	__be32	s_wtime;	/* Write time */
	__be16	s_magic;	/* Magic signature */
	char	s_volume_name[16]; /* volume name */

	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
/*D0*/	__u8	s_journal_uuid[16];	/* uuid of journal superblock */
/*E0*/	__le32	s_journal_inum;		/* inode number of journal file */
	__le32	s_journal_dev;		/* device number of journal file */
	__le32	s_last_orphan;		/* start of list of inodes to delete */
	__le32	s_hash_seed[4];		/* HTREE hash seed */
	__u8	s_def_hash_version;	/* Default hash version to use */
	__u8	s_reserved_char_pad;
	__u16	s_reserved_word_pad;
	__le32	s_default_mount_opts;
	__le32	s_first_meta_bg;	/* First metablock block group */
	__le32	s_mkfs_time;		/* When the filesystem was created */
	__le32	s_jnl_blocks[17];	/* Backup of the journal inode */
	/* 64bit support valid if EXT4_FEATURE_COMPAT_64BIT */
/*150*/	__le32	s_blocks_count_hi;
	/* Blocks count */
	__le32	s_r_blocks_count_hi;	/* Reserved blocks count */
	__le32	s_free_blocks_count_hi;	/* Free blocks count */
	__le16	s_min_extra_isize;	/* All inodes have at least # bytes */
	__le16	s_want_extra_isize; 	/* New inodes should reserve # bytes */
	__le32	s_flags;		/* Miscellaneous flags */
	__le16  s_raid_stride;		/* RAID stride */
	__le16  s_mmp_interval;         /* # seconds to wait in MMP checking */
	__le64  s_mmp_block;            /* Block for multi-mount protection */
	__le32  s_raid_stripe_width;    /* blocks on all data disks (N*stride)*/
	__u8	s_log_groups_per_flex;  /* FLEX_BG group size */
	__u8	s_reserved_char_pad2;
	__le16  s_reserved_pad;
	__u32   s_reserved[162];        /* Padding to the end of the block */

};

/* The root inode follows immediately after the redundant super block */
#define PRAM_ROOT_INO (PRAM_SB_SIZE*2)

/*
 * XATTR related
 */

/* Magic value in attribute blocks */
#define PRAM_XATTR_MAGIC		0x6d617270

/* Maximum number of references to one attribute block */
#define PRAM_XATTR_REFCOUNT_MAX		1024

/* Name indexes */
#define PRAM_XATTR_INDEX_USER			1
#define PRAM_XATTR_INDEX_POSIX_ACL_ACCESS	2
#define PRAM_XATTR_INDEX_POSIX_ACL_DEFAULT	3
#define PRAM_XATTR_INDEX_TRUSTED		4
#define PRAM_XATTR_INDEX_SECURITY	        5

struct pram_xattr_header {
	__be32	h_magic;	/* magic number for identification */
	__be32	h_refcount;	/* reference count */
	__be32	h_hash;		/* hash value of all attributes */
	__u32	h_reserved[4];	/* zero right now */
};

struct pram_xattr_entry {
	__u8	e_name_len;	/* length of name */
	__u8	e_name_index;	/* attribute name index */
	__be16	e_value_offs;	/* offset in disk block of value */
	__be32	e_value_block;	/* disk block attribute is stored on (n/i) */
	__be32	e_value_size;	/* size of attribute value */
	__be32	e_hash;		/* hash value of name and value */
	char	e_name[0];	/* attribute name */
};

#define PRAM_XATTR_PAD_BITS		2
#define PRAM_XATTR_PAD		(1<<PRAM_XATTR_PAD_BITS)
#define PRAM_XATTR_ROUND		(PRAM_XATTR_PAD-1)
#define PRAM_XATTR_LEN(name_len) \
	(((name_len) + PRAM_XATTR_ROUND + \
	sizeof(struct pram_xattr_entry)) & ~PRAM_XATTR_ROUND)
#define PRAM_XATTR_NEXT(entry) \
	((struct pram_xattr_entry *)( \
	  (char *)(entry) + PRAM_XATTR_LEN((entry)->e_name_len)))
#define PRAM_XATTR_SIZE(size) \
	(((size) + PRAM_XATTR_ROUND) & ~PRAM_XATTR_ROUND)

#endif	/* _UAPI_LINUX_PRAM_FS_H */
