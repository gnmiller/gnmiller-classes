int main()
{
	char *data;
	data = malloc( 32 );
	strcpy( data, "this.is.some.text" );
	
	int i,count;
	for( i = 0; i < strlen( data ); ++i )
	{
		++count;
		if( data[i] == '.' )
		{
			data[i] = count;
			count = 0;
		}
	}
	for( i = 0; i < strlen( data ); ++i )
	{
		write( STDOUT_FILENO, data[i], 1 );
	}
	//printf( "%s\n", data );
}