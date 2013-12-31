#include <LuaInterface.hpp>
#include <malloc.h>
#include <string.h>
#include <stdexcept>

#if _WIN32
#define snprintf _snprintf
#endif

#define BYTEBUFFER_META "bytebuffer"

class ByteBuffer
{
public:
	ByteBuffer( ) : buffer_pointer( 0 ), buffer_size( 0 ), buffer_capacity( 0 ), buffer_offset( 0 ) { }

	ByteBuffer( size_t size ) : buffer_pointer( 0 ), buffer_size( 0 ), buffer_capacity( 0 ), buffer_offset( 0 )
	{
		Resize( size );
	}

	ByteBuffer( const unsigned char *copy_buffer, size_t size ) : buffer_pointer( 0 ), buffer_size( 0 ), buffer_capacity( 0 ), buffer_offset( 0 )
	{
		Assign( copy_buffer, size );
	}

	~ByteBuffer( )
	{
		Clear( );
	}

	// This doesn't work with arrays, pointers, classes, structs, etc
	// Only use single value types
	template<typename T> void Read( T &value )
	{
		ReadInternal( (void *)&value, sizeof( T ) );
	}

	// This is meant to be used with arrays
	template<typename T, size_t size> void Read( T ( &value )[size] )
	{
		ReadInternal( (void *)&value, size * sizeof( T ) );
	}

	// This is meant to be used with pointers to arrays
	template<typename T> void Read( T *value, size_t size )
	{
		ReadInternal( (void *)value, size * sizeof( T ) );
	}

	// This doesn't work with arrays, pointers, classes, structs, etc
	// Only use single value types
	template<typename T> void Write( const T &value )
	{
		WriteInternal( (const void *)&value, sizeof( T ) );
	}

	// This is meant to be used with arrays
	template<typename T, size_t size> void Write( const T ( &value )[size] )
	{
		WriteInternal( (const void *)&value, size * sizeof( T ) );
	}

	// This is meant to be used with pointers to arrays
	template<typename T> void Write( const T *value, size_t size )
	{
		WriteInternal( (const void *)value, size * sizeof( T ) );
	}

	size_t StringLength( )
	{
		if( !IsBufferValid( ) ) throw std::runtime_error( "trying to get string length from unexistent buffer" );

		return strnlen( (const char *)&buffer_pointer[buffer_offset], buffer_size - buffer_offset );
	}

	size_t Tell( ) const
	{
		return buffer_offset;
	}

	size_t Size( ) const
	{
		return buffer_size;
	}

	size_t Capacity( ) const
	{
		return buffer_capacity;
	}

	void Seek( size_t position )
	{
		if( position >= buffer_size ) throw std::out_of_range( "requested position is out of range" );

		buffer_offset = position;
	}

	bool EOB( ) const
	{
		return buffer_offset == buffer_size;
	}

	bool IsBufferValid( ) const
	{
		return buffer_pointer != 0;
	}

	unsigned char *GetBuffer( )
	{
		return buffer_pointer;
	}

	const unsigned char *GetBuffer( ) const
	{
		return buffer_pointer;
	}

	void Clear( )
	{
		if( !IsBufferValid( ) ) return;

		free( buffer_pointer );
		buffer_pointer = 0;
		buffer_size = 0;
		buffer_capacity = 0;
		buffer_offset = 0;
	}

	void Reserve( size_t capacity )
	{
		if( capacity <= buffer_capacity ) return;

		unsigned char *new_buffer = (unsigned char *)realloc( buffer_pointer, capacity );
		if( new_buffer == 0 ) throw std::bad_alloc( );
		buffer_pointer = new_buffer;
		buffer_capacity = capacity;
	}

	void Resize( size_t size )
	{
		Reserve( size );
		buffer_size = size;
		if( buffer_offset >= size ) buffer_offset = size == 0 ? 0 : size - 1;
	}

	void Assign( const unsigned char *copy_buffer, size_t size )
	{
		if( size > buffer_capacity )
		{
			unsigned char *new_buffer = (unsigned char *)malloc( size );
			if( new_buffer == 0 ) throw std::bad_alloc( );
			if( IsBufferValid( ) ) free( buffer_pointer );
			buffer_pointer = new_buffer;
			buffer_capacity = size;
		}

		buffer_size = size;
		buffer_offset = 0;
		memcpy( buffer_pointer, copy_buffer, size );
	}

	static bool IsLittleEndian( )
	{
		static const int _endian_test = 1;
		return *( (char *)&_endian_test ) != 0;
	}

	template<typename T> static void SwapByteEndian( T &value )
	{
		unsigned char *bytes = (unsigned char *)&value;
		unsigned char temp;
		for( size_t i = 0; i < sizeof( T ) / 2; i++ )
		{
			temp = bytes[i];
			bytes[i] = bytes[sizeof( T ) - 1 - i];
			bytes[sizeof( T ) - 1 - i] = temp;
		}
	}

private:
	void ReadInternal( void *value, size_t size )
	{
		if( !IsBufferValid( ) ) throw std::runtime_error( "trying to read from unexistent buffer" );
		if( value == 0 || size == 0 ) throw std::invalid_argument( "invalid data recipient or size to read" );
		if( buffer_offset + size >= buffer_size ) throw std::out_of_range( "requested read size overflows buffer" );

		memcpy( value, &buffer_pointer[buffer_offset], size );
		buffer_offset += size;
	}

	void WriteInternal( const void *value, size_t size )
	{
		if( value == 0 || size == 0 ) throw std::invalid_argument( "invalid data or size to write" );
		if( buffer_offset + size > buffer_size ) Resize( buffer_offset + size );

		memcpy( &buffer_pointer[buffer_offset], value, size );
		buffer_offset += size;
	}

	unsigned char *buffer_pointer;
	size_t buffer_size;
	size_t buffer_capacity;
	size_t buffer_offset;
};

LUA_FUNCTION( bytebuffer_new )
{
	ByteBuffer *buffer = reinterpret_cast<ByteBuffer *>( LUA->NewUserdata( sizeof( ByteBuffer ) ) );
	if( buffer == 0 ) LUA->ThrowError( "failed to create new " BYTEBUFFER_META );
	new( buffer ) ByteBuffer( );

	LUA->NewMetatable( BYTEBUFFER_META );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	LUA->SetFEnv( -2 );

	if( LUA->GetType( 1 ) == LUATYPE_STRING )
	{
		size_t size = 0;
		const unsigned char *data = (const unsigned char *)LUA->ToString( 1, &size );
		buffer->Assign( data, size );
	}

	return 1;
}

LUA_FUNCTION( bytebuffer_delete )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->~ByteBuffer( );
	return 0;
}

LUA_FUNCTION( bytebuffer_tostring )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	char msg[30];
	snprintf( msg, sizeof( msg ), "%s: 0x%p", BYTEBUFFER_META, reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) ) );
	LUA->PushString( msg );
	return 1;
}

LUA_FUNCTION( bytebuffer_index )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );

	LUA->GetFEnv( 1 );
	LUA->PushValue( 2 );
	LUA->RawGet( -2 );
	if( LUA->GetType( -1 ) > LUATYPE_NIL )
	{
		return 1;
	}
	LUA->Pop( 2 );

	LUA->GetMetaTable( 1 );
	LUA->PushValue( 2 );
	LUA->RawGet( -2 );
	return 1;
}

LUA_FUNCTION( bytebuffer_newindex )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->GetFEnv( 1 );
	LUA->PushValue( 2 );
	LUA->PushValue( 3 );
	LUA->RawSet( -3 );
	return 0;
}

LUA_FUNCTION( bytebuffer_readinteger )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_NUMBER );

	ByteBuffer *buffer = reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) );
	if( LUA->IsType( 3, LUATYPE_BOOLEAN ) && LUA->ToBoolean( 3 ) )
	{
		switch( (int)LUA->ToNumber( 2 ) )
		{
			case 1:
			{
				unsigned char number;
				buffer->Read( number );
				LUA->PushNumber( (double)number );
				break;
			}

			case 2:
			{
				unsigned short number;
				buffer->Read( number );
				LUA->PushNumber( (double)number );
				break;
			}

			case 4:
			{
				unsigned int number;
				buffer->Read( number );
				LUA->PushNumber( (double)number );
				break;
			}

			case 8:
			{
				unsigned long long number;
				buffer->Read( number );
				LUA->PushNumber( (double)number );
				break;
			}

			default:
			{
				return LUA->ArgError( 2, "requested number of bytes to read not implemented (only 1, 2, 4 and 8 can be used)" );
			}
		}
	}
	else
	{
		switch( (int)LUA->ToNumber( 2 ) )
		{
			case 1:
			{
				char number;
				buffer->Read( number );
				LUA->PushNumber( (double)number );
				break;
			}

			case 2:
			{
				short number;
				buffer->Read( number );
				LUA->PushNumber( (double)number );
				break;
			}

			case 4:
			{
				int number;
				buffer->Read( number );
				LUA->PushNumber( (double)number );
				break;
			}

			case 8:
			{
				long long number;
				buffer->Read( number );
				LUA->PushNumber( (double)number );
				break;
			}

			default:
			{
				return LUA->ArgError( 2, "requested number of bytes to read not implemented (only 1, 2, 4 and 8 can be used)" );
			}
		}
	}
	
	return 1;
}

LUA_FUNCTION( bytebuffer_readfloat )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	float number;
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Read( number );
	LUA->PushNumber( (double)number );
	return 1;
}

LUA_FUNCTION( bytebuffer_readdouble )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	double number;
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Read( number );
	LUA->PushNumber( number );
	return 1;
}

LUA_FUNCTION( bytebuffer_readbool )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	bool number;
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Read( number );
	LUA->PushBoolean( number );
	return 1;
}

LUA_FUNCTION( bytebuffer_readstring )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	ByteBuffer *buffer = reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) );
	size_t len = buffer->StringLength( );
	std::string str;
	str.reserve( len );
	char *data = (char *)str.c_str( );
	buffer->Read( data, len );
	LUA->PushString( str.c_str( ), len );
	return 1;
}

LUA_FUNCTION( bytebuffer_writeinteger )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_NUMBER );
	LUA->CheckType( 3, LUATYPE_NUMBER );

	ByteBuffer *buffer = reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) );
	if( LUA->IsType( 4, LUATYPE_BOOLEAN ) && LUA->ToBoolean( 4 ) )
	{
		switch( (int)LUA->ToNumber( 3 ) )
		{
			case 1:
			{
				buffer->Write( (const unsigned char)LUA->ToNumber( 2 ) );
				break;
			}

			case 2:
			{
				buffer->Write( (const unsigned short)LUA->ToNumber( 2 ) );
				break;
			}

			case 4:
			{
				buffer->Write( (const unsigned int)LUA->ToNumber( 2 ) );
				break;
			}

			case 8:
			{
				buffer->Write( (const unsigned long long)LUA->ToNumber( 2 ) );
				break;
			}

			default:
			{
				return LUA->ArgError( 3, "requested number of bytes to read not implemented (only 1, 2, 4 and 8 can be used)" );
			}
		}
	}
	else
	{
		switch( (int)LUA->ToNumber( 3 ) )
		{
			case 1:
			{
				buffer->Write( (const char)LUA->ToNumber( 2 ) );
				break;
			}

			case 2:
			{
				buffer->Write( (const short)LUA->ToNumber( 2 ) );
				break;
			}

			case 4:
			{
				buffer->Write( (const int)LUA->ToNumber( 2 ) );
				break;
			}

			case 8:
			{
				buffer->Write( (const long long)LUA->ToNumber( 2 ) );
				break;
			}

			default:
			{
				return LUA->ArgError( 3, "requested number of bytes to read not implemented (only 1, 2, 4 and 8 can be used)" );
			}
		}
	}
	
	return 0;
}

LUA_FUNCTION( bytebuffer_writefloat )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_NUMBER );
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Write( (float)LUA->ToNumber( 2 ) );
	return 0;
}

LUA_FUNCTION( bytebuffer_writedouble )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_NUMBER );
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Write( LUA->ToNumber( 2 ) );
	return 0;
}

LUA_FUNCTION( bytebuffer_writebool )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_BOOLEAN );
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Write( LUA->ToBoolean( 2 ) == 1 );
	return 0;
}

LUA_FUNCTION( bytebuffer_writestring )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_STRING );
	size_t len = 0;
	const char *str = LUA->ToString( 2, &len );
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Write( str, len + 1 );
	return 0;
}

LUA_FUNCTION( bytebuffer_assign )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_STRING );
	size_t len = 0;
	const unsigned char *data = (const unsigned char *)LUA->ToString( 2, &len );
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Assign( data, len );
	return 0;
}

LUA_FUNCTION( bytebuffer_stringlength )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->PushNumber( (double)reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->StringLength( ) );
	return 1;
}

LUA_FUNCTION( bytebuffer_reserve )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_NUMBER );
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Reserve( (size_t)LUA->ToNumber( 2 ) );
	return 0;
}

LUA_FUNCTION( bytebuffer_resize )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_STRING );
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Resize( (size_t)LUA->ToNumber( 2 ) );
	return 0;
}

LUA_FUNCTION( bytebuffer_clear )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Clear( );
	return 0;
}

LUA_FUNCTION( bytebuffer_tell )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->PushNumber( (double)reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Tell( ) );
	return 1;
}

LUA_FUNCTION( bytebuffer_size )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->PushNumber( (double)reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Size( ) );
	return 1;
}

LUA_FUNCTION( bytebuffer_capacity )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->PushNumber( (double)reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Capacity( ) );
	return 1;
}

LUA_FUNCTION( bytebuffer_seek )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_NUMBER );
	reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->Seek( (size_t)LUA->ToNumber( 2 ) );
	return 0;
}

LUA_FUNCTION( bytebuffer_isbuffervalid )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_NUMBER );
	LUA->PushBoolean( reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->IsBufferValid( ) );
	return 1;
}

LUA_FUNCTION( bytebuffer_eob )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	LUA->CheckType( 2, LUATYPE_NUMBER );
	LUA->PushBoolean( reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) )->EOB( ) );
	return 1;
}

LUA_FUNCTION( bytebuffer_getbuffer )
{
	LUA->CheckUserdata( 1, BYTEBUFFER_META );
	ByteBuffer *buffer = reinterpret_cast<ByteBuffer *>( LUA->ToUserdata( 1 ) );
	LUA->PushString( (const char *)buffer->GetBuffer( ), buffer->Size( ) );
	return 1;
}

LUA_MODULE_LOAD( )
{
	LUA->NewMetatable( BYTEBUFFER_META );

	LUA->PushCFunction( bytebuffer_readinteger );
	LUA->SetField( -2, "readinteger" );

	LUA->PushCFunction( bytebuffer_readfloat );
	LUA->SetField( -2, "readfloat" );

	LUA->PushCFunction( bytebuffer_readdouble );
	LUA->SetField( -2, "readdouble" );

	LUA->PushCFunction( bytebuffer_readbool );
	LUA->SetField( -2, "readbool" );

	LUA->PushCFunction( bytebuffer_readstring );
	LUA->SetField( -2, "readstring" );

	LUA->PushCFunction( bytebuffer_writeinteger );
	LUA->SetField( -2, "writeinteger" );

	LUA->PushCFunction( bytebuffer_writefloat );
	LUA->SetField( -2, "writefloat" );

	LUA->PushCFunction( bytebuffer_writedouble );
	LUA->SetField( -2, "writedouble" );

	LUA->PushCFunction( bytebuffer_writebool );
	LUA->SetField( -2, "writebool" );

	LUA->PushCFunction( bytebuffer_writestring );
	LUA->SetField( -2, "writestring" );

	LUA->PushCFunction( bytebuffer_assign );
	LUA->SetField( -2, "assign" );

	LUA->PushCFunction( bytebuffer_stringlength );
	LUA->SetField( -2, "stringlength" );

	LUA->PushCFunction( bytebuffer_reserve );
	LUA->SetField( -2, "reserve" );

	LUA->PushCFunction( bytebuffer_resize );
	LUA->SetField( -2, "resize" );

	LUA->PushCFunction( bytebuffer_clear );
	LUA->SetField( -2, "clear" );

	LUA->PushCFunction( bytebuffer_tell );
	LUA->SetField( -2, "tell" );

	LUA->PushCFunction( bytebuffer_size );
	LUA->SetField( -2, "size" );

	LUA->PushCFunction( bytebuffer_capacity );
	LUA->SetField( -2, "capacity" );

	LUA->PushCFunction( bytebuffer_capacity );
	LUA->SetField( -2, "capacity" );

	LUA->PushCFunction( bytebuffer_seek );
	LUA->SetField( -2, "seek" );

	LUA->PushCFunction( bytebuffer_isbuffervalid );
	LUA->SetField( -2, "isbuffervalid" );

	LUA->PushCFunction( bytebuffer_eob );
	LUA->SetField( -2, "eob" );

	LUA->PushCFunction( bytebuffer_getbuffer );
	LUA->SetField( -2, "getbuffer" );

	LUA->PushCFunction( bytebuffer_delete );
	LUA->SetField( -2, "__gc" );

	LUA->PushCFunction( bytebuffer_tostring );
	LUA->SetField( -2, "__tostring" );

	LUA->PushCFunction( bytebuffer_index );
	LUA->SetField( -2, "__index" );

	LUA->PushCFunction( bytebuffer_newindex );
	LUA->SetField( -2, "__newindex" );

	LUA->PushValue( -1 );
	LUA->SetField( -2, "__metatable" );

	LUA->Pop( 1 );

	LUA->PushCFunction( bytebuffer_new );
	return 1;
}

LUA_MODULE_UNLOAD( )
{
	return 0;
}