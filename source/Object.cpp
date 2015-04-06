#include <Lua/Object.hpp>
#include <Lua/Interface.hpp>
#include <Reference.hpp>
#include <atomic>

namespace Lua
{

Object::Object( Interface *state, int index )
{
	state->PushValue( index );
	reference = std::make_shared<Reference>( state );
}

}