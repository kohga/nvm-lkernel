#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE2( kohga_syscall, char __user *, buf, int, count )
{
	long err;
	char text[ ] = "Kohga hacked!";

	printk( "<KOHGA SYSCALL>%s\n", text );

	if( count < sizeof( text ) )
	{
		return( -ENOMEM );
	}

	/* copy untill null terminator */
	err = copy_to_user( buf, text, sizeof( text ) );

	return( err );
}
