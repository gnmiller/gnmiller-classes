#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char** argv )
{
	if( argc != 4 )
	{
		fprintf( stdout, "Invalid arguments\nvig <plain/ciphertext> <key> -d/-e\n" );
		fprintf( stdout, "Arguments are expected to be all capital letters with no whitespace\n-d indicates decryption, -e encryption\n" );
		exit( EXIT_FAILURE );
	}
	if( strcmp( argv[3], "-e" ) == 0 )
	{
		fprintf( stdout, "Plaintext: %s\nKey: %s\nCiphertext numerical: ", argv[1], argv[2] );
		char buf[256];
		char out[256];
		memset( out, 0, 256 );
		for( int i = 0; i < strlen( argv[1] ); ++i )
		{
			char c = argv[1][i];
			char d = argv[2][i%strlen(argv[2])];
			int a = c - 65;
			int b = d - 65;
			int o = ((c + d) % 26) + 65;
			sprintf( buf, "%c", o );
			fprintf( stdout, "%d ", o-65 );
			strcat( out, buf );
		}
		fprintf( stdout, "\nCiphertext: %s\n", out );
		exit( EXIT_SUCCESS );
	}
	else if( strcmp( argv[3], "-d" ) == 0 )
	{
		fprintf( stdout, "Ciphertext: %s\nKey: %s\nPlaintext numerical: ", argv[1], argv[2] );
		char buf[256];
		char out[256];
		memset( out, 0, 256 );
		for( int i = 0; i < strlen( argv[1] ); ++i )
		{
			char c = argv[1][i];
			char d = argv[2][i%strlen(argv[2])];
			int a = c - 65;
			int b = d - 65;
			int o = ((c - d) % 26) + 65;
			sprintf( buf, "%c", o );
			fprintf( stdout, "%d ", o-65 );
			strcat( out, buf );
		}
		fprintf( stdout, "\nPlaintext: %s\n", out );
		exit( EXIT_SUCCESS );
	}
	else
	{
		exit( EXIT_FAILURE );
	}
}