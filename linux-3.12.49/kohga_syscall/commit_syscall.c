#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/pram_fs.h>

SYSCALL_DEFINE2( commit_syscall, char __user *, buf, int, count )
{
	long err;
	char text[ ] = "commit syscall!";

	printk( "<COMMIT_SYSCALL>%s\n", text );
	/*
	if (pram_flags & PRAM_COMMIT ){
		printk( "<COMMIT_SYSCALL:typeA>Break COMMIT\n");
		pram_flags &= ~PRAM_COMMIT;
		//pram_flags &= ~PRAM_COW;
		//pram_flags |= PRAM_INIT;
	} else {
		printk( "<COMMIT_SYSCALL:typeB>Set COMMIT&COW\n");
		//printk( "<COMMIT_SYSCALL:typeB>Break INIT&NEW&OLD\n");
		pram_flags |= PRAM_COMMIT;
		pram_flags |= PRAM_COW;
		//pram_flags &= ~PRAM_INIT;
		//pram_flags &= ~PRAM_NEW;
		//pram_flags &= ~PRAM_OLD;
	}
	*/

	if( count < sizeof( text ) )
	{
		return( -ENOMEM );
	}

	/* copy untill null terminator */
	err = copy_to_user( buf, text, sizeof( text ) );

	return( err );
}
