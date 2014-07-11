#pragma once

namespace Lua
{

class Interface;

namespace Internal
{

void SetLuaInterface( struct lua_State *state, Interface *iface );

}

}