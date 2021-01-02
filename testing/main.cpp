#include <Lua/Interface.hpp>
#include <stdio.h>

int main( int argc, char **argv )
{
	printf( "%i\n", Lua::InterfaceOffset );

	Lua::Interface lua;
	if( !lua.RunFile( argc < 2 ? "main.lua" : argv[1] ) )
	{
		printf( "%s\n", lua.ToString( -1 ) );
		return -1;
	}

	return 0;
}
