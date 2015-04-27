#include "syscall.h"

int a[4096];
int b[4096];
int c[4096];

int
main()
{
	int i;
	for( i = 0; i < 4096; ++i )
	{
		a[i] = 1;
		b[i] = 2;
		c[i] = 4;
	}
	int t,u,v;
	t = u = v = 0;
	for( i = 0; i < 4096; ++i )
	{
		t += a[i];
		u += b[i];
		v += c[i];
	}

	if( t == 4096 )
	{
		Write( "t = 4096\n", 9, ConsoleOutput );
	}
	else
	{
		Write( "t wrong val!!\n", 14, ConsoleOutput );
	}
	
	if( u == 8192 )
	{
		Write( "u = 8192\n", 9, ConsoleOutput );
	}
	else
	{
		Write( "u wrong val!!\n", 14, ConsoleOutput );
	}
	
	if( v == 16384 )
	{
		Write( "v = 16384\n", 9, ConsoleOutput );
	}
	else
	{
		Write( "v wrong val!!\n", 14, ConsoleOutput );
	}
    Exit( 0 );		/* and then we're done */
}
