extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
}

#include "LuaInterface.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef LoadString
#else
#include <pthread.h>
#endif
#include <exception>

#define THREAD_TYPE "cthread"

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

	LuaThread( ) : thread( 0 ), references( 1 ), state( PENDING )
	{
		LuaInterface *lua_interface = new LuaInterface;

#if defined _WIN32
		if( ( thread = CreateThread( 0, 0, &LuaThread::Callback, this, 0, 0 ) ) == 0 )
		{
			delete lua_interface;
			throw std::exception( );
		}
#elif defined __APPLE__ || defined __linux
		if( pthread_create( &thread, NULL, thread_callback, wrapper ) != 0 )
		{
			delete lua_interface;
			throw std::exception( );
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
		LuaThread *thread = (LuaThread *)userdata;
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

	LuaInterface *lua_interface;
	volatile State state;
	volatile int references;
#if defined _WIN32
	HANDLE thread;
#elif defined __APPLE__ || defined __linux
	pthread_t thread;
#endif
};

LUA_FUNCTION( thread_create )
{
	LUA->CheckType( 1, LUATYPE_STRING );

	LuaThread *thread = new LuaThread;

	const char *code = LUA->ToString( 1 );
	if( thread->lua_interface->LoadString( code ) != 0 )
	{
		LUA->PushString( thread->lua_interface->ToString( -1 ) );
		delete thread;
		return LUA->Error( );
	}

	LuaThread **userdata = (LuaThread **)LUA->NewUserdata( sizeof( LuaThread * ) );
	*userdata = thread;

	LUA->NewMetatable( THREAD_TYPE );
	LUA->SetMetaTable( -2 );

	return 1;
}

LUA_FUNCTION( thread_destroy )
{
	LUA->CheckUserdata( 1, THREAD_TYPE );

	LuaThread **userdata = (LuaThread **)LUA->ToUserdata( 1 );
	LuaThread *thread = *userdata;
	if( thread == 0 )
		return LUA->ArgError( 1, "invalid " THREAD_TYPE " object" );

	thread->Release( );
	*userdata = 0;

	return 1;
}

LUA_FUNCTION( thread_sleep )
{
	LUA->CheckType( 1, LUATYPE_NUMBER );

	unsigned int time = (unsigned int)LUA->ToNumber( 1 );

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

LUA_FUNCTION( thread_join )
{
	LUA->CheckUserdata( 1, THREAD_TYPE );

	LuaThread *thread = *(LuaThread **)LUA->ToUserdata( 1 );
	if( thread == 0 )
		return LUA->ArgError( 1, "invalid " THREAD_TYPE " object" );

	thread->Join( );

	return 0;
}

LUA_FUNCTION( thread_detach )
{
	LUA->CheckUserdata( 1, THREAD_TYPE );

	LuaThread *thread = *(LuaThread **)LUA->ToUserdata( 1 );
	if( thread == 0 )
		return LUA->ArgError( 1, "invalid " THREAD_TYPE " object" );

	thread->Detach( );

	return 0;
}

LUA_FUNCTION( thread_get )
{
	LUA->CheckUserdata( 1, THREAD_TYPE );
	LUA->CheckAny( 2 );

	LuaThread *thread = *(LuaThread **)LUA->ToUserdata( 1 );
	if( thread == 0 )
		return LUA->ArgError( 1, "invalid " THREAD_TYPE " object" );



	return 0;
}

LUA_FUNCTION( thread_set )
{
	LUA->CheckUserdata( 1, THREAD_TYPE );
	LUA->CheckAny( 2 );
	LUA->CheckAny( 3 );

	LuaThread *thread = *(LuaThread **)LUA->ToUserdata( 1 );
	if( thread == 0 )
		return LUA->ArgError( 1, "invalid " THREAD_TYPE " object" );



	return 0;
}

LUA_MODULE_LOAD( )
{
	LUA->CreateTable( );

	LUA->PushCFunction( thread_create );
	LUA->SetField( -2, "create" );

	LUA->PushCFunction( thread_sleep );
	LUA->SetField( -2, "sleep" );

	LUA->NewMetatable( THREAD_TYPE );

	LUA->CreateTable( );	// __metatable value

	LUA->PushCFunction( thread_join );
	LUA->SetField( -2, "join" );

	LUA->PushCFunction( thread_detach );
	LUA->SetField( -2, "detach" );

	LUA->PushCFunction( thread_get );
	LUA->SetField( -2, "get" );

	LUA->PushCFunction( thread_set );
	LUA->SetField( -2, "set" );

	LUA->PushCFunction( thread_destroy );
	LUA->SetField( -2, "__gc" );

	LUA->SetField( -2, "__metatable" );

	return 1;
}

LUA_MODULE_UNLOAD( )
{
	return 0;
}