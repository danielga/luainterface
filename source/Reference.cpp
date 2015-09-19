#include <Reference.hpp>
#include <Lua/Interface.hpp>

namespace Lua
{

Reference::Reference( Interface *lua ) :
	lua_state( lua ),
	reference( lua->ReferenceCreate( ) )
{ }

Reference::~Reference( )
{
	if( lua_state != nullptr )
	{
		lua_state->ReferenceFree( reference.exchange( Lua::ReferenceInvalid ) );
		lua_state = nullptr;
	}
}

bool Reference::IsValid( ) const
{
	return lua_state != nullptr && reference != Lua::ReferenceInvalid;
}

int Reference::Get( ) const
{
	return reference;
}

}
