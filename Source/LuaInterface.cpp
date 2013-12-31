extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
	#include "luajit.h"
}

#include "LuaInterface.hpp"
#include <string>

class lua_exception : public std::exception
{
public:
	lua_exception( ) throw( ) : custom_msg( "Lua exception" ) { }
	lua_exception( const char *str ) throw( ) : custom_msg( str ) { }
	lua_exception( const std::string &str ) throw( ) : custom_msg( str ) { }
	lua_exception( const lua_exception &rhs ) throw( ) : custom_msg( rhs.custom_msg ) { }
	lua_exception &operator=( const lua_exception &rhs ) throw( ) { custom_msg = rhs.custom_msg; }
	virtual ~lua_exception( ) throw( ) { }
	virtual const char *what( ) const throw( ) { return custom_msg.c_str( ); }

private:
	std::string custom_msg;
};

// On LuaJIT, use the C call wrapper feature http://luajit.org/ext_c_api.html#mode_wrapcfunc
static int SafeLuaFunction( lua_State *state, lua_CFunction func )
{
	try
	{
		return func( state );
	}
	catch( const char *s )
	{
		LUA->PushString( s );
	}
	catch( std::exception &e )
	{
		LUA->PushString( e.what( ) );
	}
	catch( ... )
	{
		LUA->PushString( "Unknown Lua error." );
	}

	return LUA->Error( );
}

extern "C" LUAINTERFACE_API_IMPORT void ( *luai_userstatethread ) ( lua_State *, lua_State * );
extern "C" LUAINTERFACE_API_IMPORT void ( *luai_userstatefree ) ( lua_State *, lua_State * );

extern "C" void luai_userstatethread_impl( lua_State *, lua_State *state )
{
	state->lua_interface = new LuaInterface( (lua_State *)state );
}

extern "C" void luai_userstatefree_impl( lua_State *, lua_State *state )
{
	if( state->lua_interface != 0 )
		delete state->lua_interface;
}

extern "C" LUAINTERFACE_API_IMPORT const char *luai_moduleloadfunc;
extern "C" LUAINTERFACE_API_IMPORT const char *luai_moduleunloadfunc;

static bool __lua_initialized = false;
static void InitializeLua( )
{
	if( !__lua_initialized )
	{
		__lua_initialized = true;

		luai_userstatethread = luai_userstatethread_impl;
		luai_userstatefree = luai_userstatefree_impl;

#define __STRINGIFY2( s ) #s
#define __STRINGIFY( s ) __STRINGIFY2( s )

		luai_moduleloadfunc = __STRINGIFY( LUA_MODULE_LOAD_NAME );
		luai_moduleunloadfunc = __STRINGIFY( LUA_MODULE_UNLOAD_NAME );

#undef __STRINGIFY
#undef __STRINGIFY2
	}
}

LUA_FUNCTION( LuaPanic )
{
	luaL_Buffer buffer;
	LUA->BufferInit( &buffer );
	LUA->BufferAddString( &buffer, "LUA PANIC: unprotected error in call to Lua API (" );
	LUA->BufferAddString( &buffer, LUA->ToString( ) );
	LUA->BufferAddString( &buffer, ")" );
	LUA->BufferFinish( &buffer );

	throw lua_exception( LUA->ToString( ) );
	return 0;
}

LuaInterface::LuaInterface( lua_State *state ) : lua_state( state ), manual( false )
{
	InitializeLua( );

	if( lua_state != 0 )
		return;

	if( ( lua_state = luaL_newstate( ) ) == 0 )
		throw lua_exception( "Unable to create a new Lua state!" );

	manual = true;

	lua_state->lua_interface = this;

#if defined LUAJIT_VERSION && ( defined _WIN32 && !defined _WIN64 )
	lua_pushlightuserdata( lua_state, SafeLuaFunction );
	luaJIT_setmode( lua_state, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON );
#endif

	lua_atpanic( lua_state, LuaPanic );

	luaL_openlibs( lua_state );
}

LuaInterface::~LuaInterface( )
{
	lua_state->lua_interface = 0;
	if( manual && lua_state != 0 )
		lua_close( lua_state );
}

int LuaInterface::GetTop( )
{
	return lua_gettop( lua_state );
}

void LuaInterface::SetTop( int num )
{
	lua_settop( lua_state, num );
}

void LuaInterface::Pop( int amount )
{
	lua_pop( lua_state, amount );
}

void LuaInterface::CreateTable( int array_elems, int nonarray_elems )
{
	lua_createtable( lua_state, array_elems, nonarray_elems );
}

void LuaInterface::GetTable( int stackpos )
{
	lua_gettable( lua_state, stackpos );
}

void LuaInterface::SetTable( int stackpos )
{
	lua_settable( lua_state, stackpos );
}

void LuaInterface::GetField( int stackpos, const char *strName )
{
	lua_getfield( lua_state, stackpos, strName );
}

void LuaInterface::SetField( int stackpos, const char *strName )
{
	lua_setfield( lua_state, stackpos, strName );
}

int LuaInterface::Next( int stackpos )
{
	return lua_next( lua_state, stackpos );
}

int LuaInterface::NewMetatable( const char *name )
{
	return luaL_newmetatable( lua_state, name );
}

void LuaInterface::GetMetaTable( const char *name )
{
	luaL_getmetatable( lua_state, name );
}

int LuaInterface::GetMetaTable( int stackpos )
{
	return lua_getmetatable( lua_state, stackpos );
}

int LuaInterface::GetMetaTableField( int stackpos, const char *strName )
{
	return luaL_getmetafield( lua_state, stackpos, strName );
}

int LuaInterface::SetMetaTable( int stackpos )
{
	return lua_setmetatable( lua_state, stackpos );
}

void LuaInterface::Call( int args, int results )
{
	lua_call( lua_state, args, results );
}

int LuaInterface::PCall( int args, int results, int errorfuncpos )
{
	return lua_pcall( lua_state, args, results, errorfuncpos );
}

int LuaInterface::CallMeta( int stackpos, const char *strName )
{
	return luaL_callmeta( lua_state, stackpos, strName );
}

void LuaInterface::GetFEnv( int stackpos )
{
#if LUA_VERSION_NUM >= 502
	lua_getuservalue( lua_state, stackpos );
#else
	lua_getfenv( lua_state, stackpos );
#endif
}

int LuaInterface::SetFEnv( int stackpos )
{
#if LUA_VERSION_NUM >= 502
	lua_getuservalue( lua_state, stackpos );
#else
	lua_getfenv( lua_state, stackpos );
#endif
	return lua_setfenv( lua_state, stackpos );
}

void LuaInterface::Insert( int stackpos )
{
	lua_insert( lua_state, stackpos );
}

void LuaInterface::Remove( int stackpos )
{
	lua_remove( lua_state, stackpos );
}

void LuaInterface::XMove( LuaInterface *lua_interface, int n )
{
	lua_xmove( lua_state, lua_interface->lua_state, n );
}

int LuaInterface::Error( )
{
	return lua_error( lua_state );
}

int LuaInterface::ThrowError( const char *error )
{
	PushString( error );
	return Error( );
}

int LuaInterface::ArgError( int argnum, const char *message )
{
	return luaL_argerror( lua_state, argnum, message );
}

int LuaInterface::TypeError( int argnum, const char *type_expected )
{
	luaL_Buffer buffer;
	BufferInit( &buffer );
	BufferAddString( &buffer, type_expected );
	BufferAddString( &buffer, " expected, got " );
	BufferAddString( &buffer, GetTypeName( GetType( argnum ) ) );
	BufferFinish( &buffer );
	return ArgError( argnum, ToString( ) );
}

const char *LuaInterface::OptionalString( int stackpos, const char *def, size_t *outlen )
{
	return luaL_optlstring( lua_state, stackpos, def, outlen );
}

double LuaInterface::OptionalNumber( int stackpos, double def )
{
	return luaL_optnumber( lua_state, stackpos, def );
}

ptrdiff_t LuaInterface::OptionalInteger( int stackpos, ptrdiff_t def )
{
	return luaL_optinteger( lua_state, stackpos, def );
}

const char *LuaInterface::CheckString( int stackpos, size_t *outlen )
{
	return luaL_checklstring( lua_state, stackpos, outlen );
}

double LuaInterface::CheckNumber( int stackpos )
{
	return luaL_checknumber( lua_state, stackpos );
}

ptrdiff_t LuaInterface::CheckInteger( int stackpos )
{
	return luaL_checkinteger( lua_state, stackpos );
}

void LuaInterface::CheckType( int stackpos, int type )
{
	luaL_checktype( lua_state, stackpos, type );
}

void *LuaInterface::CheckUserdata( int stackpos, const char *name )
{
	return luaL_checkudata( lua_state, stackpos, name );
}

void LuaInterface::CheckAny( int stackpos )
{
	luaL_checkany( lua_state, stackpos );
}

int LuaInterface::Equal( int stackpos_a, int stackpos_b )
{
#if LUA_VERSION_NUM >= 502
	return lua_compare( lua_state, stackpos_a, stackpos_b, LUA_OPEQ );
#else
	return lua_equal( lua_state, stackpos_a, stackpos_b );
#endif
}

int LuaInterface::RawEqual( int stackpos_a, int stackpos_b )
{
	return lua_rawequal( lua_state, stackpos_a, stackpos_b );
}

void LuaInterface::RawGet( int stackpos )
{
	lua_rawget( lua_state, stackpos );
}

void LuaInterface::RawSet( int stackpos )
{
	lua_rawset( lua_state, stackpos );
}

void LuaInterface::OpenLibrary( const char *libname, const luaL_Reg *list, int nup )
{
	luaL_openlib( lua_state, libname, list, nup );
}

const char *LuaInterface::ToString( int stackpos, size_t *outlen )
{
	return lua_tolstring( lua_state, stackpos, outlen );
}

double LuaInterface::ToNumber( int stackpos )
{
	return lua_tonumber( lua_state, stackpos );
}

ptrdiff_t LuaInterface::ToInteger( int stackpos )
{
	return lua_tointeger( lua_state, stackpos );
}

bool LuaInterface::ToBoolean( int stackpos )
{
	return lua_toboolean( lua_state, stackpos ) == 1;
}

lua_CFunction LuaInterface::ToCFunction( int stackpos )
{
	return lua_tocfunction( lua_state, stackpos );
}

void *LuaInterface::ToUserdata( int stackpos )
{
	return lua_touserdata( lua_state, stackpos );
}

void *LuaInterface::NewUserdata( size_t size )
{
	return lua_newuserdata( lua_state, size );
}

void LuaInterface::BufferInit( luaL_Buffer *buffer )
{
	luaL_buffinit( lua_state, buffer );
}

char *LuaInterface::BufferPrepare( luaL_Buffer *buffer )
{
	return luaL_prepbuffer( buffer );
}

void LuaInterface::BufferAddChar( luaL_Buffer *buffer, const char ch )
{
	luaL_addchar( buffer, ch );
}

void LuaInterface::BufferAddString( luaL_Buffer *buffer, const char *str, size_t len )
{
	if( len == 0 )
		luaL_addstring( buffer, str );
	else
		luaL_addlstring( buffer, str, len );
}

void LuaInterface::BufferAddSize( luaL_Buffer *buffer, size_t size )
{
	luaL_addsize( buffer, size );
}

void LuaInterface::BufferAdd( luaL_Buffer *buffer )
{
	luaL_addvalue( buffer );
}

void LuaInterface::BufferFinish( luaL_Buffer *buffer )
{
	luaL_pushresult( buffer );
}

void LuaInterface::PushValue( int stackpos )
{
	lua_pushvalue( lua_state, stackpos );
}

void LuaInterface::PushNil( )
{
	lua_pushnil( lua_state );
}

void LuaInterface::PushString( const char *val, size_t len )
{
	if( len == 0 )
		lua_pushstring( lua_state, val );
	else
		lua_pushlstring( lua_state, val, len );
}

void LuaInterface::PushNumber( double val )
{
	lua_pushnumber( lua_state, val );
}

void LuaInterface::PushInteger( ptrdiff_t val )
{
	lua_pushinteger( lua_state, val );
}

void LuaInterface::PushBoolean( bool val )
{
	lua_pushboolean( lua_state, val );
}

void LuaInterface::PushCFunction( lua_CFunction val )
{
	lua_pushcfunction( lua_state, val );
}

void LuaInterface::PushCClosure( lua_CFunction val, int vars )
{
	lua_pushcclosure( lua_state, val, vars );
}

void LuaInterface::PushLightUserdata( void *val )
{
	lua_pushlightuserdata( lua_state, val );
}

void LuaInterface::PushPseudoIndex( LuaPseudoIndex index )
{
	switch( index )
	{
		case LUA_GLOBAL:
#if LUA_VERSION_NUM >= 502
			lua_pushglobaltable( lua_state );
#else
			PushValue( LUA_GLOBALSINDEX );
#endif
			break;

		case LUA_REGISTRY:
			PushValue( LUA_REGISTRYINDEX );
			break;
	}
}

int LuaInterface::LoadBuffer( const char *data, size_t size, const char *name )
{
	return luaL_loadbuffer( lua_state, data, size, name );
}

int LuaInterface::LoadString( const char *data )
{
	return luaL_loadstring( lua_state, data );
}

int LuaInterface::LoadFile( const char *path )
{
	return luaL_loadfile( lua_state, path );
}

int LuaInterface::ReferenceCreate( )
{
	return luaL_ref( lua_state, LUA_REGISTRYINDEX );
}

void LuaInterface::ReferenceFree( int ref )
{
	luaL_unref( lua_state, LUA_REGISTRYINDEX, ref );
}

void LuaInterface::ReferencePush( int ref )
{
	PushInteger( ref );
	RawGet( LUA_REGISTRYINDEX );
}

int LuaInterface::GetType( int stackpos )
{
	return lua_type( lua_state, stackpos );
}

bool LuaInterface::IsType( int stackpos, int type )
{
	return GetType( stackpos ) == type;
}

const char *LuaInterface::GetTypeName( int type )
{
	return lua_typename( lua_state, type );
}

static int FunctionDumper( lua_State *state, const void *bchunk, size_t size, void *buffer )
{
	LUA->BufferAddString( (luaL_Buffer *)buffer, (const char *)bchunk, size );
	return 0;
}

const char *LuaInterface::Dump( size_t *outlen )
{
	luaL_Buffer buffer;
	BufferInit( &buffer );

	if( lua_dump( lua_state, FunctionDumper, &buffer ) != 0 )
	{
		return 0;
	}

	BufferFinish( &buffer );
	return ToString( -1, outlen );
}

bool LuaInterface::RunBuffer( const char *data, size_t size, const char *name, bool pcall )
{
	if( LoadBuffer( data, size, name ) == 0 )
	{
		if( pcall )
			return PCall( ) == 0;

		Call( );
		return true;
	}

	return false;
}

bool LuaInterface::RunString( const char *data, bool pcall )
{
	if( LoadString( data ) == 0 )
	{
		if( pcall )
			return PCall( ) == 0;

		Call( );
		return true;
	}

	return false;
}

bool LuaInterface::RunFile( const char *path, bool pcall )
{
	if( LoadFile( path ) == 0 )
	{
		if( pcall )
			return PCall( ) == 0;

		Call( );
		return true;
	}

	return false;
}
