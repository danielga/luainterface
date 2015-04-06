#pragma once

#include <Lua/Config.hpp>
#include <memory>

namespace Lua
{

class Interface;
class Reference;

class LUAINTERFACE_API Object
{
public:


private:
	Object( Interface *state, int index );

	std::shared_ptr<Reference> reference;

	friend Interface;
};

}