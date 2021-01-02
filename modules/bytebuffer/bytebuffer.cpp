#include <Lua/Interface.hpp>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <vector>

#if _WIN32

#define snprintf _snprintf

#endif

static const char metaname[] = "bytebuffer";
static const char invalid_object[] = "failed to create new bytebuffer";

enum SeekMode
{
	SEEKMODE_SET,
	SEEKMODE_CUR,
	SEEKMODE_END
};

class ByteBuffer
{
public:
	ByteBuffer( ) :
		end_of_file( true ),
		buffer_offset( 0 )
	{ }

	ByteBuffer( size_t size ) :
		end_of_file( true ),
		buffer_offset( 0 )
	{
		Resize( size );
	}

	ByteBuffer( const uint8_t *copy_buffer, size_t size ) :
		end_of_file( true ),
		buffer_offset( 0 )
	{
		Assign( copy_buffer, size );
	}

	typedef void ( *unspecified_bool_type ) ( );
	static void unspecified_bool_true( ) { }

	bool IsValid( ) const
	{
		return !EndOfFile( );
	}

	operator unspecified_bool_type( ) const
	{
		return IsValid( ) ? unspecified_bool_true : 0;
	}

	bool operator!( ) const
	{
		return !IsValid( );
	}

	size_t Tell( ) const
	{
		return buffer_offset;
	}

	size_t Size( ) const
	{
		return buffer_internal.size( );
	}

	size_t Capacity( ) const
	{
		return buffer_internal.capacity( );
	}

	bool Seek( int32_t position, SeekMode mode = SEEKMODE_SET )
	{
		assert( mode != SEEKMODE_SET || ( mode == SEEKMODE_SET && position >= 0 ) );
		assert( mode != SEEKMODE_CUR || ( mode == SEEKMODE_CUR && Tell( ) + position >= 0 ) );
		assert( mode != SEEKMODE_END || ( mode == SEEKMODE_END && Size( ) + position >= 0 ) );

		int32_t temp;
		switch( mode )
		{
		case SEEKMODE_SET:
			buffer_offset = static_cast<size_t>( position > 0 ? position : 0 );
			break;

		case SEEKMODE_CUR:
			temp = Tell( ) + position;
			buffer_offset = static_cast<size_t>( temp > 0 ? temp : 0 );
			break;

		case SEEKMODE_END:
			temp = Size( ) + position;
			buffer_offset = static_cast<size_t>( temp > 0 ? temp : 0 );
			break;

		default:
			return false;
		}

		end_of_file = false;
		return true;
	}

	bool EndOfFile( ) const
	{
		return end_of_file;
	}

	uint8_t *GetBuffer( )
	{
		return &buffer_internal[0];
	}

	const uint8_t *GetBuffer( ) const
	{
		return &buffer_internal[0];
	}

	void Clear( )
	{
		buffer_internal.clear( );
		buffer_offset = 0;
		end_of_file = false;
	}

	void Reserve( size_t capacity )
	{
		buffer_internal.reserve( capacity );
	}

	void Resize( size_t size )
	{
		buffer_internal.resize( size );
	}

	void ShrinkToFit( )
	{
		std::vector<uint8_t>( buffer_internal ).swap( buffer_internal );
	}

	void Assign( const uint8_t *copy_buffer, size_t size )
	{
		assert( copy_buffer != nullptr && size != 0 );

		buffer_internal.assign( copy_buffer, copy_buffer + size );
		buffer_offset = 0;
		end_of_file = false;
	}

	size_t Read( void *value, size_t size )
	{
		assert( value != nullptr && size != 0 );

		if( buffer_offset >= buffer_internal.size( ) )
		{
			end_of_file = true;
			return 0;
		}

		size_t clamped = buffer_internal.size( ) - buffer_offset;
		if( clamped > size )
			clamped = size;
		memcpy( value, &buffer_internal[buffer_offset], clamped );
		buffer_offset += clamped;
		if( clamped < size )
			end_of_file = true;

		return clamped;
	}

	size_t Write( const void *value, size_t size )
	{
		assert( value != nullptr && size != 0 );

		if( buffer_internal.size( ) < buffer_offset + size )
			Resize( buffer_offset + size );

		memcpy( &buffer_internal[buffer_offset], value, size );
		buffer_offset += size;
		return size;
	}

	ByteBuffer &operator>>( bool &data )
	{
		bool value;
		if( Read( &value, sizeof( bool ) ) == sizeof( bool ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( int8_t &data )
	{
		int8_t value;
		if( Read( &value, sizeof( int8_t ) ) == sizeof( int8_t ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( uint8_t &data )
	{
		uint8_t value;
		if( Read( &value, sizeof( uint8_t ) ) == sizeof( uint8_t ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( int16_t &data )
	{
		int16_t value;
		if( Read( &value, sizeof( int16_t ) ) == sizeof( int16_t ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( uint16_t &data )
	{
		uint16_t value;
		if( Read( &value, sizeof( uint16_t ) ) == sizeof( uint16_t ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( int32_t &data )
	{
		int32_t value;
		if( Read( &value, sizeof( int32_t ) ) == sizeof( int32_t ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( uint32_t &data )
	{
		uint32_t value;
		if( Read( &value, sizeof( uint32_t ) ) == sizeof( uint32_t ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( int64_t &data )
	{
		int64_t value;
		if( Read( &value, sizeof( int64_t ) ) == sizeof( int64_t ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( uint64_t &data )
	{
		uint64_t value;
		if( Read( &value, sizeof( uint64_t ) ) == sizeof( uint64_t ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( float &data )
	{
		float value;
		if( Read( &value, sizeof( float ) ) == sizeof( float ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( double &data )
	{
		double value;
		if( Read( &value, sizeof( double ) ) == sizeof( double ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( char &data )
	{
		char value;
		if( Read( &value, sizeof( char ) ) == sizeof( char ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( char *data )
	{
		assert( data != nullptr );

		char ch = 0;
		size_t offset = 0;
		while( *this >> ch )
		{
			data[offset] = ch;

			if( ch == 0 )
				break;

			++offset;
		}

		return *this;
	}

	ByteBuffer &operator>>( std::string &data )
	{
		char ch = 0;
		while( *this >> ch && ch != 0 )
			data += ch;

		return *this;
	}

	ByteBuffer &operator>>( wchar_t &data )
	{
		wchar_t value;
		if( Read( &value, sizeof( wchar_t ) ) == sizeof( wchar_t ) )
			data = value;

		return *this;
	}

	ByteBuffer &operator>>( wchar_t *data )
	{
		assert( data != nullptr );

		wchar_t ch = 0;
		size_t offset = 0;
		while( *this >> ch )
		{
			data[offset] = ch;

			if( ch == 0 )
				break;

			++offset;
		}

		return *this;
	}

	ByteBuffer &operator>>( std::wstring &data )
	{
		wchar_t ch = 0;
		while( *this >> ch && ch != 0 )
			data += ch;

		return *this;
	}

	ByteBuffer &operator<<( const bool &data )
	{
		Write( &data, sizeof( bool ) );
		return *this;
	}

	ByteBuffer &operator<<( const int8_t &data )
	{
		Write( &data, sizeof( int8_t ) );
		return *this;
	}

	ByteBuffer &operator<<( const uint8_t &data )
	{
		Write( &data, sizeof( uint8_t ) );
		return *this;
	}

	ByteBuffer &operator<<( const int16_t &data )
	{
		Write( &data, sizeof( int16_t ) );
		return *this;
	}

	ByteBuffer &operator<<( const uint16_t &data )
	{
		Write( &data, sizeof( uint16_t ) );
		return *this;
	}

	ByteBuffer &operator<<( const int32_t &data )
	{
		Write( &data, sizeof( int32_t ) );
		return *this;
	}

	ByteBuffer &operator<<( const uint32_t &data )
	{
		Write( &data, sizeof( uint32_t ) );
		return *this;
	}

	ByteBuffer &operator<<( const int64_t &data )
	{
		Write( &data, sizeof( int64_t ) );
		return *this;
	}

	ByteBuffer &operator<<( const uint64_t &data )
	{
		Write( &data, sizeof( uint64_t ) );
		return *this;
	}

	ByteBuffer &operator<<( const float &data )
	{
		Write( &data, sizeof( float ) );
		return *this;
	}

	ByteBuffer &operator<<( const double &data )
	{
		Write( &data, sizeof( double ) );
		return *this;
	}

	ByteBuffer &operator<<( const char &data )
	{
		Write( &data, sizeof( char ) );
		return *this;
	}

	ByteBuffer &operator<<( const char *data )
	{
		assert( data != nullptr );

		Write( data, ( strlen( data ) + 1 ) * sizeof( char ) );
		return *this;
	}

	ByteBuffer &operator<<( const std::string &data )
	{
		Write( data.c_str( ), ( data.length( ) + 1 ) * sizeof( char ) );
		return *this;
	}

	ByteBuffer &operator<<( const wchar_t &data )
	{
		Write( &data, sizeof( wchar_t ) );
		return *this;
	}

	ByteBuffer &operator<<( const wchar_t *data )
	{
		assert( data != nullptr );

		Write( data, ( wcslen( data ) + 1 ) * sizeof( wchar_t ) );
		return *this;
	}

	ByteBuffer &operator<<( const std::wstring &data )
	{
		Write( data.c_str( ), ( data.length( ) + 1 ) * sizeof( wchar_t ) );
		return *this;
	}

private:
	bool end_of_file;
	std::vector<uint8_t> buffer_internal;
	size_t buffer_offset;
};

namespace bytebuffer
{
	static int create( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		ByteBuffer *buffer = reinterpret_cast<ByteBuffer *>( lua.NewUserdata( sizeof( ByteBuffer ) ) );
		if( buffer == nullptr )
			lua.ThrowError( invalid_object );

		new( buffer ) ByteBuffer( );

		lua.NewMetatable( metaname );
		lua.SetMetaTable( -2 );

		lua.CreateTable( );
		lua.SetUserValue( -2 );

		if( lua.GetType( 1 ) == Lua::Type::String )
		{
			size_t size = 0;
			const uint8_t *data = reinterpret_cast<const uint8_t *>( lua.ToString( 1, &size ) );
			buffer->Assign( data, size );
		}

		return 1;
	}

	static int destroy( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.ToUserdata<ByteBuffer>( 1 )->~ByteBuffer( );
		return 0;
	}

	static int tostring( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		char msg[30];
		snprintf( msg, sizeof( msg ), "%s: 0x%p", metaname, lua.ToUserdata<ByteBuffer>( 1 ) );
		lua.PushString( msg );
		return 1;
	}

	static int index( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );

		lua.GetUserValue( 1 );
		lua.PushValue( 2 );
		lua.RawGet( -2 );

		if( lua.GetType( -1 ) > Lua::Type::Nil )
			return 1;

		lua.Pop( 2 );

		lua.GetMetaTable( 1 );
		lua.PushValue( 2 );
		lua.RawGet( -2 );

		return 1;
	}

	static int newindex( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.GetUserValue( 1 );
		lua.PushValue( 2 );
		lua.PushValue( 3 );
		lua.RawSet( -3 );
		return 0;
	}

	static int readinteger( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::Number );

		ByteBuffer &buffer = *lua.ToUserdata<ByteBuffer>( 1 );
		if( lua.IsType( 3, Lua::Type::Boolean ) && lua.ToBoolean( 3 ) )
			switch( static_cast<int32_t>( lua.ToNumber( 2 ) ) )
			{
			case 1:
			{
				uint8_t number;
				if( !( buffer >> number ) )
					return 0;

				lua.PushNumber( number );
				break;
			}

			case 2:
			{
				uint16_t number;
				if( !( buffer >> number ) )
					return 0;

				lua.PushNumber( number );
				break;
			}

			case 4:
			{
				uint32_t number;
				if( !( buffer >> number ) )
					return 0;

				lua.PushNumber( number );
				break;
			}

			case 8:
			{
				uint64_t number;
				if( !( buffer >> number ) )
					return 0;

				lua.PushNumber( static_cast<double>( number ) );
				break;
			}

			default:
				return lua.ArgError( 2, "requested number of bytes to read not implemented (only 1, 2, 4 and 8 can be used)" );
			}
		else
			switch( static_cast<int32_t>( lua.ToNumber( 2 ) ) )
			{
			case 1:
			{
				int8_t number;
				if( !( buffer >> number ) )
					return 0;

				lua.PushNumber( number );
				break;
			}

			case 2:
			{
				int16_t number;
				if( !( buffer >> number ) )
					return 0;

				lua.PushNumber( number );
				break;
			}

			case 4:
			{
				int32_t number;
				if( !( buffer >> number ) )
					return 0;

				lua.PushNumber( number );
				break;
			}

			case 8:
			{
				int64_t number;
				if( !( buffer >> number ) )
					return 0;

				lua.PushNumber( static_cast<double>( number ) );
				break;
			}

			default:
				return lua.ArgError( 2, "requested number of bytes to read not implemented (only 1, 2, 4 and 8 can be used)" );
			}

		return 1;
	}

	static int readfloat( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );

		float number;
		if( !( *lua.ToUserdata<ByteBuffer>( 1 ) >> number ) )
			return 0;

		lua.PushNumber( number );
		return 1;
	}

	static int readdouble( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );

		double number;
		if( !( *lua.ToUserdata<ByteBuffer>( 1 ) >> number ) )
			return 0;

		lua.PushNumber( number );
		return 1;
	}

	static int readbool( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );

		bool number;
		if( !( *lua.ToUserdata<ByteBuffer>( 1 ) >> number ) )
			return 0;

		lua.PushNumber( number );
		return 1;
	}

	static int readstring( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );

		std::string str;
		if( !( *lua.ToUserdata<ByteBuffer>( 1 ) >> str ) )
			return 0;

		lua.PushString( str.c_str( ), str.size( ) );
		return 1;
	}

	static int writeinteger( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::Number );
		lua.CheckType( 3, Lua::Type::Number );

		ByteBuffer &buffer = *lua.ToUserdata<ByteBuffer>( 1 );
		if( lua.IsType( 4, Lua::Type::Boolean ) && lua.ToBoolean( 4 ) )
			switch( static_cast<int32_t>( lua.ToNumber( 3 ) ) )
			{
			case 1:
				buffer << static_cast<const uint8_t>( lua.ToNumber( 2 ) );
				break;

			case 2:
				buffer << static_cast<const uint16_t>( lua.ToNumber( 2 ) );
				break;

			case 4:
				buffer << static_cast<const uint32_t>( lua.ToNumber( 2 ) );
				break;

			case 8:
				buffer << static_cast<const uint64_t>( lua.ToNumber( 2 ) );
				break;

			default:
				return lua.ArgError( 3, "requested number of bytes to read not implemented (only 1, 2, 4 and 8 can be used)" );
			}
		else
			switch( static_cast<int32_t>( lua.ToNumber( 3 ) ) )
			{
			case 1:
				buffer << static_cast<const int8_t>( lua.ToNumber( 2 ) );
				break;

			case 2:
				buffer << static_cast<const int16_t>( lua.ToNumber( 2 ) );
				break;

			case 4:
				buffer << static_cast<const int32_t>( lua.ToNumber( 2 ) );
				break;

			case 8:
				buffer << static_cast<const int64_t>( lua.ToNumber( 2 ) );
				break;

			default:
				return lua.ArgError( 3, "requested number of bytes to read not implemented (only 1, 2, 4 and 8 can be used)" );
			}

		return 0;
	}

	static int writefloat( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::Number );
		*lua.ToUserdata<ByteBuffer>( 1 ) << static_cast<float>( lua.ToNumber( 2 ) );
		return 0;
	}

	static int writedouble( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::Number );
		*lua.ToUserdata<ByteBuffer>( 1 ) << lua.ToNumber( 2 );
		return 0;
	}

	static int writebool( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::Boolean );
		*lua.ToUserdata<ByteBuffer>( 1 ) << ( lua.ToBoolean( 2 ) == 1 );
		return 0;
	}

	static int writestring( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::String );
		*lua.ToUserdata<ByteBuffer>( 1 ) << lua.ToString( 2 );
		return 0;
	}

	static int assign( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::String );
		size_t len = 0;
		const uint8_t *data = reinterpret_cast<const uint8_t *>( lua.ToString( 2, &len ) );
		lua.ToUserdata<ByteBuffer>( 1 )->Assign( data, len );
		return 0;
	}

	static int reserve( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::Number );
		lua.ToUserdata<ByteBuffer>( 1 )->Reserve( static_cast<size_t>( lua.ToNumber( 2 ) ) );
		return 0;
	}

	static int resize( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::String );
		lua.ToUserdata<ByteBuffer>( 1 )->Resize( static_cast<size_t>( lua.ToNumber( 2 ) ) );
		return 0;
	}

	static int clear( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.ToUserdata<ByteBuffer>( 1 )->Clear( );
		return 0;
	}

	static int tell( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.PushNumber( lua.ToUserdata<ByteBuffer>( 1 )->Tell( ) );
		return 1;
	}

	static int size( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.PushNumber( lua.ToUserdata<ByteBuffer>( 1 )->Size( ) );
		return 1;
	}

	static int capacity( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.PushNumber( lua.ToUserdata<ByteBuffer>( 1 )->Capacity( ) );
		return 1;
	}

	static int shrinktofit( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.ToUserdata<ByteBuffer>( 1 )->ShrinkToFit( );
		return 0;
	}

	static int seek( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::Number );
		lua.PushNumber( lua.ToUserdata<ByteBuffer>( 1 )->Seek( static_cast<int32_t>( lua.ToNumber( 2 ) ) ) );
		return 1;
	}

	static int isvalid( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::Number );
		lua.PushBoolean( lua.ToUserdata<ByteBuffer>( 1 )->IsValid( ) );
		return 1;
	}

	static int eof( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		lua.CheckType( 2, Lua::Type::Number );
		lua.PushBoolean( lua.ToUserdata<ByteBuffer>( 1 )->EndOfFile( ) );
		return 1;
	}

	static int getbuffer( lua_State *state )
	{
		Lua::Interface &lua = GetLuaInterface( state );
		lua.CheckUserdata( 1, metaname );
		ByteBuffer &buffer = *lua.ToUserdata<ByteBuffer>( 1 );
		lua.PushString( reinterpret_cast<const char *>( buffer.GetBuffer( ) ), buffer.Size( ) );
		return 1;
	}
}

extern "C" int luaopen_bytebuffer( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );

	lua.NewMetatable( metaname );

	lua.PushFunction( bytebuffer::readinteger );
	lua.SetField( -2, "readinteger" );

	lua.PushFunction( bytebuffer::readfloat );
	lua.SetField( -2, "readfloat" );

	lua.PushFunction( bytebuffer::readdouble );
	lua.SetField( -2, "readdouble" );

	lua.PushFunction( bytebuffer::readbool );
	lua.SetField( -2, "readbool" );

	lua.PushFunction( bytebuffer::readstring );
	lua.SetField( -2, "readstring" );

	lua.PushFunction( bytebuffer::writeinteger );
	lua.SetField( -2, "writeinteger" );

	lua.PushFunction( bytebuffer::writefloat );
	lua.SetField( -2, "writefloat" );

	lua.PushFunction( bytebuffer::writedouble );
	lua.SetField( -2, "writedouble" );

	lua.PushFunction( bytebuffer::writebool );
	lua.SetField( -2, "writebool" );

	lua.PushFunction( bytebuffer::writestring );
	lua.SetField( -2, "writestring" );

	lua.PushFunction( bytebuffer::assign );
	lua.SetField( -2, "assign" );

	lua.PushFunction( bytebuffer::reserve );
	lua.SetField( -2, "reserve" );

	lua.PushFunction( bytebuffer::resize );
	lua.SetField( -2, "resize" );

	lua.PushFunction( bytebuffer::clear );
	lua.SetField( -2, "clear" );

	lua.PushFunction( bytebuffer::tell );
	lua.SetField( -2, "tell" );

	lua.PushFunction( bytebuffer::size );
	lua.SetField( -2, "size" );

	lua.PushFunction( bytebuffer::capacity );
	lua.SetField( -2, "capacity" );

	lua.PushFunction( bytebuffer::shrinktofit );
	lua.SetField( -2, "shrinktofit" );

	lua.PushFunction( bytebuffer::seek );
	lua.SetField( -2, "seek" );

	lua.PushFunction( bytebuffer::isvalid );
	lua.SetField( -2, "isvalid" );

	lua.PushFunction( bytebuffer::eof );
	lua.SetField( -2, "eof" );

	lua.PushFunction( bytebuffer::getbuffer );
	lua.SetField( -2, "getbuffer" );

	lua.PushFunction( bytebuffer::destroy );
	lua.SetField( -2, "__gc" );

	lua.PushFunction( bytebuffer::tostring );
	lua.SetField( -2, "__tostring" );

	lua.PushFunction( bytebuffer::index );
	lua.SetField( -2, "__index" );

	lua.PushFunction( bytebuffer::newindex );
	lua.SetField( -2, "__newindex" );

	lua.PushValue( -1 );
	lua.SetField( -2, "__metatable" );

	lua.Pop( 1 );

	lua.PushFunction( bytebuffer::create );
	return 1;
}
