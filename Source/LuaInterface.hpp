/** @file */

#ifndef LUAINTERFACE_HPP
#define LUAINTERFACE_HPP

#ifdef _WIN32
	#pragma once
#endif

#include <stdint.h>

#if defined _MSC_VER
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

/**
* @brief Allows direct access to the LuaInterface class
* from the lua_State struct.
*/
#define LUA ( state->lua_interface )

/**
* @brief Declares Lua functions in a easy way.
*/
#define LUA_FUNCTION( name ) static int name( lua_State *state )

/**
* @brief Name of the Lua module load function.
*/
#define LUA_MODULE_LOAD_NAME LuaInterfaceModuleLoad

/**
* @brief Name of the Lua module unload function.
*/
#define LUA_MODULE_UNLOAD_NAME LuaInterfaceModuleUnload

/**
* @brief Declares the Lua module load function.
*/
#define LUA_MODULE_LOAD( ) extern "C" LUAINTERFACE_API_EXPORT int LUA_MODULE_LOAD_NAME( lua_State *state )

/**
* @brief Declares the Lua module unload function.
*/
#define LUA_MODULE_UNLOAD( ) extern "C" LUAINTERFACE_API_EXPORT int LUA_MODULE_UNLOAD_NAME( lua_State *state )

/**
* @brief Forward declaration of the lua_State struct.
* @details Shouldn't be touched directly.
*/
struct lua_State
{
	char __common_lua_data__[48];
	class LuaInterface *lua_interface;
};

#ifndef LUA_VERSION
/**
* @brief Tells Lua to push all values returned by a function to
* the stack.
*/
#define LUA_MULTRET ( -1 )

/**
* @brief Special reference to the Lua value nil.
*/
#define LUA_REFNIL ( -1 )

/**
* @brief Marks references as invalid.
*/
#define LUA_NOREF ( -2 )

/**
* @brief Lua C function type.
*/
typedef int ( *lua_CFunction )( lua_State *state );

/**
* @brief A struct for functions to be registered with OpenLibrary.
*/
struct luaL_Reg
{
	const char *name;	///< name of the function on the table
	lua_CFunction func;	///< Lua C function to be added
};

/**
* @brief Maximum buffer size of the luaL_Buffer struct.
*/
#define LUAL_BUFFERSIZE 512

/**
* @brief A struct that allows C code to build Lua strings.
*/
struct luaL_Buffer
{
	char *p;						///< current position in buffer
	int lvl;						///< number of strings in the stack (level)
	lua_State *L;					///< lua_State where strings are pushed
	char buffer[LUAL_BUFFERSIZE];	///< buffer for intermediate manipulations
};
#endif

/**
* @brief Defines Lua pseudo-indices.
*/
enum LuaPseudoIndex
{
	LUA_GLOBAL,		///< Lua global table
	LUA_REGISTRY	///< Lua registry
};

/**
* @brief Defines Lua types.
*/
enum LuaType
{
	LUATYPE_NONE = -1,		///< Lua none type (invalid stack index)
	LUATYPE_NIL,			///< Lua nil type
	LUATYPE_BOOLEAN,		///< Lua boolean type
	LUATYPE_LIGHTUSERDATA,	///< Lua light userdata type
	LUATYPE_NUMBER,			///< Lua number type
	LUATYPE_STRING,			///< Lua string type
	LUATYPE_TABLE,			///< Lua table type
	LUATYPE_FUNCTION,		///< Lua function type
	LUATYPE_USERDATA,		///< Lua userdata type
	LUATYPE_THREAD			///< Lua thread type
};

/**
* @brief A C++ lua_State struct wrapper.
* @details Allows usage of the Lua C API in a traditional
* C++ way by using objects.
*/
class LUAINTERFACE_API LuaInterface
{
public:
	/**
	* @brief Constructor of the LuaInterface class.
	* @param state instance of the lua_State struct
	* (instead of the constructor creating a new one)
	*/
	LuaInterface( lua_State *state = 0 );
	/**
	* @brief Virtual destructor of the LuaInterface class.
	*/
	virtual ~LuaInterface( );

	/**
	* @brief Returns the index of the top element in
	* the stack.
	* @details Because indices start at 1, this result is equal
	* to the number of elements in the stack (and so 0
	* means an empty stack). 
	* @return number of elements on the stack
	*/
	virtual int GetTop( );
	/**
	* @brief Accepts any acceptable index, or 0, and sets
	* the stack top to this value.
	* @details If the new top is larger than the old one, then the
	* new elements are filled with nil. If index is 0, then
	* all stack elements are removed. 
	* @param num new stack top
	*/
	virtual void SetTop( int num );
	/**
	* @brief Pops elements from the stack.
	* @param amount number of elements to pop from the stack
	*/
	virtual void Pop( int amount = 1 );

	/**
	* @brief Creates a new empty table and pushes it
	* onto the stack.
	* @details Pre-allocation is useful when you know exactly
	* how many elements the table will have.
	* @param array_elems number of pre-allocated array elements
	* @param nonarray_elems number of pre-allocated non-array
	* elements
	*/
	virtual void CreateTable( int array_elems = 0, int nonarray_elems = 0 );
	/**
	* @brief Pushes the result of t[k] to the stack.
	* @details t is the value at the given valid index and k is
	* the value at the top of the stack. This function
	* pops the key from the stack (putting the resulting
	* value in its place).
	* As in Lua, this function may trigger a metamethod
	* for the "index" event.
	* @param stackpos stack index of the value
	*/
	virtual void GetTable( int stackpos );
	/**
	* @brief Does the equivalent to t[k] = v.
	* @details t is the value at the given valid index, v is
	* the value at the top of the stack, and k is the
	* value just below the top. This function pops both
	* the key and the value from the stack.
	* As in Lua, this function may trigger a metamethod
	* for the "newindex" event.
	* @param stackpos stack index of the value
	*/
	virtual void SetTable( int stackpos );
	/**
	* @brief Pushes the result of t[k] to the stack.
	* @details t is the value at the given valid index.
	* As in Lua, this function may trigger a metamethod
	* for the "index" event.
	* @see GetTable()
	* @param stackpos stack index of the value
	*/
	virtual void GetField( int stackpos, const char *strName );
	/**
	* @brief Does the equivalent to t[k] = v.
	* @details t is the value at the given valid index, v is
	* the value at the top of the stack, and k is the
	* value just below the top. This function pops both
	* the key and the value from the stack.
	* As in Lua, this function may trigger a metamethod
	* for the "newindex" event.
	* @see SetTable()
	* @param stackpos stack index of the value
	*/
	virtual void SetField( int stackpos, const char *strName );
	/**
	* @brief Pushes a key-value pair from a table.
	* @details Pops a key from the stack, and pushes
	* a key-value pair from the table at the given
	* index (the "next" pair after the given key).
	* @param stackpos position in stack of the table
	* @return 1 if succeeded (pushes key and value onto stack),
	* 0 otherwise
	*/
	virtual int Next( int stackpos );

	/**
	* @brief Pushes onto the stack the value associated with
	* the key name in the registry.
	* @details If it doesn't already exist in the registry,
	* creates a new table and sets it as the value of
	* the key name of the registry and pushes it onto
	* the stack.
	* @param name name of the metatable
	* @return 1 if metatable was created, 0 if it already existed
	*/
	virtual int NewMetatable( const char *name );
	/**
	* @brief Pushes onto the stack the metatable associated with
	* the key name in the registry.
	* @see NewMetatable()
	* @param name name of the metatable
	*/
	virtual void GetMetaTable( const char *name );
	/**
	* @brief Pushes onto the stack the metatable of the
	* value at the given acceptable index.
	* @details If the index is not valid, or if the value does not
	* have a metatable, the function returns 0 and pushes
	* nothing on the stack.
	* @param stackpos stack index of the value
	*/
	virtual int GetMetaTable( int stackpos );
	/**
	* @brief Pushes onto the stack the field strName
	* from the  metatable of the value at index stackpos.
	* @param stackpos stack index of the value
	* @param strName name of the metatable member
	* @return 1 if value had metatable and member existed on it,
	* 0 otherwise
	*/
	virtual int GetMetaTableField( int stackpos, const char *strName );
	/**
	* @brief Pops a table from the stack and sets it as the
	* new metatable for the value at the given
	* acceptable index. 
	* @param stackpos position in stack of the table
	* @return 1 if succeeded, 0 otherwise
	*/
	virtual int SetMetaTable( int stackpos );

	/**
	* @brief Calls a function.
	* @details To call a function you must use the following
	* protocol: first, the function to be called is
	* pushed onto the stack; then, the arguments to
	* the function are pushed in direct order; that
	* is, the first argument is pushed first. Finally
	* you call Call; args is the number of arguments
	* that you pushed onto the stack. All arguments
	* and the function value are popped from the stack
	* when the function is called. The function results
	* are pushed onto the stack when the function
	* returns. The number of results is adjusted to
	* results, unless results is LUA_MULTRET. In this
	* case, all results from the function are pushed.
	* Lua takes care that the returned values fit into
	* the stack space. The function results are pushed
	* onto the stack in direct order (the first result
	* is pushed first), so that after the call the last
	* result is on the top of the stack. Any error inside
	* the called function is propagated upwards.
	* @param args number of arguments for the function
	* @param results number of results retrieved from the function
	*/
	virtual void Call( int args = 0, int results = 0 );
	/**
	* @brief Calls a function in protected mode.
	* @details Both args and results have the same meaning as
	* in Call. If there are no errors during the call,
	* PCall behaves exactly like Call. However, if there
	* is any error, PCall catches it, pushes a single
	* value on the stack (the error message), and returns
	* an error code. Like Call, PCall always removes the
	* function and its arguments from the stack.
	* If errorfuncpos is 0, then the error messagereturned
	* on the stack is exactly the original error message.
	* Otherwise, errorfuncpos is the stack index of an
	* error handler function. (In the current
	* implementation, this index cannot be a pseudo-index.)
	* In case of runtime errors, this function will be
	* called with the error message and its return value
	* will be the message returned on the stack by PCall. 
	* @see Call()
	* @param args number of arguments for the function
	* @param results number of results retrieved from the function
	* @param errorfuncpos position of the error callback in the stack
	* @return 1 if no errors were caught, 0 otherwise
	*/
	virtual int PCall( int args = 0, int results = 0, int errorfuncpos = 0 );
	/**
	* @brief Calls a metamethod.
	* @details If the object at index stackpos has a metatable
	* and  this metatable has a field strName, this function
	* calls this field and passes the object as its only
	* argument.
	* @param stackpos stack index of the value
	* @param strName name of the metatable member
	* @return 1 if succeeded (pushes result onto stack), 0
	* otherwise
	*/
	virtual int CallMeta( int stackpos, const char *strName );

	virtual void GetFEnv( int stackpos );
	virtual int SetFEnv( int stackpos );

	/**
	* @brief Moves the top element into the given valid index.
	* @details After moving the top element to the provided
	* index, it shifts up the elements above this index to
	* open space. Cannot be called with a pseudo-index,
	* because a pseudo-index is not an actual stack position. 
	* @param stackpos new position in stack for the value
	*/
	virtual void Insert( int stackpos );
	/**
	* @brief Removes the element at the given valid index.
	* @details After removing the element at the provided
	* index, it shifts down the other elements to fill the
	* gap. Cannot be called with a pseudo-index, because a
	* pseudo-index is not an actual stack position.
	* @param stackpos stack index of the value
	*/
	virtual void Remove( int stackpos );
	/**
	* @brief Exchange values between different threads of
	* the same global state.
	* @param lua_interface LuaInterface to push the values to
	* @param n number of values to push and then pop
	*/
	virtual void XMove( LuaInterface *lua_interface, int n );

	/**
	* @brief Generates a Lua error.
	* @details The error message (which can actually be a
	* Lua value of any type) must be on the stack top.
	* @return this function does a long jump, and therefore
	* never returns
	*/
	virtual int Error( );
	/**
	* @brief Throws a Lua error.
	* @param error a string detailing the error
	* @return this function does a long jump, and therefore
	* never returns
	*/
	virtual int ThrowError( const char *error );
	/**
	* @brief Throws a Lua error.
	* @details This error concerns a certain element in
	* the stack.
	* @param argnum stack index of the value
	* @param message message that provides more insight
	* to the error
	* @return this function does a long jump, and therefore
	* never returns
	*/
	virtual int ArgError( int argnum, const char *message );
	/**
	* @brief Throws a Lua error.
	* @details This error concerns a certain element in
	* the stack and its type.
	* @param argnum stack index of the value
	* @param type_expected the expected type of the value
	* @return this function does a long jump, and therefore
	* never returns
	*/
	virtual int TypeError( int argnum, const char *type_expected );

	/**
	* @brief If the function argument stackpos is a string,
	* returns this string.
	* @details If this argument is absent or is nil,
	* returns def. Otherwise, raises an error.
	* @param stackpos position in stack of the argument
	* @param def default string
	* @param outlen the length of the string returned
	* @return string at the given index
	*/
	virtual const char *OptionalString( int stackpos, const char *def, size_t *outlen = 0 );
	/**
	* @brief If the function argument stackpos is a number,
	* returns this number.
	* @details If this argument is absent or is nil,
	* returns def. Otherwise, raises an error. 
	* @param stackpos position in stack of the argument
	* @param def default number
	* @return number at the given index
	*/
	virtual double OptionalNumber( int stackpos, double def );
	/**
	* @brief If the function argument stackpos is a number,
	* returns this number cast to an integer.
	* @details If this argument is absent or is nil,
	* returns def. Otherwise, raises an error.
	* @param stackpos position in stack of the argument
	* @param def default number
	* @return number at the given index
	*/
	virtual ptrdiff_t OptionalInteger( int stackpos, ptrdiff_t def );

	/**
	* @brief Checks whether the function argument stackpos
	* is a string and returns this string.
	* @details If outlen is not NULL fills outlen with the
	* string's length. All conversions and caveats of
	* ToString apply here.
	* @see ToString()
	* @param stackpos position in stack of the argument
	* @param outlen length of the returned string
	* @return string at the given stack position
	*/
	virtual const char *CheckString( int stackpos, size_t *outlen = 0 );
	/**
	* @brief Checks whether the function argument stackpos
	* is a number and returns this number.
	* @see ToNumber()
	* @param stackpos position in stack of the argument
	* @return number at the given stack position
	*/
	virtual double CheckNumber( int stackpos );
	/**
	* @brief Checks whether the function argument stackpos
	* is a number and returns this number cast to integer.
	* @see ToInteger()
	* @param stackpos position in stack of the argument
	* @return number at the given stack position
	*/
	virtual ptrdiff_t CheckInteger( int stackpos );

	/**
	* @brief Checks whether the function argument stackpos
	* has type t.
	* @see LuaType
	* @param stackpos position in stack of the argument
	* @param t type to check with element at the given index
	*/
	virtual void CheckType( int stackpos, int t );
	/**
	* @brief Checks whether the function argument stackpos
	* is a userdata of the type name.
	* @see NewMetatable()
	* @param stackpos position in stack of the argument
	* @param name userdata type to check with element at
	* the given index
	*/
	virtual void *CheckUserdata( int stackpos, const char *name );
	/**
	* @brief Checks whether the function has an argument
	* of any type (including nil) at position stackpos. 
	* @param stackpos position in stack of the argument
	*/
	virtual void CheckAny( int stackpos );

	/**
	* @brief Checks if the two values in acceptable
	* indices stackpos_a and stackpos_b are equal.
	* @details Lua may use metamethods to perform this check.
	* @param stackpos_a position in stack of the first value
	* @param stackpos_b position in stack of the second value
	* @return 1 if values are equal, 0 otherwise (also
	* if any stack position is invalid)
	*/
	virtual int Equal( int stackpos_a, int stackpos_b );
	/**
	* @brief Checks if the two values in acceptable
	* indices stackpos_a and stackpos_b are primitively
	* equal (that is, without calling metamethods).
	* @see Equal()
	* @param stackpos_a position in stack of the first value
	* @param stackpos_b position in stack of the second value
	* @return 1 if values are equal, 0 otherwise (also
	* if any stack position is invalid)
	*/
	virtual int RawEqual( int stackpos_a, int stackpos_b );

	/**
	* @brief Does the equivalent of GetTable but
	* doesn't call metamethods.
	* @see GetTable()
	* @param stackpos position in stack of the table
	*/
	virtual void RawGet( int stackpos );
	/**
	* @brief Does the equivalent of SetTable but
	* doesn't call metamethods.
	* @see SetTable()
	* @param stackpos position in stack of the table
	*/
	virtual void RawSet( int stackpos );

	/**
	* @brief Receives a list of C functions and their
	* respective names and registers all of them inside
	* a table.
	* @details If libname is not NULL, the key libname
	* of the global table will be set to this table.
	* Otherwise, it pushes the table onto the stack.
	* @param libname library name
	* @param list list of Lua C functions (must end
	* in a double NULL)
	* @param nup number of upvalues
	*/
	virtual void OpenLibrary( const char *libname, const luaL_Reg *list, int nup = 0 );

	/**
	* @brief Converts the Lua value at the given
	* acceptable index to a C string.
	* @details If len is not NULL, it also sets
	* outlen with the string length. The Lua
	* value must be a string or a number;
	* otherwise, the function returns NULL. If
	* the value is a number, then ToString also
	* changes the actual value in the stack to a
	* string. (This change confuses Next when
	* ToString is applied to keys during a table
	* traversal.)
	* @param stackpos stack index of the value
	* @param outlen length of the returned string
	* @return memory block of the userdata
	*/
	virtual const char *ToString( int stackpos = -1, size_t *outlen = 0 );
	/**
	* @brief Converts the Lua value at the given
	* acceptable index to the C type double.
	* @details The Lua value must be a number or
	* a string convertible to a number.
	* @param stackpos stack index of the value
	* @return number representation of the value in
	* the stack, otherwise 0
	*/
	virtual double ToNumber( int stackpos = -1 );
	/**
	* @brief Converts the Lua value at the given
	* acceptable index to the signed integral
	* type ptrdiff_t.
	* @details The Lua value must be a number or
	* a string convertible to a number. If the
	* number is not an integer, it is truncated
	* in some non-specified way. 
	* @param stackpos stack index of the value
	* @return number representation of the value in
	* the stack, otherwise 0
	*/
	virtual ptrdiff_t ToInteger( int stackpos = -1 );
	/**
	* @brief Converts the Lua value at the given
	* acceptable index to a boolean.
	* @details Like all tests in Lua, ToBoolean
	* returns true for any Lua value different from
	* false and nil; otherwise it returns false. It
	* also returns false when called with a non-valid
	* index. (If you want to accept only actual
	* boolean values, use IsType to test the
	* value's type.)
	* @param stackpos stack index of the value
	* @return boolean representation of the value in
	* the stack, otherwise 0
	*/
	virtual bool ToBoolean( int stackpos = -1 );
	/**
	* @brief Converts a value at the given
	* acceptable index to a C function.
	* @details That value must be a C
	* function; otherwise, returns NULL.
	* @param stackpos stack index of the value
	* @return Lua C function representation of the
	* value in the stack, otherwise NULL
	*/
	virtual lua_CFunction ToCFunction( int stackpos = -1 );
	/**
	* @brief Converts a value at the given
	* acceptable index to a Lua userdata or
	* light userdata.
	* @details If the value is a full userdata,
	* returns its block address. If the value
	* is a light userdata, returns its pointer.
	* Otherwise, returns NULL.
	* @param stackpos stack index of the value
	* @return Lua userdata representation of the value
	* in the stack, otherwise NULL
	*/
	virtual void *ToUserdata( int stackpos = -1 );

	/**
	* @brief Creates a new userdata on the top of the
	* stack.
	* @details This function allocates a new block of
	* memory with the given size, pushes onto the
	* stack a new full userdata with the block
	* address, and returns this address. 
	* @param size the size of the memory block
	* @return memory block of the userdata
	*/
	virtual void *NewUserdata( size_t size );

	/**
	* @brief Initializes a buffer.
	* @details This function does not allocate
	* any space; the buffer must be declared as
	* a variable.
	* @param buffer luaL_Buffer struct declared
	* by the user
	*/
	virtual void BufferInit( luaL_Buffer *buffer );
	/**
	* @brief Pushes all previous changes done to the
	* buffer onto the stack and returns a pointer to
	* a char array of size LUAL_BUFFERSIZE to be
	* modified by the user.
	* @details You must call BufferAddSize with the
	* size of the data you wrote to the array.
	* @param buffer luaL_Buffer struct previously
	* initialized
	* @return pointer to a char array of size
	* LUAL_BUFFERSIZE
	*/
	virtual char *BufferPrepare( luaL_Buffer *buffer );
	/**
	* @brief Adds to the buffer a string of length size, 
	* previously copied to the buffer area.
	* @param buffer luaL_Buffer struct previously
	* initialized
	* @param size size of the string previously copied
	*/
	virtual void BufferAddSize( luaL_Buffer *buffer, size_t size );
	/**
	* @brief Adds the character ch to the buffer.
	* @param buffer luaL_Buffer struct previously
	* initialized
	* @param ch character to be added to the buffer
	*/
	virtual void BufferAddChar( luaL_Buffer *buffer, const char ch );
	/**
	* @brief Adds the string pointed to by str with
	* length len to the buffer.
	* @details The string may contain embedded zeros.
	* @param buffer luaL_Buffer struct previously
	* initialized
	* @param str string to be copied to the buffer
	* @param len length of the string (0 to let Lua
	* get it automatically)
	*/
	virtual void BufferAddString( luaL_Buffer *buffer, const char *str, size_t len = 0 );
	/**
	* @brief Adds the value at the top of the stack
	* to the buffer and pops it.
	* @param buffer luaL_Buffer struct previously
	* initialized
	*/
	virtual void BufferAdd( luaL_Buffer *buffer );
	/**
	* @brief Finishes the use of the buffer leaving
	* the final string on the top of the stack.
	* @param buffer luaL_Buffer struct previously
	* initialized
	*/
	virtual void BufferFinish( luaL_Buffer *buffer );

	/**
	* @brief Pushes a copy of the element at the
	* given valid index onto the stack.
	* @param stackpos stack index of the value
	*/
	virtual void PushValue( int stackpos );
	/**
	* @brief Pushes a nil value onto the stack.
	*/
	virtual void PushNil( );
	/**
	* @brief Pushes the string pointed to by val
	* with size len onto the stack.
	* @details Lua makes (or reuses) an internal
	* copy of the given string, so the memory at
	* val can be freed or reused immediately after
	* the function returns. The string can contain
	* embedded zeros. If len is 0, Lua
	* automatically calculates the string length
	* using strlen or equivalent and the string
	* can't have embedded zeros.
	* @param val string to be pushed
	* @param len length of the string to be pushed
	*/
	virtual void PushString( const char *val, size_t len = 0 );
	/**
	* @brief Pushes a number with value val onto
	* the stack.
	* @param val number to be pushed
	*/
	virtual void PushNumber( double val );
	/**
	* @brief Pushes a number with value val onto
	* the stack.
	* @param val number to be pushed
	*/
	virtual void PushInteger( ptrdiff_t val );
	/**
	* @brief Pushes a boolean with value val onto
	* the stack.
	* @param val boolean to be pushed
	*/
	virtual void PushBoolean( bool val );
	/**
	* @brief Pushes a C function onto the stack.
	* @details Any function to be registered in
	* Lua must follow the correct protocol to
	* receive its parameters and return its
	* results.
	* @see lua_CFunction
	* @param val function to be pushed
	*/
	virtual void PushCFunction( lua_CFunction val );
	/**
	* @brief Pushes a new C closure onto the stack.
	* @details When a C function is created, it is
	* possible to associate some values with it,
	* thus creating a C closure; these values are
	* then accessible to the function whenever it
	* is called. To associate values with a C
	* function, first these values should be pushed
	* onto the stack (when there are multiple
	* values, the first value is pushed first).
	* Then PushCClosure is called to create and
	* push the C function onto the stack, with the
	* argument vars telling how many values should
	* be associated with the function. PushCClosure
	* also pops these values from the stack.
	* The maximum value for vars is 255. 
	* @see lua_CFunction
	* @see PushCFunction()
	* @param val function to be pushed
	* @param vars number of elements on the stack to
	* be associated with the function
	*/
	virtual void PushCClosure( lua_CFunction val, int vars );
	/**
	* @brief Pushes a light userdata onto the stack.
	* @details Userdata represent C values in Lua. A
	* light userdata represents a pointer. It is a
	* value (like a number): you do not create it,
	* it has no individual metatable, and it is not
	* collected (as it was never created). A light
	* userdata is equal to "any" light userdata with
	* the same C address.
	* @param val C pointer to be pushed
	*/
	virtual void PushLightUserdata( void *val );

	/**
	* @brief Pushes a pseudo-index onto the stack.
	* @param index pseudo-index
	*/
	virtual void PushPseudoIndex( LuaPseudoIndex index );

	/**
	* @brief Loads a buffer as a Lua chunk.
	* @param data buffer to be run as a Lua chunk
	* @param size size in bytes of the buffer
	* @param name name of the Lua chunk
	*/
	virtual int LoadBuffer( const char *data, size_t size, const char *name );
	/**
	* @brief Loads a zero-terminated string
	* as a Lua chunk.
	* @param data Lua chunk
	*/
	virtual int LoadString( const char *data );
	/**
	* @brief Loads a file as a Lua chunk.
	* @details If filename is NULL, then
	* it loads from the standard input.
	* @param path path to the file
	*/
	virtual int LoadFile( const char *path );

	/**
	* @brief Creates and returns a reference.
	* @details If the object at the top of the
	* stack is nil, ReferenceCreate returns the
	* constant LUA_REFNIL. The constant
	* LUA_NOREF is guaranteed to be different
	* from any reference returned by
	* ReferenceCreate.
	* @see LUA_REFNIL
	* @see LUA_NOREF
	* @return unique integer key used to reference the
	* value
	*/
	virtual int ReferenceCreate( );
	/**
	* @brief Releases the reference.
	* @details The reference ref is also freed
	* to be used again. If ref is LUA_NOREF or
	* LUA_REFNIL, ReferenceFree does nothing.
	* @see ReferenceCreate()
	* @param ref unique integer key used to reference
	* the value
	*/
	virtual void ReferenceFree( int ref );
	/**
	* @brief Pushes a reference onto the stack.
	* @see ReferenceCreate()
	* @param ref reference to be pushed
	*/
	virtual void ReferencePush( int ref );

	/**
	* @brief Returns the type of the value in the
	* given acceptable index, or LUATYPE_NONE for
	* a non-valid index (that is, an index to an
	* "empty" stack position).
	* @see LuaType
	* @param stackpos stack index of the value
	* @return Lua type of the value
	*/
	virtual int GetType( int stackpos );
	/**
	* @brief Checks for equality between the type
	* of the element at the given stack index and
	* the provided type.
	* @see LuaType
	* @see GetType()
	* @param stackpos stack index of the value
	* @param type type to check with
	* @return true if both types match, false otherwise
	*/
	virtual bool IsType( int stackpos, int type );
	/**
	* @brief Returns the name of the type encoded
	* by the value type, which must be one the
	* values returned by GetType.
	* @see GetType()
	* @param type type from which to retrieve the name
	* @return name of the Lua type
	*/
	virtual const char *GetTypeName( int type );

	/**
	* @brief Dumps a function as a binary chunk.
	* @details Receives a Lua function on the top
	* of the stack and produces a binary chunk
	* that, if loaded again, results in a function
	* equivalent to the one dumped.
	* @return outlen size of the binary chunk
	* @return binary chunk that represents the function
	*/
	virtual const char *Dump( size_t *outlen = 0 );

	/**
	* @brief Loads and runs a buffer as a Lua chunk.
	* @details 
	* @see LoadBuffer()
	* @param data buffer to be run as a Lua chunk
	* @param size size in bytes of the buffer
	* @param name name of the Lua chunk
	* @param pcall run in protected mode
	* @return 
	*/
	virtual bool RunBuffer( const char *data, size_t size, const char *name, bool pcall = true );
	/**
	* @brief Loads and runs a string as a Lua chunk.
	* @details 
	* @see LoadString()
	* @param data string to be run
	* @param pcall run in protected mode
	* @return 
	*/
	virtual bool RunString( const char *data, bool pcall = true );
	/**
	* @brief Loads and runs a file as a Lua chunk.
	* @see LoadFile()
	* @param path path to the file to be run
	* @param pcall run in protected mode
	* @return true if it ran successfully, false otherwise
	* (can also create a Lua error if ran in unprotected mode)
	*/
	virtual bool RunFile( const char *path, bool pcall = true );

protected:
	lua_State *lua_state;
	bool manual;
};

#endif
