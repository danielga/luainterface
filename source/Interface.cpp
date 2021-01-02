#include <lua.hpp>
#include <Lua/Interface.hpp>
#include <Internal.hpp>
#include <Reference.hpp>
#include <stdexcept>

#if defined LUAJIT_VERSION

#define LUA_EXTERN extern "C"

// On LuaJIT, use the C call wrapper feature http://luajit.org/ext_c_api.html#mode_wrapcfunc
static int SafeLuaFunction( lua_State *state, lua_CFunction func )
{
	Lua::Interface &lua = GetLuaInterface( state );

	try
	{
		return func( state );
	}
	catch( const char *s )
	{
		lua.PushString( s );
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}
	catch( ... )
	{
		lua.PushString( "Unknown Lua error." );
	}

	return lua.Error( );
}

#else

#define LUA_EXTERN

#endif

#define __STRINGIFY2( s ) #s
#define __STRINGIFY( s ) __STRINGIFY2( s )

LUA_EXTERN const char *luai_moduleloadfunc = __STRINGIFY( LUA_MODULE_LOAD_NAME );
LUA_EXTERN const char *luai_moduleunloadfunc = __STRINGIFY( LUA_MODULE_UNLOAD_NAME );

#undef __STRINGIFY
#undef __STRINGIFY2

LUA_EXTERN void luai_userstatethread( lua_State *global, lua_State *state )
{
	Lua::UserstateCreated( global, state );
}

LUA_EXTERN void luai_userstatefree( lua_State *, lua_State *state )
{
	delete &GetLuaInterface( state );
}

static int LuaPanic( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	throw Lua::Panic( lua, lua.ToString( -1 ) );
}

namespace Lua
{

static void UserstateCreated( lua_State *global, lua_State *state )
{
	new Interface( state, global );
}

const char *Interface::Version( )
{

#ifdef LUAJIT_VERSION

	return LUAJIT_VERSION;

#else

	return LUA_VERSION;

#endif

}

int Interface::VersionNumber( )
{

#ifdef LUAJIT_VERSION_NUM

	return LUAJIT_VERSION_NUM;

#else

	return LUA_VERSION_NUM;

#endif

}

Interface::Interface( ) :
	lua_state( luaL_newstate( ) ), global_state( nullptr )
{
	Internal::SetLuaInterface( lua_state, this );

#if defined LUAJIT_VERSION && defined _WIN32 && !defined _WIN64

	lua_pushlightuserdata( lua_state, SafeLuaFunction );
	luaJIT_setmode( lua_state, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON );

#endif

	lua_atpanic( lua_state, LuaPanic );

	luaL_openlibs( lua_state );
}

Interface::Interface( lua_State *state, lua_State *global ) :
	lua_state( state ), global_state( global )
{
	Internal::SetLuaInterface( lua_state, this );
}

Interface::~Interface( )
{
	if( global_state == nullptr )
		lua_close( lua_state );

	lua_state = nullptr;
	global_state = nullptr;
}

lua_State *Interface::GetLuaState( ) const
{
	return lua_state;
}

int Interface::GetTop( )
{
	return lua_gettop( lua_state );
}

void Interface::SetTop( int num )
{
	lua_settop( lua_state, num );
}

void Interface::Pop( int amount )
{
	lua_pop( lua_state, amount );
}

void Interface::CreateTable( int array_elems, int nonarray_elems )
{
	lua_createtable( lua_state, array_elems, nonarray_elems );
}

void Interface::GetTable( int stackpos )
{
	lua_gettable( lua_state, stackpos );
}

void Interface::SetTable( int stackpos )
{
	lua_settable( lua_state, stackpos );
}

void Interface::GetField( int stackpos, const char *strName )
{
	lua_getfield( lua_state, stackpos, strName );
}

void Interface::SetField( int stackpos, const char *strName )
{
	lua_setfield( lua_state, stackpos, strName );
}

int Interface::Next( int stackpos )
{
	return lua_next( lua_state, stackpos );
}

int Interface::NewMetatable( const char *name )
{
	return luaL_newmetatable( lua_state, name );
}

void Interface::GetMetaTable( const char *name )
{
	luaL_getmetatable( lua_state, name );
}

int Interface::GetMetaTable( int stackpos )
{
	return lua_getmetatable( lua_state, stackpos );
}

int Interface::GetMetaTableField( int stackpos, const char *strName )
{
	return luaL_getmetafield( lua_state, stackpos, strName );
}

int Interface::SetMetaTable( int stackpos )
{
	return lua_setmetatable( lua_state, stackpos );
}

void Interface::Call( int args, int results )
{
	lua_call( lua_state, args, results );
}

int Interface::PCall( int args, int results, int errorfuncpos )
{
	return lua_pcall( lua_state, args, results, errorfuncpos );
}

int Interface::CallMeta( int stackpos, const char *strName )
{
	return luaL_callmeta( lua_state, stackpos, strName );
}

void Interface::GetUserValue( int stackpos )
{

#if LUA_VERSION_NUM >= 502

	lua_getuservalue( lua_state, stackpos );

#else

	CheckType( stackpos, Lua::Type::Userdata );
	lua_getfenv( lua_state, stackpos );

#endif

}

int Interface::SetUserValue( int stackpos )
{

#if LUA_VERSION_NUM >= 502

	lua_setuservalue( lua_state, stackpos );
	return 1;

#else

	CheckType( stackpos, Lua::Type::Userdata );
	return lua_setfenv( lua_state, stackpos );

#endif

}

void Interface::Insert( int stackpos )
{
	lua_insert( lua_state, stackpos );
}

void Interface::Remove( int stackpos )
{
	lua_remove( lua_state, stackpos );
}

void Interface::Replace( int stackpos )
{
	return lua_replace( lua_state, stackpos );
}

void Interface::XMove( Interface &lua_interface, int n )
{
	lua_xmove( lua_state, lua_interface.lua_state, n );
}

int Interface::Error( )
{
	return lua_error( lua_state );
}

int Interface::ArgError( int argnum, const char *message )
{
	return luaL_argerror( lua_state, argnum, message );
}

int Interface::TypeError( int argnum, const char *type_expected )
{
	const char *msg = PushFormattedString( "%s expected, got %s", type_expected, GetTypeName( GetType( argnum ) ) );
	return ArgError( argnum, msg );
}

const char *Interface::OptionalString( int stackpos, const char *def, size_t *outlen )
{
	return luaL_optlstring( lua_state, stackpos, def, outlen );
}

double Interface::OptionalNumber( int stackpos, double def )
{
	return luaL_optnumber( lua_state, stackpos, def );
}

long long Interface::OptionalInteger( int stackpos, long long def )
{
	return luaL_optinteger( lua_state, stackpos, static_cast<lua_Integer>( def ) );
}

const char *Interface::CheckString( int stackpos, size_t *outlen )
{
	return luaL_checklstring( lua_state, stackpos, outlen );
}

double Interface::CheckNumber( int stackpos )
{
	return luaL_checknumber( lua_state, stackpos );
}

long long Interface::CheckInteger( int stackpos )
{
	return luaL_checkinteger( lua_state, stackpos );
}

void *Interface::CheckUserdata( int stackpos, const char *name )
{
	return luaL_checkudata( lua_state, stackpos, name );
}

void Interface::CheckAny( int stackpos )
{
	luaL_checkany( lua_state, stackpos );
}

void Interface::CheckStack( int size, const char *msg )
{
	luaL_checkstack( lua_state, size, msg );
}

int Interface::Equal( int stackpos_a, int stackpos_b )
{

#if LUA_VERSION_NUM >= 502

	return lua_compare( lua_state, stackpos_a, stackpos_b, LUA_OPEQ );

#else

	return lua_equal( lua_state, stackpos_a, stackpos_b );

#endif

}

int Interface::RawEqual( int stackpos_a, int stackpos_b )
{
	return lua_rawequal( lua_state, stackpos_a, stackpos_b );
}

void Interface::RawGet( int stackpos )
{
	lua_rawget( lua_state, stackpos );
}

void Interface::RawGetI( int stackpos, int n )
{
	lua_rawgeti( lua_state, stackpos, n );
}

void Interface::RawSet( int stackpos )
{
	lua_rawset( lua_state, stackpos );
}

void Interface::RawSetI( int stackpos, int n )
{
	lua_rawseti( lua_state, stackpos, n );
}

void Interface::Register( const char *libname, const ModuleFunction *list )
{
	luaL_register( lua_state, libname, reinterpret_cast<const luaL_Reg *>( list ) );
}

const char *Interface::ToString( int stackpos, size_t *outlen )
{
	return lua_tolstring( lua_state, stackpos, outlen );
}

const char *Interface::ToString( int stackpos )
{
	return lua_tostring( lua_state, stackpos );
}

double Interface::ToNumber( int stackpos )
{
	return lua_tonumber( lua_state, stackpos );
}

long long Interface::ToInteger( int stackpos )
{
	return lua_tointeger( lua_state, stackpos );
}

bool Interface::ToBoolean( int stackpos )
{
	return lua_toboolean( lua_state, stackpos ) == 1;
}

lua_CFunction Interface::ToFunction( int stackpos )
{
	return lua_tocfunction( lua_state, stackpos );
}

void *Interface::ToUserdata( int stackpos )
{
	return lua_touserdata( lua_state, stackpos );
}

void *Interface::NewUserdata( size_t size )
{
	return lua_newuserdata( lua_state, size );
}

lua_State *Interface::NewThread( )
{
	return lua_newthread( lua_state );
}

int Interface::Yield( int results )
{
	return lua_yield( lua_state, results );
}

int Interface::Resume( int args )
{

#if LUA_VERSION_NUM >= 503

	return lua_resume( lua_state, nullptr, args );

#else

	return lua_resume( lua_state, args );

#endif

}

int Interface::Resume( const Interface &from, int args )
{

#if LUA_VERSION_NUM >= 503

	return lua_resume( lua_state, from.GetLuaState( ), args );

#else

	return lua_resume( lua_state, args );

#endif

}

int Interface::Status( )
{
	return lua_status( lua_state );
}

Buffer *Interface::BufferInit( )
{
	luaL_Buffer *buffer = new luaL_Buffer;
	luaL_buffinit( lua_state, buffer );
	return reinterpret_cast<Buffer *>( buffer );
}

char *Interface::BufferPrepare( Buffer *buffer )
{
	return luaL_prepbuffer( reinterpret_cast<luaL_Buffer *>( buffer ) );
}

void Interface::BufferAddChar( Buffer *buffer, const char ch )
{
	luaL_addchar( reinterpret_cast<luaL_Buffer *>( buffer ), ch );
}

void Interface::BufferAddString( Buffer *buffer, const char *str, size_t len )
{
	luaL_addlstring( reinterpret_cast<luaL_Buffer *>( buffer ), str, len );
}

void Interface::BufferAddString( Buffer *buffer, const char *str )
{
	luaL_addstring( reinterpret_cast<luaL_Buffer *>( buffer ), str );
}

void Interface::BufferAddSize( Buffer *buffer, size_t size )
{
	luaL_addsize( reinterpret_cast<luaL_Buffer *>( buffer ), size );
}

void Interface::BufferAdd( Buffer *buffer )
{
	luaL_addvalue( reinterpret_cast<luaL_Buffer *>( buffer ) );
}

void Interface::BufferFinish( Buffer *b )
{
	luaL_Buffer *buffer = reinterpret_cast<luaL_Buffer *>( b );
	luaL_pushresult( buffer );
	delete buffer;
}

const char *Interface::GSub( const char *str, const char *pattern, const char *replacement )
{
	return luaL_gsub( lua_state, str, pattern, replacement );
}

void Interface::PushValue( int stackpos )
{
	lua_pushvalue( lua_state, stackpos );
}

void Interface::PushNil( )
{
	lua_pushnil( lua_state );
}

void Interface::PushString( const char *val, size_t len )
{
	lua_pushlstring( lua_state, val, len );
}

void Interface::PushString( const char *val )
{
	lua_pushstring( lua_state, val );
}

const char *Interface::PushFormattedString( const char *fmt, ... )
{
	va_list argp;
	va_start( argp, fmt );
	const char *str = PushFormattedString( fmt, argp );
	va_end( argp );
	return str;
}

const char *Interface::PushFormattedString( const char *fmt, va_list argp )
{
	return lua_pushvfstring( lua_state, fmt, argp );
}

void Interface::PushNumber( double val )
{
	lua_pushnumber( lua_state, val );
}

void Interface::PushInteger( long long val )
{
	lua_pushinteger( lua_state, val );
}

void Interface::PushBoolean( bool val )
{
	lua_pushboolean( lua_state, val );
}

void Interface::PushFunction( Function val )
{
	lua_pushcfunction( lua_state, val );
}

void Interface::PushClosure( Function val, int vars )
{
	lua_pushcclosure( lua_state, val, vars );
}

void Interface::PushLightUserdata( void *val )
{
	lua_pushlightuserdata( lua_state, val );
}

int Interface::PushThread( lua_State *thread )
{
	return lua_pushthread( thread );
}

void Interface::PushGlobal( )
{

#if LUA_VERSION_NUM >= 502

	lua_pushglobaltable( lua_state );

#else

	PushValue( LUA_GLOBALSINDEX );

#endif

}

void Interface::PushRegistry( )
{
	PushValue( LUA_REGISTRYINDEX );
}

int Interface::LoadBuffer( const char *data, size_t size, const char *name )
{
	return luaL_loadbuffer( lua_state, data, size, name );
}

int Interface::LoadString( const char *data )
{
	return luaL_loadstring( lua_state, data );
}

int Interface::LoadFile( const char *path )
{
	return luaL_loadfile( lua_state, path );
}

int Interface::ReferenceCreate( )
{
	return luaL_ref( lua_state, LUA_REGISTRYINDEX );
}

void Interface::ReferenceFree( int ref )
{
	luaL_unref( lua_state, LUA_REGISTRYINDEX, ref );
}

void Interface::ReferencePush( int ref )
{
	RawGetI( LUA_REGISTRYINDEX, ref );
}

const char *Interface::GetTypeName( Type type )
{
	return lua_typename( lua_state, static_cast<int>( type ) );
}

int Interface::GarbageCollect( GC what, int data )
{
	return lua_gc( lua_state, static_cast<int>( what ), data );
}

Type Interface::GetType( int stackpos )
{
	return static_cast<Type>( lua_type( lua_state, stackpos ) );
}

bool Interface::IsType( int stackpos, Type type )
{
	return GetType( stackpos ) == type;
}

void Interface::CheckType( int stackpos, Type type )
{
	luaL_checktype( lua_state, stackpos, static_cast<int>( type ) );
}

Object Interface::ToObject( int stackpos )
{
	return Object( this, stackpos );
}

void Interface::PushObject( const Object &obj )
{
	ReferencePush( obj.reference->Get( ) );
}

int Interface::ThrowError( const char *fmt, ... )
{
	va_list argp;
	va_start( argp, fmt );
	PushFormattedString( fmt, argp );
	va_end( argp );
	return Error( );
}

static int FunctionDumper( lua_State *state, const void *bchunk, size_t size, void *buffer )
{
	Buffer *lua_buffer = reinterpret_cast<Buffer *>( buffer );
	const char *binary_chunk = reinterpret_cast<const char *>( bchunk );
	GetLuaInterface( state ).BufferAddString( lua_buffer, binary_chunk, size );
	return 0;
}

const char *Interface::Dump( size_t *outlen, bool strip )
{
	Buffer *buffer = BufferInit( );

#if LUA_VERSION_NUM >= 503

	if( lua_dump( lua_state, FunctionDumper, &buffer, strip ? 1 : 0 ) == 0 )

#else

	if( lua_dump( lua_state, FunctionDumper, &buffer ) == 0 )

#endif

		return nullptr;

	BufferFinish( buffer );
	return ToString( -1, outlen );
}

bool Interface::RunBuffer( const char *data, size_t size, const char *name, bool pcall )
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

bool Interface::RunString( const char *data, bool pcall )
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

bool Interface::RunFile( const char *path, bool pcall )
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

}
