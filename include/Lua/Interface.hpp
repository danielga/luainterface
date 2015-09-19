#pragma once

#include <Lua/Config.hpp>
#include <Lua/Object.hpp>

namespace Lua
{

class LUAINTERFACE_API Interface
{
public:
	/*!
	 \brief Returns the Lua version as a string.
	 \return string representing the version Lua used internally
	 */
	static const char *Version( );

	/*!
	 \brief Returns the Lua version as an integer.
	 \return integer representing the version Lua used internally
	 */
	static int VersionNumber( );

	/*!
	 \brief Default constructor.
	 */
	Interface( );

	/*!
	 \brief Returns the internal Lua state.
	 \return internal Lua state
	 */
	lua_State *GetLuaState( ) const;

	/*!
	 \brief Returns the index of the top element in
	 the stack.
	 \details Because indices start at 1, this result is equal
	 to the number of elements in the stack (and so 0
	 means an empty stack).
	 \return number of elements on the stack
	 */
	int GetTop( );

	/*!
	 \brief Accepts any acceptable index, or 0, and sets
	 the stack top to this value.
	 \details If the new top is larger than the old one, then the
	 new elements are filled with nil. If index is 0, then
	 all stack elements are removed.
	 \param num new stack top
	 */
	void SetTop( int num );

	/*!
	 \brief Pops elements from the stack.
	 \param amount number of elements to pop from the stack
	 */
	void Pop( int amount = 1 );

	/*!
	 \brief Creates a new empty table and pushes it
	 onto the stack.
	 \details Pre-allocation is useful when you know exactly
	 how many elements the table will have.
	 \param array_elems number of pre-allocated array elements
	 \param nonarray_elems number of pre-allocated non-array
	 elements
	 */
	void CreateTable( int array_elems = 0, int nonarray_elems = 0 );

	/*!
	 \brief Pushes the result of t[k] to the stack.
	 \details t is the value at the given valid index and k is
	 the value at the top of the stack. This function
	 pops the key from the stack (putting the resulting
	 value in its place).
	 As in Lua, this function may trigger a metamethod
	 for the "index" event.
	 \param stackpos stack index of the value
	 */
	void GetTable( int stackpos );

	/*!
	 \brief Does the equivalent to t[k] = v.
	 \details t is the value at the given valid index, v is
	 the value at the top of the stack, and k is the
	 value just below the top. This function pops both
	 the key and the value from the stack.
	 As in Lua, this function may trigger a metamethod
	 for the "newindex" event.
	 \param stackpos stack index of the value
	 */
	void SetTable( int stackpos );

	/*!
	 \brief Pushes the result of t[k] to the stack.
	 \details t is the value at the given valid index.
	 As in Lua, this function may trigger a metamethod
	 for the "index" event.
	 \sa GetTable()
	 \param stackpos stack index of the value
	 */
	void GetField( int stackpos, const char *strName );

	/*!
	 \brief Does the equivalent to t[k] = v.
	 \details t is the value at the given valid index, v is
	 the value at the top of the stack, and k is the
	 value just below the top. This function pops both
	 the key and the value from the stack.
	 As in Lua, this function may trigger a metamethod
	 for the "newindex" event.
	 \sa SetTable()
	 \param stackpos stack index of the value
	 */
	void SetField( int stackpos, const char *strName );

	/*!
	 \brief Pushes a key-value pair from a table.
	 \details Pops a key from the stack, and pushes
	 a key-value pair from the table at the given
	 index (the "next" pair after the given key).
	 \param stackpos position in stack of the table
	 \return 1 if succeeded (pushes key and value onto stack),
	 0 otherwise
	 */
	int Next( int stackpos );

	/*!
	 \brief Pushes onto the stack the value associated with
	 the key name in the registry.
	 \details If it doesn't already exist in the registry,
	 creates a new table and sets it as the value of
	 the key name of the registry and pushes it onto
	 the stack.
	 \param name name of the metatable
	 \return 1 if metatable was created, 0 if it already existed
	 */
	int NewMetatable( const char *name );

	/*!
	 \brief Pushes onto the stack the metatable associated with
	 the key name in the registry.
	 \sa NewMetatable()
	 \param name name of the metatable
	 */
	void GetMetaTable( const char *name );

	/*!
	 \brief Pushes onto the stack the metatable of the
	 value at the given acceptable index.
	 \details If the index is not valid, or if the value does not
	 have a metatable, the function returns 0 and pushes
	 nothing on the stack.
	 \param stackpos stack index of the value
	 */
	int GetMetaTable( int stackpos );

	/*!
	 \brief Pushes onto the stack the field strName
	 from the  metatable of the value at index stackpos.
	 \param stackpos stack index of the value
	 \param strName name of the metatable member
	 \return 1 if value had metatable and member existed on it,
	 0 otherwise
	 */
	int GetMetaTableField( int stackpos, const char *strName );

	/*!
	 \brief Pops a table from the stack and sets it as the
	 new metatable for the value at the given
	 acceptable index.
	 \param stackpos position in stack of the table
	 \return 1 if succeeded, 0 otherwise
	 */
	int SetMetaTable( int stackpos );

	/*!
	 \brief Calls a function.
	 \details To call a function you must use the following
	 protocol: first, the function to be called is
	 pushed onto the stack; then, the arguments to
	 the function are pushed in direct order; that
	 is, the first argument is pushed first. Finally
	 you call Call; args is the number of arguments
	 that you pushed onto the stack. All arguments
	 and the function value are popped from the stack
	 when the function is called. The function results
	 are pushed onto the stack when the function
	 returns. The number of results is adjusted to
	 results, unless results is LUA_MULTRET. In this
	 case, all results from the function are pushed.
	 Lua takes care that the returned values fit into
	 the stack space. The function results are pushed
	 onto the stack in direct order (the first result
	 is pushed first), so that after the call the last
	 result is on the top of the stack. Any error inside
	 the called function is propagated upwards.
	 \param args number of arguments for the function
	 \param results number of results retrieved from the function
	 */
	void Call( int args = 0, int results = 0 );

	/*!
	 \brief Calls a function in protected mode.
	 \details Both args and results have the same meaning as
	 in Call. If there are no errors during the call,
	 PCall behaves exactly like Call. However, if there
	 is any error, PCall catches it, pushes a single
	 value on the stack (the error message), and returns
	 an error code. Like Call, PCall always removes the
	 function and its arguments from the stack.
	 If errorfuncpos is 0, then the error messagereturned
	 on the stack is exactly the original error message.
	 Otherwise, errorfuncpos is the stack index of an
	 error handler function. (In the current
	 implementation, this index cannot be a pseudo-index.)
	 In case of runtime errors, this function will be
	 called with the error message and its return value
	 will be the message returned on the stack by PCall.
	 \sa Call()
	 \param args number of arguments for the function
	 \param results number of results retrieved from the function
	 \param errorfuncpos position of the error callback in the stack
	 \return 1 if no errors were caught, 0 otherwise
	 */
	int PCall( int args = 0, int results = 0, int errorfuncpos = 0 );

	/*!
	 \brief Calls a metamethod.
	 \details If the object at index stackpos has a metatable
	 and  this metatable has a field strName, this function
	 calls this field and passes the object as its only
	 argument.
	 \param stackpos stack index of the value
	 \param strName name of the metatable member
	 \return 1 if succeeded (pushes result onto stack), 0
	 otherwise
	 */
	int CallMeta( int stackpos, const char *strName );

	/*!
	 \brief Pushes onto the stack the user value (a table) of the
	 value at the given index.
	 \param stackpos stack index of the value
	 */
	void GetUserValue( int stackpos );

	/*!
	 \brief Pops a table from the stack and sets it as the new
	 user value for the value at the given index.
	 \details If the value at the given index is not a userdata,
	 returns 0. Otherwise, it returns 1.
	 \param stackpos stack index of the value
	 \return 1 if succeeded, 0 otherwise
	 */
	int SetUserValue( int stackpos );

	/*!
	 \brief Moves the top element into the given valid index.
	 \details After moving the top element to the provided
	 index, it shifts up the elements above this index to
	 open space. Cannot be called with a pseudo-index,
	 because a pseudo-index is not an actual stack position.
	 \param stackpos new position in stack for the value
	 */
	void Insert( int stackpos );

	/*!
	 \brief Removes the element at the given valid index.
	 \details After removing the element at the provided
	 index, it shifts down the other elements to fill the
	 gap. Cannot be called with a pseudo-index, because a
	 pseudo-index is not an actual stack position.
	 \param stackpos stack index of the value
	 */
	void Remove( int stackpos );

	/*!
	 \brief Moves the top element into the given position
	 (and pops it), without shifting any element (therefore
	 replacing the value at the given position).
	 \param stackpos stackpos of the position to replace
	 */
	void Replace( int stackpos );

	/*!
	 \brief Exchange values between different threads of
	 the same global state.
	 \param lua_interface LuaState to push the values to
	 \param n number of values to push and then pop
	 */
	void XMove( Interface &lua_interface, int n );

	/*!
	 \brief Generates a Lua error.
	 \details The error message (which can actually be a
	 Lua value of any type) must be on the stack top.
	 \return this function does a long jump, and therefore
	 never returns
	 */
	int Error( );

	/*!
	 \brief Throws a Lua error.
	 \details This error concerns a certain element in
	 the stack.
	 \param argnum stack index of the value
	 \param message message that provides more insight
	 to the error
	 \return this function does a long jump, and therefore
	 never returns
	 */
	int ArgError( int argnum, const char *message );

	/*!
	 \brief Throws a Lua error.
	 \details This error concerns a certain element in
	 the stack and its type.
	 \param argnum stack index of the value
	 \param type_expected the expected type of the value
	 \return this function does a long jump, and therefore
	 never returns
	 */
	int TypeError( int argnum, const char *type_expected );

	/*!
	 \brief If the function argument stackpos is a string,
	 returns this string.
	 \details If this argument is absent or is nil,
	 returns def. Otherwise, raises an error.
	 \param stackpos position in stack of the argument
	 \param def default string
	 \param outlen the length of the string returned
	 \return string at the given index
	 */
	const char *OptionalString( int stackpos, const char *def, size_t *outlen = 0 );

	/*!
	 \brief If the function argument stackpos is a number,
	 returns this number.
	 \details If this argument is absent or is nil,
	 returns def. Otherwise, raises an error.
	 \param stackpos position in stack of the argument
	 \param def default number
	 \return number at the given index
	 */
	double OptionalNumber( int stackpos, double def );

	/*!
	 \brief If the function argument stackpos is a number,
	 returns this number cast to an integer.
	 \details If this argument is absent or is nil,
	 returns def. Otherwise, raises an error.
	 \param stackpos position in stack of the argument
	 \param def default number
	 \return number at the given index
	 */
	ptrdiff_t OptionalInteger( int stackpos, ptrdiff_t def );

	/*!
	 \brief Checks whether the function argument stackpos
	 is a string and returns this string.
	 \details If outlen is not NULL fills outlen with the
	 string's length. All conversions and caveats of
	 ToString apply here.
	 \sa ToString()
	 \param stackpos position in stack of the argument
	 \param outlen length of the returned string
	 \return string at the given stack position
	 */
	const char *CheckString( int stackpos, size_t *outlen = 0 );

	/*!
	 \brief Checks whether the function argument stackpos
	 is a number and returns this number.
	 \sa ToNumber()
	 \param stackpos position in stack of the argument
	 \return number at the given stack position
	 */
	double CheckNumber( int stackpos );

	/*!
	 \brief Checks whether the function argument stackpos
	 is a number and returns this number cast to integer.
	 \sa ToInteger()
	 \param stackpos position in stack of the argument
	 \return number at the given stack position
	 */
	ptrdiff_t CheckInteger( int stackpos );

	/*!
	 \brief Checks whether the function argument stackpos
	 is a userdata of the type name.
	 \sa NewMetatable()
	 \param stackpos position in stack of the argument
	 \param name userdata type to check with element at
	 the given index
	 */
	void *CheckUserdata( int stackpos, const char *name );

	/*!
	 \brief Checks whether the function has an argument
	 of any type (including nil) at position stackpos.
	 \param stackpos position in stack of the argument
	 */
	void CheckAny( int stackpos );

	/*!
	 \brief Grows the stack size a certain number of elements,
	 raising an error if the stack cannot grow to that size.
	 \details msg is an additional text to go into the error message.
	 \param size number of stack positions to grow
	 \param msg additional text to go into the error message
	 */
	void CheckStack( int size, const char *msg );

	/*!
	 \brief Checks if the two values in acceptable
	 indices stackpos_a and stackpos_b are equal.
	 \details Lua may use metamethods to perform this check.
	 \param stackpos_a position in stack of the first value
	 \param stackpos_b position in stack of the second value
	 \return 1 if values are equal, 0 otherwise (also
	 if any stack position is invalid)
	 */
	int Equal( int stackpos_a, int stackpos_b );

	/*!
	 \brief Checks if the two values in acceptable
	 indices stackpos_a and stackpos_b are primitively
	 equal (that is, without calling metamethods).
	 \sa Equal()
	 \param stackpos_a position in stack of the first value
	 \param stackpos_b position in stack of the second value
	 \return 1 if values are equal, 0 otherwise (also
	 if any stack position is invalid)
	 */
	int RawEqual( int stackpos_a, int stackpos_b );

	/*!
	 \brief Does the equivalent of GetTable but
	 doesn't call metamethods.
	 \sa GetTable()
	 \param stackpos position in stack of the table
	 */
	void RawGet( int stackpos );

	/*!
	 \brief Does the equivalent of GetTable but
	 the index is an integer.
	 \sa RawGet()
	 \param stackpos position in stack of the table
	 \param n index in the table to get the value from
	 */
	void RawGetI( int stackpos, int n );

	/*!
	 \brief Does the equivalent of SetTable but
	 doesn't call metamethods.
	 \sa SetTable()
	 \param stackpos position in stack of the table
	 */
	void RawSet( int stackpos );

	/*!
	 \brief Does the equivalent of RawSet but
	 the index is an integer.
	 \sa RawSet()
	 \param stackpos position in stack of the table
	 \param n index in the table to set the value to
	 */
	void RawSetI( int stackpos, int n );

	/*!
	 \brief Receives a list of C functions and their
	 respective names and registers all of them inside
	 a table.
	 \details If libname is not NULL, the key libname
	 of the global table will be set to this table.
	 Otherwise, it pushes the table onto the stack.
	 \param libname library name
	 \param list list of Lua C functions (must end
	 in a double NULL)
	 */
	void Register( const char *libname, const ModuleFunction *list );

	/*!
	 \brief Converts the Lua value at the given
	 acceptable index to a C string.
	 \details If len is not NULL, it also sets
	 outlen with the string length. The Lua
	 value must be a string or a number;
	 otherwise, the function returns NULL. If
	 the value is a number, then ToString also
	 changes the actual value in the stack to a
	 string. (This change confuses Next when
	 ToString is applied to keys during a table
	 traversal.)
	 \param stackpos stack index of the value
	 \param outlen length of the returned string
	 \return memory block of the userdata
	 */
	const char *ToString( int stackpos, size_t *outlen );

	/*!
	 \brief Converts the Lua value at the given
	 acceptable index to a C string.
	 \details The Lua value must be a string or
	 a number; otherwise, the function returns
	 NULL. If the value is a number, then ToString
	 also  changes the actual value in the stack
	 to a string. (This change confuses Next when
	 ToString is applied to keys during a table
	 traversal.)
	 \param stackpos stack index of the value
	 \return memory block of the userdata
	 \sa ToString()
	 \overload
	 */
	const char *ToString( int stackpos );

	/*!
	 \brief Converts the Lua value at the given
	 acceptable index to the C type double.
	 \details The Lua value must be a number or
	 a string convertible to a number.
	 \param stackpos stack index of the value
	 \return number representation of the value in
	 the stack, otherwise 0
	 */
	double ToNumber( int stackpos );

	/*!
	 \brief Converts the Lua value at the given
	 acceptable index to the signed integral
	 type ptrdiff_t.
	 \details The Lua value must be a number or
	 a string convertible to a number. If the
	 number is not an integer, it is truncated
	 in some non-specified way.
	 \param stackpos stack index of the value
	 \return number representation of the value in
	 the stack, otherwise 0
	 */
	ptrdiff_t ToInteger( int stackpos );

	/*!
	 \brief Converts the Lua value at the given
	 acceptable index to a boolean.
	 \details Like all tests in Lua, ToBoolean
	 returns true for any Lua value different from
	 false and nil; otherwise it returns false. It
	 also returns false when called with a non-valid
	 index. (If you want to accept only actual
	 boolean values, use IsType to test the
	 value's type.)
	 \param stackpos stack index of the value
	 \return boolean representation of the value in
	 the stack, otherwise 0
	 */
	bool ToBoolean( int stackpos );

	/*!
	 \brief Converts a value at the given
	 acceptable index to a C function.
	 \details That value must be a C
	 function; otherwise, returns NULL.
	 \param stackpos stack index of the value
	 \return Lua C function representation of the
	 value in the stack, otherwise NULL
	 */
	Function ToFunction( int stackpos );

	/*!
	 \brief Converts a value at the given
	 acceptable index to a Lua userdata or
	 light userdata.
	 \details If the value is a full userdata,
	 returns its block address. If the value
	 is a light userdata, returns its pointer.
	 Otherwise, returns NULL.
	 \param stackpos stack index of the value
	 \return Lua userdata representation of the value
	 in the stack, otherwise NULL
	 */
	void *ToUserdata( int stackpos );

	/*!
	 \brief Creates a new userdata on the top of the
	 stack.
	 \details This function allocates a new block of
	 memory with the given size, pushes onto the
	 stack a new full userdata with the block
	 address, and returns this address.
	 \param size the size of the memory block
	 \return memory block of the userdata
	 */
	void *NewUserdata( size_t size );

	/*!
	 \brief Creates a new thread, pushes it on the stack,
	 and returns a pointer to a lua_State that represents
	 this new thread.
	 \details The new state returned by this function
	 shares with the original state all global objects
	 (such as tables), but has an independent execution
	 stack.
	 There is no explicit function to close or to destroy
	 a thread. Threads are subject to garbage collection,
	 like any Lua object.
	 \return lua_State representing the new thread
	 */
	lua_State *NewThread( );

	/*!
	 \brief Yields a coroutine.
	 \details This function should only be called as the
	 return expression of a C function, as follows:
	 return State::Yield( L, nresults );
	 When a C function calls Yield in that way, the running
	 coroutine suspends its execution, and the call to Resume
	 that started this coroutine returns. The parameter
	 results is the number of values from the stack that are
	 passed as results to Resume.
	 \param results number of results to pass to Resume
	 \return status code
	 */
	int Yield( int results );

	/*!
	 \brief Starts and resumes a coroutine in a given thread.
	 \details To start a coroutine, you first create a new
	 thread (see NewThread); then you push onto its stack the
	 main function plus any arguments; then you call Resume,
	 with args being the number of arguments. This call returns
	 when the coroutine suspends or finishes its execution.
	 When it returns, the stack contains all values passed to
	 Yield, or all values returned by the body function. Resume
	 returns Status::Yield if the coroutine yields,
	 Status::Success if the coroutine finishes its execution
	 without errors, or an error code in case of errors (see
	 PCall). In case of errors, the stack is not unwound, so
	 you can use the debug API over it. The error message is
	 on the top of the stack. To restart a coroutine, you put
	 on its stack only the values to be passed as results from
	 Yield, and then call Resume.
	 \param args number of arguments to pop from stack
	 \return status code
	 */
	int Resume( int args );

	/*!
	 \brief Returns the status of the thread.
	 \details The status can be 0 for a normal thread, an error
	 code if the thread finished its execution with an error,
	 or Status::Yield if the thread is suspended.
	 \return status code
	 */
	int Status( );

	/*!
	 \brief Initializes a buffer.
	 \details This function does not allocate
	 any space; the buffer must be declared as
	 a variable.
	 \param buffer Buffer struct declared
	 by the user
	 */
	void BufferInit( Buffer *buffer );

	/*!
	 \brief Pushes all previous changes done to the
	 buffer onto the stack and returns a pointer to
	 a char array of size LUAL_BUFFERSIZE to be
	 modified by the user.
	 \details You must call BufferAddSize with the
	 size of the data you wrote to the array.
	 \param buffer Buffer struct previously
	 initialized
	 \return pointer to a char array of size
	 LUAL_BUFFERSIZE
	 */
	char *BufferPrepare( Buffer *buffer );

	/*!
	 \brief Adds to the buffer a string of length size,
	 previously copied to the buffer area.
	 \param buffer Buffer struct previously
	 initialized
	 \param size size of the string previously copied
	 */
	void BufferAddSize( Buffer *buffer, size_t size );

	/*!
	 \brief Adds the character ch to the buffer.
	 \param buffer Buffer struct previously
	 initialized
	 \param ch character to be added to the buffer
	 */
	void BufferAddChar( Buffer *buffer, const char ch );

	/*!
	 \brief Adds the string pointed to by str with
	 length len to the buffer.
	 \details The string may contain embedded zeros.
	 \param buffer Buffer struct previously
	 initialized
	 \param str string to be copied to the buffer
	 \param len length of the string
	 */
	void BufferAddString( Buffer *buffer, const char *str, size_t len );

	/*!
	 \brief Adds the string pointed to by str to the buffer.
	 \details The string may not contain embedded zeros.
	 \param buffer Buffer struct previously
	 initialized
	 \param str string to be copied to the buffer
	 \sa BufferAddString()
	 \overload
	 */
	void BufferAddString( Buffer *buffer, const char *str );

	/*!
	 \brief Adds the value at the top of the stack
	 to the buffer and pops it.
	 \param buffer Buffer struct previously
	 initialized
	 */
	void BufferAdd( Buffer *buffer );

	/*!
	 \brief Finishes the use of the buffer leaving
	 the final string on the top of the stack.
	 \param buffer Buffer struct previously
	 initialized
	 */
	void BufferFinish( Buffer *buffer );

	/*!
	 \brief Creates a copy of the string by replacing
	 any occurrence of the pattern with the replacement.
	 \details Pushes the resulting string on the stack
	 and returns it.
	 \param str string to modify/copy
	 \param pattern pattern of the substring to be modified
	 \param replacement replacement for each match
	 \return modified string if successful
	 */
	const char *GSub( const char *str, const char *pattern, const char *replacement );

	/*!
	 \brief Pushes a copy of the element at the
	 given valid index onto the stack.
	 \param stackpos stack index of the value
	 */
	void PushValue( int stackpos );

	/*!
	 \brief Pushes a nil value onto the stack.
	 */
	void PushNil( );

	/*!
	 \brief Pushes the string pointed to by val
	 with size len onto the stack.
	 \details Lua makes (or reuses) an internal
	 copy of the given string, so the memory at
	 val can be freed or reused immediately after
	 the function returns. The string can contain
	 embedded zeros.
	 \param val string to be pushed
	 \param len length of the string to be pushed
	 */
	void PushString( const char *val, size_t len );

	/*!
	 \brief Pushes the string pointed to by val
	 onto the stack.
	 \details Lua makes (or reuses) an internal
	 copy of the given string, so the memory at
	 val can be freed or reused immediately after
	 the function returns. The string can not contain
	 embedded zeros. Lua  automatically calculates
	 the string length using strlen or equivalent and
	 the string can't have embedded zeros.
	 \param val string to be pushed
	 */
	void PushString( const char *val );

	/*!
	 \brief Pushes onto the stack a formatted string and
	 returns a pointer to this string.
	 \details It is similar to the C function sprintf, but
	 has some important differences.
	 You do not have to allocate space for the result: the
	 result is a Lua string and Lua takes care of memory
	 allocation (and deallocation, through garbage collection).
	 The conversion specifiers are quite restricted. There
	 are no flags, widths, or precisions. The conversion
	 specifiers can only be '%%' (inserts a '%' in the string),
	 '%s' (inserts a zero-terminated string, with no size
	 restrictions), '%f' (inserts a lua_Number), '%p' (inserts
	 a pointer as a hexadecimal numeral), '%d' (inserts an
	 int), and '%c' (inserts an int as a character).
	 Receives a variable number of parameters to format the
	 string with.
	 \param fmt describes how the string is formatted
	 \return formatted string, if no error occurred
	 */
	const char *PushFormattedString( const char *fmt, ... );

	/*!
	 \brief Pushes onto the stack a formatted string and
	 returns a pointer to this string.
	 \details It is similar to the C function sprintf, but
	 has some important differences.
	 You do not have to allocate space for the result: the
	 result is a Lua string and Lua takes care of memory
	 allocation (and deallocation, through garbage collection).
	 The conversion specifiers are quite restricted. There
	 are no flags, widths, or precisions. The conversion
	 specifiers can only be '%%' (inserts a '%' in the string),
	 '%s' (inserts a zero-terminated string, with no size
	 restrictions), '%f' (inserts a lua_Number), '%p' (inserts
	 a pointer as a hexadecimal numeral), '%d' (inserts an
	 int), and '%c' (inserts an int as a character).
	 Only differs from the first method on the parameters it
	 receives. It uses a va_list to format the string instead
	 of a variable number of parameters.
	 \param fmt describes how the string is formatted
	 \return formatted string, if no error occurred
	 \sa PushFormattedString()
	 \overload
	 */
	const char *PushFormattedString( const char *fmt, va_list argp );

	/*!
	 \brief Pushes a number with value val onto
	 the stack.
	 \param val number to be pushed
	 */
	void PushNumber( double val );

	/*!
	 \brief Pushes a number with value val onto
	 the stack.
	 \param val number to be pushed
	 */
	void PushInteger( ptrdiff_t val );

	/*!
	 \brief Pushes a boolean with value val onto
	 the stack.
	 \param val boolean to be pushed
	 */
	void PushBoolean( bool val );

	/*!
	 \brief Pushes a C function onto the stack.
	 \details Any function to be registered in
	 Lua must follow the correct protocol to
	 receive its parameters and return its
	 results.
	 \sa Function
	 \param val function to be pushed
	 */
	void PushFunction( Function val );

	/*!
	 \brief Pushes a new C closure onto the stack.
	 \details When a C function is created, it is
	 possible to associate some values with it,
	 thus creating a C closure; these values are
	 then accessible to the function whenever it
	 is called. To associate values with a C
	 function, first these values should be pushed
	 onto the stack (when there are multiple
	 values, the first value is pushed first).
	 Then PushClosure is called to create and
	 push the C function onto the stack, with the
	 argument vars telling how many values should
	 be associated with the function. PushClosure
	 also pops these values from the stack.
	 The maximum value for vars is 255.
	 \sa Function
	 \sa PushFunction()
	 \param val function to be pushed
	 \param vars number of elements on the stack to
	 be associated with the function
	 */
	void PushClosure( Function val, int vars );

	/*!
	 \brief Pushes a light userdata onto the stack.
	 \details Userdata represent C values in Lua. A
	 light userdata represents a pointer. It is a
	 value (like a number): you do not create it,
	 it has no individual metatable, and it is not
	 collected (as it was never created). A light
	 userdata is equal to "any" light userdata with
	 the same C address.
	 \param val C pointer to be pushed
	 */
	void PushLightUserdata( void *val );

	/*!
	 \brief Pushes the thread onto the stack.
	 \param thread thread to push onto the stack
	 \return 1 if this thread is the main thread of its state
	 */
	int PushThread( lua_State *thread );

	/*!
	 \brief Pushes the global table onto the stack.
	 */
	void PushGlobal( );

	/*!
	 \brief Pushes the registry table onto the stack.
	 */
	void PushRegistry( );

	/*!
	 \brief Loads a buffer as a Lua chunk.
	 \param data buffer to be run as a Lua chunk
	 \param size size in bytes of the buffer
	 \param name name of the Lua chunk
	 */
	int LoadBuffer( const char *data, size_t size, const char *name );

	/*!
	 \brief Loads a zero-terminated string
	 as a Lua chunk.
	 \param data Lua chunk
	 */
	int LoadString( const char *data );

	/*!
	 \brief Loads a file as a Lua chunk.
	 \details If filename is NULL, then
	 it loads from the standard input.
	 \param path path to the file
	 */
	int LoadFile( const char *path );

	/*!
	 \brief Creates and returns a reference.
	 \details If the object at the top of the
	 stack is nil, ReferenceCreate returns the
	 constant LUA_REFNIL. The constant
	 LUA_NOREF is guaranteed to be different
	 from any reference returned by
	 ReferenceCreate.
	 \sa LUA_REFNIL
	 \sa LUA_NOREF
	 \return unique integer key used to reference the
	 value
	 */
	int ReferenceCreate( );

	/*!
	 \brief Releases the reference.
	 \details The reference ref is also freed
	 to be used again. If ref is LUA_NOREF or
	 LUA_REFNIL, ReferenceFree does nothing.
	 \sa ReferenceCreate()
	 \param ref unique integer key used to reference
	 the value
	 */
	void ReferenceFree( int ref );

	/*!
	 \brief Pushes a reference onto the stack.
	 \sa ReferenceCreate()
	 \param ref reference to be pushed
	 */
	void ReferencePush( int ref );

	/*!
	 \brief Returns the name of the type encoded
	 by the value type, which must be one the
	 values returned by GetType.
	 \sa GetType()
	 \param type type from which to retrieve the name
	 \return name of the Lua type
	 */
	const char *GetTypeName( Type type );

	/*!
	 \brief Controls the garbage collector.
	 \details This function performs several tasks,
	 according to the value of the parameter what.
	 \param what requested operation to execute on the gc
	 \param data parameter to provide along the operation
	 \return data returned by the executed operation, if any
	 */
	int GarbageCollect( GC what, int data = 0 );

	/*!
	 \brief Returns the type of the value in the
	 given acceptable index, or Type::None for
	 a non-valid index (that is, an index to an
	 "empty" stack position).
	 \sa Type
	 \param stackpos stack index of the value
	 \return Lua type of the value
	 */
	Type GetType( int stackpos );

	/*!
	 \brief Checks for equality between the type
	 of the element at the given stack index and
	 the provided type.
	 \sa Type
	 \sa GetType()
	 \param stackpos stack index of the value
	 \param type type to check with
	 \return true if both types match, false otherwise
	 */
	bool IsType( int stackpos, Type type );

	/*!
	 \brief Checks whether the function argument stackpos
	 has type t.
	 \sa Type
	 \param stackpos position in stack of the argument
	 \param t type to check with element at the given index
	 */
	void CheckType( int stackpos, Type t );

	/*!
	 \brief Converts the Lua value at the given
	 acceptable index to a C++ object.
	 \details The returned Lua object is valid as long as at
	 least one C++ object is alive.
	 \param stackpos stack index of the value
	 \return object representation of the value in
	 the stack, otherwise an invalid C++ object
	 */
	Object ToObject( int stackpos = -1 );

	/*!
	 \brief Pushes a a Lua object onto the stack.
	 \param val object to be pushed
	 */
	void PushObject( const Object &obj );

	/*!
	 \brief Throws a Lua error.
	 \param error a string detailing the error
	 \return this function does a long jump, and therefore
	 never returns
	 */
	int ThrowError( const char *error );

	void SCall( )
	{
		Call( pushed_scall );
		pushed_scall = 0;
	}

	template<typename ...ArgsTypes> void SCall( std::nullptr_t, ArgsTypes... args )
	{
		PushNil( );
		++pushed_scall;
		return SCall( args );
	}

	template<typename ...ArgsTypes> void SCall( const char *arg, ArgsTypes... args )
	{
		PushString( arg );
		++pushed_scall;
		return SCall( args );
	}

	template<typename ...ArgsTypes> void SCall( double arg, ArgsTypes... args )
	{
		PushNumber( arg );
		++pushed_scall;
		return SCall( args );
	}

	template<typename ...ArgsTypes> void SCall( ptrdiff_t arg, ArgsTypes... args )
	{
		PushInteger( arg );
		++pushed_scall;
		return SCall( args );
	}

	template<typename ...ArgsTypes> void SCall( bool arg, ArgsTypes... args )
	{
		PushBoolean( arg );
		++pushed_scall;
		return SCall( args );
	}

	template<typename ...ArgsTypes> void SCall( Function arg, ArgsTypes... args )
	{
		PushFunction( arg );
		++pushed_scall;
		return SCall( args );
	}

	template<typename ...ArgsTypes> void SCall( lua_State *arg, ArgsTypes... args )
	{
		PushThread( arg );
		++pushed_scall;
		return SCall( args );
	}

	template<typename ...ArgsTypes> void SCall( const Object &arg, ArgsTypes... args )
	{
		PushObject( arg );
		++pushed_scall;
		return SCall( args );
	}

	/*!
	 \brief Dumps a function as a binary chunk.
	 \details Receives a Lua function on the top
	 of the stack and produces a binary chunk
	 that, if loaded again, results in a function
	 equivalent to the one dumped.
	 \param outlen length of the returned binary chunk
	 \return binary chunk representing the Lua function
	 */
	const char *Dump( size_t *outlen );

	/*!
	 \brief Loads and runs a buffer as a Lua chunk.
	 \details
	 \sa LoadBuffer()
	 \param data buffer to be run as a Lua chunk
	 \param size size in bytes of the buffer
	 \param name name of the Lua chunk
	 \param pcall run in protected mode
	 \return
	 */
	virtual bool RunBuffer( const char *data, size_t size, const char *name, bool pcall = true );

	/*!
	 \brief Loads and runs a string as a Lua chunk.
	 \details
	 \sa LoadString()
	 \param data string to be run
	 \param pcall run in protected mode
	 \return
	 */
	virtual bool RunString( const char *data, bool pcall = true );

	/*!
	 \brief Loads and runs a file as a Lua chunk.
	 \sa LoadFile()
	 \param path path to the file to be run
	 \param pcall run in protected mode
	 \return true if it ran successfully, false otherwise
	 (can also create a Lua error if ran in unprotected mode)
	 */
	virtual bool RunFile( const char *path, bool pcall = true );

private:
	friend static void UserstateCreated( lua_State *state );

	/*!
	 \brief Constructor.
	 \deprecated It's not recommended to create Lua states
	 independently then wrapping them with a LuaInterface.
	 This is mostly for internal usage.
	 */
	Interface( lua_State *state );

	std::shared_ptr<lua_State> state_wrapper;

	int pushed_scall;
};

}
