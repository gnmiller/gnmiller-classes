#include <string.h>
#include <unistd.h>
int main()
{
	char* filename = malloc( 128 );
	strcpy( filename, "./file");
	char *tmp = malloc( strlen( filename ) );
	strcpy( tmp, filename );
	dirname( tmp );
	printf( "%s\n%s\n%d\n", filename, tmp, access( tmp, R_OK|W_OK ) );
}