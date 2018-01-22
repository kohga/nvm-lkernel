#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/pram_fs.h>
#include <linux/fs.h>
#include <linux/mm.h>


SYSCALL_DEFINE2( nvmsync, char __user *, buf, unsigned long, fd)
{
	long err;
	char text[ ] = "NVMSYNC";
	struct inode *inode = NULL;

	printk( "< %s >\n", text );

	inode = get_pram_inode(fd);

	printk(KERN_DEBUG "*** inode->i_ino = %lu\n" ,inode->i_ino);
	printk(KERN_DEBUG "*** inode->inode_pram_flags = %lu\n" ,inode->inode_pram_flags);

	pjournal_srecord_sync(inode);
	if( inode->inode_pram_flags & PRAM_INODE_NONE ){
		printk( "ERROR; inode_pram_flags is none.\n");
	}else if(!(inode->inode_pram_flags & PRAM_INODE_SYNC)){
		printk( "A side -> B side\n");
		//inode->inode_pram_flags |= PRAM_INODE_SYNC;
		inode->inode_pram_flags = PRAM_INODE_SYNC;
	}else{
		printk( "B side -> A side\n");
		//inode->inode_pram_flags |= ~PRAM_INODE_SYNC;
		inode->inode_pram_flags = 0;
	}
	pjournal_srecord_commit(inode);

	/* copy untill null terminator */
	err = copy_to_user( buf, text, sizeof( text ) );

	return( err );
}
