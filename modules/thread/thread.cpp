#include <Lua/Interface.hpp>

#if defined _WIN32

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#undef LoadString

#else

#include <pthread.h>

#endif

#include <stdexcept>

static const char metaname[] = "cthread";
static const char invalid_object[] = "invalid cthread object";

class LuaThread
{
public:
	enum State
	{
		PENDING,
		RUNNING,
		FINISHED,
		JOINING,
		JOINED,
		DETACHING,
		DETACHED
	};

	LuaThread( ) :
		lua_interface( new Lua::Interface ),
		thread( 0 ),
		references( 1 ),
		state( PENDING )
	{

#if defined _WIN32

		if( ( thread = CreateThread( nullptr, 0, &LuaThread::Callback, this, 0, nullptr ) ) == nullptr )
		{
			delete lua_interface;
			throw std::runtime_error( "Unable create new thread!" );
		}

#elif defined __APPLE__ || defined __linux

		if( pthread_create( &thread, nullptr, thread_callback, wrapper ) != 0 )
		{
			delete lua_interface;
			throw std::runtime_error( "Unable create new thread!" );
		}

#endif

	}

	~LuaThread( )
	{
		if( !Join( ) )
			Detach( );

		delete lua_interface;
	}

	void Acquire( )
	{
		++references;
	}

	void Release( )
	{
		if( --references <= 0 )
			delete this;
	}

	bool Join( )
	{

#if defined _WIN32

		if( GetCurrentThread( ) != thread && ( state == RUNNING || state == FINISHED ) )
		{
			state = JOINING;
			WaitForSingleObject( thread, INFINITE );
			CloseHandle( thread );
			state = JOINED;
			return true;
		}

#elif defined __APPLE__ || defined __linux

		if( pthread_equal( pthread_self( ), thread ) == 0 && ( state == RUNNING || state == FINISHED ) )
		{
			state = JOINING;
			pthread_join( thread, 0 );
			state = JOINED;
			return true;
		}

#endif

		return false;
	}

	bool Detach( )
	{
		if( state == RUNNING )
		{
			state = DETACHING;

#if defined _WIN32

			CloseHandle( thread );

#elif defined __APPLE__ || defined __linux

			pthread_detach( thread );

#endif

			state = DETACHED;
			return true;
		}

		return false;
	}

#if defined _WIN32

	static DWORD WINAPI Callback( void *userdata )

#elif defined __APPLE__ || defined __linux

	static void *Callback( void *userdata )

#endif

	{
		LuaThread *thread = static_cast<LuaThread *>( userdata );
		thread->state = RUNNING;
		thread->Acquire( );

		if( thread->lua_interface->PCall( 0, 0, 0 ) != 0 )
		{
			// get error
		}

		thread->Release( );
		thread->state = FINISHED;
		return 0;
	}

	Lua::Interface *lua_interface;
	volatile State state;
	volatile int references;

#if defined _WIN32

	HANDLE thread;

#elif defined __APPLE__ || defined __linux

	pthread_t thread;

#endif

};

static int thread_create( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckType( 1, Lua::Type::String );

	LuaThread *thread = new LuaThread;

	const char *code = lua.ToString( 1 );
	if( thread->lua_interface->LoadString( code ) != 0 )
	{
		lua.PushString( thread->lua_interface->ToString( -1 ) );
		delete thread;
		return lua.Error( );
	}

	LuaThread **userdata = lua.NewUserdata<LuaThread *>( sizeof( LuaThread * ) );
	*userdata = thread;

	lua.NewMetatable( metaname );
	lua.SetMetaTable( -2 );

	return 1;
}

static int thread_destroy( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, metaname );

	LuaThread **userdata = lua.ToUserdata<LuaThread *>( 1 );
	LuaThread *thread = *userdata;
	if( thread == nullptr )
		return lua.ArgError( 1, invalid_object );

	thread->Release( );
	*userdata = nullptr;

	return 1;
}

static int thread_sleep( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckType( 1, Lua::Type::Number );

	uint32_t time = static_cast<uint32_t>( lua.ToNumber( 1 ) );

#if defined _WIN32

	Sleep( time );

#elif defined __APPLE__ || defined __linux

	timespec newtime;
	newtime.tv_sec = time / 1000;
	newtime.tv_nsec = ( time % 1000 ) * 1000000;
	clock_nanosleep( CLOCK_MONOTONIC, 0, &newtime, 0 );

#endif

	return 0;
}

static int thread_join( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, metaname );

	LuaThread *thread = *lua.ToUserdata<LuaThread *>( 1 );
	if( thread == nullptr )
		return lua.ArgError( 1, invalid_object );

	thread->Join( );

	return 0;
}

static int thread_detach( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, metaname );

	LuaThread *thread = *lua.ToUserdata<LuaThread *>( 1 );
	if( thread == nullptr )
		return lua.ArgError( 1, invalid_object );

	thread->Detach( );

	return 0;
}

static int thread_get( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, metaname );
	lua.CheckAny( 2 );

	LuaThread *thread = *lua.ToUserdata<LuaThread *>( 1 );
	if( thread == nullptr )
		return lua.ArgError( 1, invalid_object );



	return 0;
}

static int thread_set( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, metaname );
	lua.CheckAny( 2 );
	lua.CheckAny( 3 );

	LuaThread *thread = *lua.ToUserdata<LuaThread *>( 1 );
	if( thread == nullptr )
		return lua.ArgError( 1, invalid_object );



	return 0;
}

extern "C" int luaopen_thread( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );

	lua.CreateTable( );

	lua.PushFunction( thread_create );
	lua.SetField( -2, "create" );

	lua.PushFunction( thread_sleep );
	lua.SetField( -2, "sleep" );

	lua.NewMetatable( metaname );

	lua.CreateTable( );	// __metatable value

	lua.PushFunction( thread_join );
	lua.SetField( -2, "join" );

	lua.PushFunction( thread_detach );
	lua.SetField( -2, "detach" );

	lua.PushFunction( thread_get );
	lua.SetField( -2, "get" );

	lua.PushFunction( thread_set );
	lua.SetField( -2, "set" );

	lua.PushFunction( thread_destroy );
	lua.SetField( -2, "__gc" );

	lua.SetField( -2, "__metatable" );

	return 1;
}
