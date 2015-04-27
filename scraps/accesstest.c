#include <unistd.h>
int main()
{
	int acc = access( "/someFile", W_OK );
	printf( "%d\n%d\n%d\n", acc, R_OK, W_OK );
}