#pragma once

#include <Lua/Config.hpp>
#include <atomic>

namespace Lua
{

class Interface;

class Reference
{
public:
	Reference( Interface *lua );

	~Reference( );

	bool IsValid( ) const;

	int Get( ) const;

private:
	Interface *lua_state;
	std::atomic<int> reference;
};

}