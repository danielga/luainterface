#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdexcept>

#if defined _MSC_VER

#pragma warning( disable : 4251 )

#define LUAINTERFACE_API_EXPORT __declspec(dllexport)
#define LUAINTERFACE_API_IMPORT __declspec(dllimport)

#elif defined __GCC__

#define LUAINTERFACE_API_EXPORT __attribute__((__visibility__("default")))
#define LUAINTERFACE_API_IMPORT __attribute__((__visibility__("default")))

#else

#error "Unsupported compiler"

#endif

#if defined LUAINTERFACE_STATIC

#define LUAINTERFACE_API

#elif defined LUAINTERFACE_EXPORT

#define LUAINTERFACE_API LUAINTERFACE_API_EXPORT

#else

#define LUAINTERFACE_API LUAINTERFACE_API_IMPORT

#endif

/*!
 \brief Forward declaration of the lua_State struct.
 */
struct lua_State
{
	char *internal_data;
};

namespace Lua
{

/*!
 \brief Defines Lua types.
 */
enum class Type
{
	None = -1,		///< Lua none type (invalid stack index)
	Nil,			///< Lua nil type
	Boolean,		///< Lua boolean type
	LightUserdata,	///< Lua light userdata type
	Number,			///< Lua number type
	String,			///< Lua string type
	Table,			///< Lua table type
	Function,		///< Lua function type
	Userdata,		///< Lua userdata type
	Thread			///< Lua thread type
};

enum class GC
{
	Stop,
	Restart,
	Collect,
	Count,
	CountBytes,
	Step,
	SetPause,
	SetStepMul
};

enum class Status
{
	Success,
	Yield,
	ErrorRun,
	ErrorSyntax,
	ErrorMemory,
	ErrorHandler
};

/*!
 \brief Forward declaration of the Interface class.
 */
class Interface;

class InvalidStackIndex : public std::runtime_error
{
public:
	InvalidStackIndex( Interface &iface, int index ) :
		runtime_error( "Invalid stack index provided!" ),
		lua_interface( &iface ),
		stack_index( index )
	{ }

	Interface *GetLuaInterface( ) const
	{
		return lua_interface;
	}

	int GetStackIndex( ) const
	{
		return stack_index;
	}

private:
	Interface *lua_interface;
	int stack_index;
};

class Panic : public std::runtime_error
{
public:
	Panic( Interface &iface, const char *error ) :
		runtime_error( error ),
		lua_interface( &iface )
	{ }

	Interface *GetLuaInterface( ) const
	{
		return lua_interface;
	}

private:
	Interface *lua_interface;
};

/*!
 \brief Offset of the Lua Interface object from the start
 of the lua_State struct.
 */
extern LUAINTERFACE_API const size_t InterfaceOffset;

/*!
 \brief Tells Lua to push all values returned by a function to
 the stack.
 */
static const int MultipleReturns = -1;

/*!
 \brief Special reference to the Lua value nil.
 */
static const int ReferenceNil = -1;

/*!
 \brief Marks references as invalid.
 */
static const int ReferenceInvalid = -2;

/*!
 \brief Lua C function prototype.
 */
typedef int ( *Function )( lua_State *state );

/*!
 \brief Lua C writer function prototype.
 */
typedef int ( *Writer ) ( lua_State *state, const void *data, size_t size, void *userdata );

/*!
 \brief A struct with functions for a module to be registered with
 State::Register.
 \details Any array of this type must end with a sentinel entry in
 which both name and func are NULL.
 */
struct ModuleFunction
{
	const char *name;	///< name of the function on the table
	Function func;		///< Lua C function to be added
};

/*!
 \brief Maximum buffer size of the luaL_Buffer struct.
 */
static const size_t BufferSize = BUFSIZ;

/*!
 \brief An opaque struct that allows C code to build Lua strings.
 */
struct Buffer;

}

/*!
 \brief Allows direct access to the LuaInterface class
 from the lua_State struct.
 Recommended way to use this is to set a variable at the
 top of the function with the value returned and use it.
 */
inline Lua::Interface &GetLuaInterface( lua_State *state )
{
	return *reinterpret_cast<Lua::Interface *>( &state->internal_data[Lua::InterfaceOffset] );
}
