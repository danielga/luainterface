#include <LuaInterface.hpp>
#include <stdio.h>
#include <string.h>

#if _WIN32
#define snprintf _snprintf
#endif

int main( int argc, char **argv )
{
	LuaInterface lua;
	if( !lua.RunFile( argc < 2 ? "main.lua" : argv[1] ) )
	{
		size_t len = 0;
		const char *luaerror = lua.ToString( -1, &len );
		char error[1024];
		snprintf( error, sizeof( error ), "%s\n", luaerror );
		error[sizeof( error ) - 1] = 0;
		printf( "%s\n", error );
		FILE *file = fopen( "errors.txt", "a" );
		if( file != 0 )
		{
			fwrite( error, 1, strlen( error ), file );
			fclose( file );
		}

		return -1;
	}

	return 0;
}

