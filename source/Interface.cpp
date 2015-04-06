#include <lua.hpp>
#include <Lua/Interface.hpp>
#include <Internal.hpp>
#include <Reference.hpp>
#include <stdexcept>

#if defined LUAJIT_VERSION

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

#endif

#define __STRINGIFY2( s ) #s
#define __STRINGIFY( s ) __STRINGIFY2( s )

extern "C" const char *luai_moduleloadfunc = __STRINGIFY( LUA_MODULE_LOAD_NAME );
extern "C" const char *luai_moduleunloadfunc = __STRINGIFY( LUA_MODULE_UNLOAD_NAME );

#undef __STRINGIFY
#undef __STRINGIFY2

extern "C" void luai_userstatethread( lua_State *, lua_State *state )
{
	Lua::UserstateCreated( state );
}

extern "C" void luai_userstatefree( lua_State *, lua_State *state )
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

static void UserstateCreated( lua_State *state )
{
	new Lua::Interface( state );
}

static void DoNothing( lua_State * )
{ }

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
	state_wrapper( luaL_newstate( ), lua_close )
{
	Internal::SetLuaInterface( state_wrapper.get( ), this );

#if defined LUAJIT_VERSION && defined _WIN32 && !defined _WIN64

	lua_pushlightuserdata( state_wrapper.get( ), SafeLuaFunction );
	luaJIT_setmode( state_wrapper.get( ), -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON );

#endif

	lua_atpanic( state_wrapper.get( ), LuaPanic );

	luaL_openlibs( state_wrapper.get( ) );
}

Interface::Interface( lua_State *state ) :
	state_wrapper( state, DoNothing )
{
	Internal::SetLuaInterface( state_wrapper.get( ), this );
}

lua_State *Interface::GetLuaState( ) const
{
	return state_wrapper.get( );
}

int Interface::GetTop( )
{
	return lua_gettop( state_wrapper.get( ) );
}

void Interface::SetTop( int num )
{
	lua_settop( state_wrapper.get( ), num );
}

void Interface::Pop( int amount )
{
	lua_pop( state_wrapper.get( ), amount );
}

void Interface::CreateTable( int array_elems, int nonarray_elems )
{
	lua_createtable( state_wrapper.get( ), array_elems, nonarray_elems );
}

void Interface::GetTable( int stackpos )
{
	lua_gettable( state_wrapper.get( ), stackpos );
}

void Interface::SetTable( int stackpos )
{
	lua_settable( state_wrapper.get( ), stackpos );
}

void Interface::GetField( int stackpos, const char *strName )
{
	lua_getfield( state_wrapper.get( ), stackpos, strName );
}

void Interface::SetField( int stackpos, const char *strName )
{
	lua_setfield( state_wrapper.get( ), stackpos, strName );
}

int Interface::Next( int stackpos )
{
	return lua_next( state_wrapper.get( ), stackpos );
}

int Interface::NewMetatable( const char *name )
{
	return luaL_newmetatable( state_wrapper.get( ), name );
}

void Interface::GetMetaTable( const char *name )
{
	luaL_getmetatable( state_wrapper.get( ), name );
}

int Interface::GetMetaTable( int stackpos )
{
	return lua_getmetatable( state_wrapper.get( ), stackpos );
}

int Interface::GetMetaTableField( int stackpos, const char *strName )
{
	return luaL_getmetafield( state_wrapper.get( ), stackpos, strName );
}

int Interface::SetMetaTable( int stackpos )
{
	return lua_setmetatable( state_wrapper.get( ), stackpos );
}

void Interface::Call( int args, int results )
{
	lua_call( state_wrapper.get( ), args, results );
}

int Interface::PCall( int args, int results, int errorfuncpos )
{
	return lua_pcall( state_wrapper.get( ), args, results, errorfuncpos );
}

int Interface::CallMeta( int stackpos, const char *strName )
{
	return luaL_callmeta( state_wrapper.get( ), stackpos, strName );
}

void Interface::GetUserValue( int stackpos )
{

#if LUA_VERSION_NUM >= 502

	lua_getuservalue( lua_state, stackpos );

#else

	CheckType( stackpos, Lua::Type::Userdata );
	lua_getfenv( state_wrapper.get( ), stackpos );

#endif

}

int Interface::SetUserValue( int stackpos )
{

#if LUA_VERSION_NUM >= 502

	return lua_setuservalue( lua_state, stackpos );

#else

	CheckType( stackpos, Lua::Type::Userdata );
	return lua_setfenv( state_wrapper.get( ), stackpos );

#endif

}

void Interface::Insert( int stackpos )
{
	lua_insert( state_wrapper.get( ), stackpos );
}

void Interface::Remove( int stackpos )
{
	lua_remove( state_wrapper.get( ), stackpos );
}

void Interface::Replace( int stackpos )
{
	return lua_replace( state_wrapper.get( ), stackpos );
}

void Interface::XMove( Interface &lua_interface, int n )
{
	lua_xmove( state_wrapper.get( ), lua_interface.state_wrapper.get( ), n );
}

int Interface::Error( )
{
	return lua_error( state_wrapper.get( ) );
}

int Interface::ArgError( int argnum, const char *message )
{
	return luaL_argerror( state_wrapper.get( ), argnum, message );
}

int Interface::TypeError( int argnum, const char *type_expected )
{
	const char *msg = PushFormattedString( "%s expected, got %s", type_expected, GetTypeName( GetType( argnum ) ) );
	return ArgError( argnum, msg );
}

const char *Interface::OptionalString( int stackpos, const char *def, size_t *outlen )
{
	return luaL_optlstring( state_wrapper.get( ), stackpos, def, outlen );
}

double Interface::OptionalNumber( int stackpos, double def )
{
	return luaL_optnumber( state_wrapper.get( ), stackpos, def );
}

ptrdiff_t Interface::OptionalInteger( int stackpos, ptrdiff_t def )
{
	return luaL_optinteger( state_wrapper.get( ), stackpos, def );
}

const char *Interface::CheckString( int stackpos, size_t *outlen )
{
	return luaL_checklstring( state_wrapper.get( ), stackpos, outlen );
}

double Interface::CheckNumber( int stackpos )
{
	return luaL_checknumber( state_wrapper.get( ), stackpos );
}

ptrdiff_t Interface::CheckInteger( int stackpos )
{
	return luaL_checkinteger( state_wrapper.get( ), stackpos );
}

void *Interface::CheckUserdata( int stackpos, const char *name )
{
	return luaL_checkudata( state_wrapper.get( ), stackpos, name );
}

void Interface::CheckAny( int stackpos )
{
	luaL_checkany( state_wrapper.get( ), stackpos );
}

void Interface::CheckStack( int size, const char *msg )
{
	luaL_checkstack( state_wrapper.get( ), size, msg );
}

int Interface::Equal( int stackpos_a, int stackpos_b )
{
	return lua_equal( state_wrapper.get( ), stackpos_a, stackpos_b );
}

int Interface::RawEqual( int stackpos_a, int stackpos_b )
{
	return lua_rawequal( state_wrapper.get( ), stackpos_a, stackpos_b );
}

void Interface::RawGet( int stackpos )
{
	lua_rawget( state_wrapper.get( ), stackpos );
}

void Interface::RawGetI( int stackpos, int n )
{
	lua_rawgeti( state_wrapper.get( ), stackpos, n );
}

void Interface::RawSet( int stackpos )
{
	lua_rawset( state_wrapper.get( ), stackpos );
}

void Interface::RawSetI( int stackpos, int n )
{
	lua_rawseti( state_wrapper.get( ), stackpos, n );
}

void Interface::Register( const char *libname, const ModuleFunction *list )
{
	luaL_register( state_wrapper.get( ), libname, reinterpret_cast<const luaL_Reg *>( list ) );
}

const char *Interface::ToString( int stackpos, size_t *outlen )
{
	return lua_tolstring( state_wrapper.get( ), stackpos, outlen );
}

const char *Interface::ToString( int stackpos )
{
	return lua_tostring( state_wrapper.get( ), stackpos );
}

double Interface::ToNumber( int stackpos )
{
	return lua_tonumber( state_wrapper.get( ), stackpos );
}

ptrdiff_t Interface::ToInteger( int stackpos )
{
	return lua_tointeger( state_wrapper.get( ), stackpos );
}

bool Interface::ToBoolean( int stackpos )
{
	return lua_toboolean( state_wrapper.get( ), stackpos ) == 1;
}

lua_CFunction Interface::ToFunction( int stackpos )
{
	return lua_tocfunction( state_wrapper.get( ), stackpos );
}

void *Interface::ToUserdata( int stackpos )
{
	return lua_touserdata( state_wrapper.get( ), stackpos );
}

void *Interface::NewUserdata( size_t size )
{
	return lua_newuserdata( state_wrapper.get( ), size );
}

lua_State *Interface::NewThread( )
{
	return lua_newthread( state_wrapper.get( ) );
}

int Interface::Yield( int results )
{
	return lua_yield( state_wrapper.get( ), results );
}

int Interface::Resume( int args )
{
	return lua_resume( state_wrapper.get( ), args );
}

int Interface::Status( )
{
	return lua_status( state_wrapper.get( ) );
}

void Interface::BufferInit( Buffer *buffer )
{
	luaL_buffinit( state_wrapper.get( ), reinterpret_cast<luaL_Buffer *>( buffer ) );
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
	luaL_addsize( buffer, size );
}

void Interface::BufferAdd( Buffer *buffer )
{
	luaL_addvalue( reinterpret_cast<luaL_Buffer *>( buffer ) );
}

void Interface::BufferFinish( Buffer *buffer )
{
	luaL_pushresult( reinterpret_cast<luaL_Buffer *>( buffer ) );
}

const char *Interface::GSub( const char *str, const char *pattern, const char *replacement )
{
	return luaL_gsub( state_wrapper.get( ), str, pattern, replacement );
}

void Interface::PushValue( int stackpos )
{
	lua_pushvalue( state_wrapper.get( ), stackpos );
}

void Interface::PushNil( )
{
	lua_pushnil( state_wrapper.get( ) );
}

void Interface::PushString( const char *val, size_t len )
{
	lua_pushlstring( state_wrapper.get( ), val, len );
}

void Interface::PushString( const char *val )
{
	lua_pushstring( state_wrapper.get( ), val );
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
	return lua_pushvfstring( state_wrapper.get( ), fmt, argp );
}

void Interface::PushNumber( double val )
{
	lua_pushnumber( state_wrapper.get( ), val );
}

void Interface::PushInteger( ptrdiff_t val )
{
	lua_pushinteger( state_wrapper.get( ), val );
}

void Interface::PushBoolean( bool val )
{
	lua_pushboolean( state_wrapper.get( ), val );
}

void Interface::PushFunction( Function val )
{
	lua_pushcfunction( state_wrapper.get( ), val );
}

void Interface::PushClosure( Function val, int vars )
{
	lua_pushcclosure( state_wrapper.get( ), val, vars );
}

void Interface::PushLightUserdata( void *val )
{
	lua_pushlightuserdata( state_wrapper.get( ), val );
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
	return luaL_loadbuffer( state_wrapper.get( ), data, size, name );
}

int Interface::LoadString( const char *data )
{
	return luaL_loadstring( state_wrapper.get( ), data );
}

int Interface::LoadFile( const char *path )
{
	return luaL_loadfile( state_wrapper.get( ), path );
}

int Interface::ReferenceCreate( )
{
	return luaL_ref( state_wrapper.get( ), LUA_REGISTRYINDEX );
}

void Interface::ReferenceFree( int ref )
{
	luaL_unref( state_wrapper.get( ), LUA_REGISTRYINDEX, ref );
}

void Interface::ReferencePush( int ref )
{
	RawGetI( LUA_REGISTRYINDEX, ref );
}

const char *Interface::GetTypeName( Type type )
{
	return lua_typename( state_wrapper.get( ), static_cast<int>( type ) );
}

int Interface::GarbageCollect( GC what, int data )
{
	return lua_gc( state_wrapper.get( ), static_cast<int>( what ), data );
}

Type Interface::GetType( int stackpos )
{
	return static_cast<Type>( lua_type( state_wrapper.get( ), stackpos ) );
}

bool Interface::IsType( int stackpos, Type type )
{
	return GetType( stackpos ) == type;
}

void Interface::CheckType( int stackpos, Type type )
{
	luaL_checktype( state_wrapper.get( ), stackpos, static_cast<int>( type ) );
}

Object Interface::ToObject( int stackpos )
{
	return Object( this, stackpos );
}

void Interface::PushObject( const Object &obj )
{
	ReferencePush( obj.reference->Get( ) );
}

int Interface::ThrowError( const char *error )
{
	PushString( error );
	return Error( );
}

static int FunctionDumper( lua_State *state, const void *bchunk, size_t size, void *buffer )
{
	Buffer *lua_buffer = reinterpret_cast<Buffer *>( buffer );
	const char *binary_chunk = reinterpret_cast<const char *>( bchunk );
	GetLuaInterface( state ).BufferAddString( lua_buffer, binary_chunk, size );
	return 0;
}

const char *Interface::Dump( size_t *outlen )
{
	Buffer buffer;
	BufferInit( &buffer );

	if( lua_dump( state_wrapper.get( ), FunctionDumper, &buffer ) == 0 )
		return NULL;

	BufferFinish( &buffer );
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