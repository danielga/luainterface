#include <lua.hpp>

#ifdef LUAJIT_VERSION

extern "C"
{
	#include <lj_obj.h>
}

#define _STDINT

#else

#include <lstate.h>

#endif

#include <Internal.hpp>

#if defined _MSC_VER

#define EXPORT __declspec(dllexport)

#elif defined __GCC__

#define EXPORT __attribute__((__visibility__("default")))

#else

#define EXPORT

#endif

namespace Lua
{

extern EXPORT const size_t InterfaceOffset = offsetof( lua_State, lua_interface );

namespace Internal
{

void SetLuaInterface( lua_State *state, Interface *iface )
{
	state->lua_interface = iface;
}

}

}
