#include <Lua/Interface.hpp>
#include <cryptopp/base64.h>
#include <cryptopp/crc.h>
#include <cryptopp/sha.h>
#include <cryptopp/tiger.h>
#include <cryptopp/md2.h>
#include <cryptopp/md4.h>
#include <cryptopp/md5.h>
#include <cryptopp/whrlpool.h>
#include <cryptopp/ripemd.h>
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/hex.h>

#define THROW_ERROR( lua, error ) ( lua.ThrowError( error ), 0 )
#define LUA_ERROR( lua ) THROW_ERROR( lua, lua.ToString( -1 ) )

#define HASHER_METATABLE "hasher"

#define GET_HASHER( lua, index ) reinterpret_cast<CryptoPP::HashTransformation *>( lua.ToUserdata( index ) )

#if defined _WIN32

#define snprintf _snprintf

#endif

class BaseObject
{
public:
	virtual void SetPrimaryKey( const std::string &priKey ) = 0;
	virtual std::string GenerateSecondaryKey( const std::string &priKey ) = 0;
	virtual void SetSecondaryKey( const std::string &secKey ) = 0;
	virtual std::string Decrypt( const std::string &data ) = 0;
	virtual std::string Encrypt( const std::string &data ) = 0;
};

class AESObject : public BaseObject
{
public:
	void SetPrimaryKey( const std::string &priKey )
	{
		size_t keySize = priKey.size( );
		if( keySize != 16 && keySize != 24 && keySize != 32 )
			return;

		const uint8_t *keyData = reinterpret_cast<const uint8_t *>( priKey.c_str( ) );

		decryptor.SetKey( keyData, keySize );
		encryptor.SetKey( keyData, keySize );
	}

	std::string GenerateSecondaryKey( const std::string &priKey )
	{
		return std::string( );
	}

	void SetSecondaryKey( const std::string &secKey )
	{
		size_t ivSize = secKey.size( );
		if( ivSize != 16 && ivSize != 24 && ivSize != 32 )
			return;

		const uint8_t *ivData = reinterpret_cast<const uint8_t *>( secKey.c_str( ) );

		decryptor.Resynchronize( ivData, ivSize );
		encryptor.Resynchronize( ivData, ivSize );
	}

	std::string Decrypt( const std::string &data )
	{
		std::string decrypted;
		CryptoPP::StringSource(
			data, true,
			new CryptoPP::StreamTransformationFilter(
				decryptor,
				new CryptoPP::StringSink( decrypted )
			)
		);

		return decrypted;
	}

	std::string Encrypt( const std::string &data )
	{
		std::string encrypted;
		CryptoPP::StringSource(
			data, true,
			new CryptoPP::StreamTransformationFilter(
				encryptor,
				new CryptoPP::StringSink( encrypted )
			)
		);

		return encrypted;
	}

private:
	CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor;
	CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor;
};

class RSAObject : public BaseObject
{
public:
	void SetPrimaryKey( const std::string &priKey )
	{
		CryptoPP::RSA::PublicKey privKey;
		CryptoPP::StringSource stringSource( priKey, true );
		privKey.Load( stringSource.Ref( ) );
		decryptor.AccessKey( ).AssignFrom( privKey );
	}

	std::string GenerateSecondaryKey( const std::string &priKey )
	{
		CryptoPP::RSA::PublicKey privKey;
		CryptoPP::StringSource stringSource( priKey, true );
		privKey.Load( stringSource.Ref( ) );

		CryptoPP::RSA::PublicKey pubKey;
		pubKey.AssignFrom( privKey );
		std::string publicKey;
		CryptoPP::StringSink pubSink( publicKey );
		pubKey.Save( pubSink.Ref( ) );
		return publicKey;
	}

	void SetSecondaryKey( const std::string &secKey )
	{
		CryptoPP::RSA::PublicKey pubKey;
		CryptoPP::StringSource stringSource( secKey, true );
		pubKey.Load( stringSource.Ref( ) );
		encryptor.AccessKey( ).AssignFrom( pubKey );
	}

	std::string Decrypt( const std::string &data )
	{
		CryptoPP::AutoSeededRandomPool rng;
		std::string decrypted;
		CryptoPP::StringSource(
			data, true,
			new CryptoPP::PK_DecryptorFilter(
				rng, decryptor,
				new CryptoPP::StringSink( decrypted )
			)
		);

		return decrypted;
	}

	std::string Encrypt( const std::string &data )
	{
		CryptoPP::AutoSeededRandomPool rng;
		std::string encrypted;
		CryptoPP::StringSource(
			data, true,
			new CryptoPP::PK_EncryptorFilter(
				rng, encryptor,
				new CryptoPP::StringSink( encrypted )
			)
		);

		return encrypted;
	}

private:
	CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor;
	CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor;
};

class ECCObject : public BaseObject
{
public:
	void SetPrimaryKey( const std::string &priKey )
	{
		CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey privKey;
		CryptoPP::StringSource stringSource( priKey, true );
		privKey.Load( stringSource.Ref( ) );
		decryptor.AccessKey( ).AssignFrom( privKey );
	}

	std::string GenerateSecondaryKey( const std::string &priKey )
	{
		CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey privKey;
		CryptoPP::StringSource stringSource( priKey, true );
		privKey.Load( stringSource.Ref( ) );

		CryptoPP::ECIES<CryptoPP::ECP>::PublicKey pubKey;
		pubKey.AssignFrom( privKey );
		std::string publicKey;
		CryptoPP::StringSink pubSink( publicKey );
		pubKey.Save( pubSink.Ref( ) );
		return publicKey;
	}

	void SetSecondaryKey( const std::string &secKey )
	{
		CryptoPP::ECIES<CryptoPP::ECP>::PublicKey pubKey;
		CryptoPP::StringSource stringSource( secKey, true );
		pubKey.Load( stringSource.Ref( ) );
		encryptor.AccessKey( ).AssignFrom( pubKey );
	}

	std::string Decrypt( const std::string &data )
	{
		CryptoPP::AutoSeededRandomPool rng;
		std::string decrypted;
		CryptoPP::StringSource(
			data, true,
			new CryptoPP::PK_DecryptorFilter(
				rng, decryptor,
				new CryptoPP::StringSink( decrypted )
			)
		);

		return decrypted;
	}

	std::string Encrypt( const std::string &data )
	{
		CryptoPP::AutoSeededRandomPool rng;
		std::string encrypted;
		CryptoPP::StringSource(
			data, true,
			new CryptoPP::PK_EncryptorFilter(
				rng, encryptor,
				new CryptoPP::StringSink( encrypted )
			)
		);

		return encrypted;
	}

private:
	CryptoPP::ECIES<CryptoPP::ECP>::Decryptor decryptor;
	CryptoPP::ECIES<CryptoPP::ECP>::Encryptor encryptor;
};

static int lBase64Encode( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckType( 1, Lua::Type::String );

	size_t dataLength = 0;
	const uint8_t *data = reinterpret_cast<const uint8_t *>( lua.ToString( 1, &dataLength ) );

	try
	{
		std::string encoded;
		CryptoPP::StringSource( data, dataLength, true,
			new CryptoPP::Base64Encoder(
				new CryptoPP::StringSink( encoded ), false
			)
		);

		lua.PushString( encoded.c_str( ), encoded.size( ) );
		return 1;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return lua.Error( );
}

static int lBase64Decode( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckType( 1, Lua::Type::String );

	size_t dataLength = 0;
	const uint8_t *data = reinterpret_cast<const uint8_t *>( lua.ToString( 1, &dataLength ) );

	try
	{
		std::string decoded;
		CryptoPP::StringSource( data, dataLength, true,
			new CryptoPP::Base64Decoder(
				new CryptoPP::StringSink( decoded )
			)
		);

		lua.PushString( decoded.c_str( ), decoded.size( ) );
		return 1;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return lua.Error( );
}

static int aesEncrypt( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	bool hasIV = lua.GetTop( ) > 2;

	lua.CheckType( 1, Lua::Type::String );
	lua.CheckType( 2, Lua::Type::String );
	if( hasIV )
		lua.CheckType( 3, Lua::Type::String );

	size_t keyLen = 0;
	const uint8_t *key = reinterpret_cast<const uint8_t *>( lua.ToString( 2, &keyLen ) );
	if( keyLen != 16 && keyLen != 24 && keyLen != 32 )
		return THROW_ERROR( lua, "invalid key length supplied" );

	size_t ivLen = 0;
	const uint8_t *iv = NULL;
	if( hasIV )
	{
		iv = reinterpret_cast<const uint8_t *>( lua.ToString( 3, &ivLen ) );
		if( ivLen != 16 && ivLen != 24 && ivLen != 32 )
			return THROW_ERROR( lua, "invalid IV length supplied" );
	}

	size_t dataLength = 0;
	const uint8_t *data = reinterpret_cast<const uint8_t *>( lua.ToString( 1, &dataLength ) );

	try
	{
		CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor;
		if( hasIV )
			encryptor.SetKeyWithIV( key, keyLen, iv, ivLen );
		else
			encryptor.SetKey( key, keyLen );

		std::string encrypted;
		CryptoPP::StringSource(
			data, dataLength, true,
			new CryptoPP::StreamTransformationFilter(
				encryptor,
				new CryptoPP::StringSink( encrypted )
			)
		);

		lua.PushString( encrypted.c_str( ), encrypted.size( ) );
		return 1;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return LUA_ERROR( lua );
}

static int aesDecrypt( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	bool hasIV = lua.GetTop( ) > 2;

	lua.CheckType( 1, Lua::Type::String );
	lua.CheckType( 2, Lua::Type::String );
	if( hasIV )
		lua.CheckType( 3, Lua::Type::String );

	size_t keyLen = 0;
	const uint8_t *key = reinterpret_cast<const uint8_t *>( lua.ToString( 2, &keyLen ) );
	if( keyLen != 16 && keyLen != 24 && keyLen != 32 )
		return THROW_ERROR( lua, "invalid key length supplied" );

	size_t ivLen = 0;
	const uint8_t *iv = NULL;
	if( hasIV )
	{
		iv = reinterpret_cast<const uint8_t *>( lua.ToString( 3, &ivLen ) );
		if( ivLen != 16 && ivLen != 24 && ivLen != 32 )
			return THROW_ERROR( lua, "invalid IV length supplied" );
	}

	size_t dataLength = 0;
	const uint8_t *data = reinterpret_cast<const uint8_t *>( lua.ToString( 1, &dataLength ) );

	try
	{
		CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor;
		if( hasIV )
			decryptor.SetKeyWithIV( key, keyLen, iv, ivLen );
		else
			decryptor.SetKey( key, keyLen );

		std::string decrypted;
		CryptoPP::StringSource(
			data, dataLength, true,
			new CryptoPP::StreamTransformationFilter(
				decryptor,
				new CryptoPP::StringSink( decrypted )
			)
		);

		lua.PushString( decrypted.c_str( ), decrypted.size( ) );
		return 1;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return LUA_ERROR( lua );
}

static int rsaGeneratePublicKey( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckType( 1, Lua::Type::String );

	size_t privateKeyLen = 0;
	const uint8_t *privateKey = reinterpret_cast<const uint8_t *>( lua.ToString( 1, &privateKeyLen ) );

	try
	{
		CryptoPP::RSA::PrivateKey privKey;
		CryptoPP::ByteQueue queue;
		queue.Put( privateKey, privateKeyLen );
		queue.MessageEnd( );
		privKey.Load( queue.Ref( ) );

		CryptoPP::RSA::PublicKey pubKey;
		pubKey.AssignFrom( privKey );
		std::string publicKey;
		CryptoPP::StringSink pubSink( publicKey );
		pubKey.Save( pubSink.Ref( ) );

		lua.PushString( publicKey.c_str( ), publicKey.size( ) );
		return 1;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return LUA_ERROR( lua );
}

static int rsaEncrypt( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckType( 1, Lua::Type::String );
	lua.CheckType( 2, Lua::Type::String );

	size_t dataLength = 0;
	const uint8_t *data = reinterpret_cast<const uint8_t *>( lua.ToString( 1, &dataLength ) );

	size_t publicKeyLen = 0;
	const uint8_t *publicKey = reinterpret_cast<const uint8_t *>( lua.ToString( 2, &publicKeyLen ) );

	try
	{
		CryptoPP::RSA::PublicKey pubKey;
		CryptoPP::ByteQueue queue;
		queue.Put( publicKey, publicKeyLen );
		queue.MessageEnd( );
		pubKey.Load( queue.Ref( ) );

		CryptoPP::AutoSeededRandomPool rng;
		CryptoPP::RSAES_OAEP_SHA_Encryptor enc( pubKey );
		std::string encrypted;
		CryptoPP::StringSource(
			data, dataLength, true,
			new CryptoPP::PK_EncryptorFilter(
				rng, enc,
				new CryptoPP::StringSink( encrypted )
			)
		);

		lua.PushString( encrypted.c_str( ), encrypted.size( ) );
		return 1;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return LUA_ERROR( lua );
}

static int rsaDecrypt( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckType( 1, Lua::Type::String );
	lua.CheckType( 2, Lua::Type::String );

	size_t dataLength = 0;
	const uint8_t *data = reinterpret_cast<const uint8_t *>( lua.ToString( 1, &dataLength ) );

	size_t privateKeyLen = 0;
	const uint8_t *privateKey = reinterpret_cast<const uint8_t *>( lua.ToString( 2, &privateKeyLen ) );

	try
	{
		CryptoPP::RSA::PrivateKey privKey;
		CryptoPP::ByteQueue queue;
		queue.Put( privateKey, privateKeyLen );
		queue.MessageEnd( );
		privKey.Load( queue.Ref( ) );

		CryptoPP::AutoSeededRandomPool rng;
		CryptoPP::RSAES_OAEP_SHA_Decryptor dec( privKey );
		std::string decrypted;
		CryptoPP::StringSource(
			data, dataLength, true,
			new CryptoPP::PK_DecryptorFilter(
				rng, dec,
				new CryptoPP::StringSink( decrypted )
			)
		);

		lua.PushString( decrypted.c_str( ), decrypted.size( ) );
		return 1;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return LUA_ERROR( lua );
}

static int hasher__tostring( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );

	CryptoPP::HashTransformation *hasher = GET_HASHER( lua, 1 );
	char buffer[30];
	snprintf( buffer, sizeof( buffer ), "%s: 0x%p", HASHER_METATABLE, hasher );
	lua.PushString( buffer );
	return 1;
}

static int hasher__eq( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );
	lua.CheckUserdata( 2, HASHER_METATABLE );

	lua.PushBoolean( GET_HASHER( lua, 1 ) == GET_HASHER( lua, 2 ) );
	return 1;
}

static int hasher__index( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );

	lua.NewMetatable( HASHER_METATABLE );
	lua.PushValue( 2 );
	lua.RawGet( -2 );
	if( !lua.IsType( -1, Lua::Type::Nil ) )
		return 1;

	lua.Pop( 2 );

	lua.GetUserValue( 1 );
	lua.PushValue( 2 );
	lua.RawGet( -2 );
	return 1;
}

static int hasher__newindex( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );

	lua.GetUserValue( 1 );
	lua.PushValue( 2 );
	lua.PushValue( 3 );
	lua.RawSet( -3 );
	return 0;
}

static int hasher__gc( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );

	CryptoPP::HashTransformation *hasher = GET_HASHER( lua, 1 );

	try
	{
		delete hasher;
		return 0;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return LUA_ERROR( lua );
}

static int hasher_update( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );
	lua.CheckType( 2, Lua::Type::String );

	CryptoPP::HashTransformation *hasher = GET_HASHER( lua, 1 );

	uint32_t len = 0;
	const uint8_t *data = reinterpret_cast<const uint8_t *>( lua.ToString( 2, &len ) );

	try
	{
		hasher->Update( data, len );
		return 0;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return LUA_ERROR( lua );
}

static int hasher_final( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );

	CryptoPP::HashTransformation *hasher = GET_HASHER( lua, 1 );

	try
	{
		uint32_t size = hasher->DigestSize( );
		std::vector<uint8_t> digest( size );
		uint8_t *digestptr = &digest[0];

		hasher->Final( digestptr );

		lua.PushString( reinterpret_cast<const char *>( digestptr ), size );
		return 1;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return LUA_ERROR( lua );
}

static int hasher_restart( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );

	CryptoPP::HashTransformation *hasher = GET_HASHER( lua, 1 );

	try
	{
		hasher->Restart( );
		return 0;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return LUA_ERROR( lua );
}

static int hasher_digest( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );
	lua.CheckType( 2, Lua::Type::String );

	CryptoPP::HashTransformation *hasher = GET_HASHER( lua, 1 );

	uint32_t len = 0;
	const uint8_t *data = reinterpret_cast<const uint8_t *>( lua.ToString( 2, &len ) );

	try
	{
		uint32_t size = hasher->DigestSize( );
		std::vector<uint8_t> digest( size );
		uint8_t *digestptr = &digest[0];

		hasher->CalculateDigest( digestptr, data, len );

		lua.PushString( reinterpret_cast<const char *>( digestptr ), size );
		return 1;
	}
	catch( std::exception &e )
	{
		lua.PushString( e.what( ) );
	}

	return LUA_ERROR( lua );
}

static int hasher_name( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );
	lua.PushString( GET_HASHER( lua, 1 )->AlgorithmName( ).c_str( ) );
	return 1;
}

static int hasher_size( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );
	lua.PushNumber( GET_HASHER( lua, 1 )->DigestSize( ) );
	return 1;
}

static int hasher_blocksize( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );
	lua.CheckUserdata( 1, HASHER_METATABLE );
	lua.PushNumber( GET_HASHER( lua, 1 )->OptimalBlockSize( ) );
	return 1;
}

#define AddFunction( lua, name, func )	\
	lua.PushFunction( func );			\
	lua.SetField( -2, name );

#define AddHashFunction( lua, name, hashType )	\
	lua.PushFunction( hash_ ## hashType );		\
	lua.SetField( -2, name );

#define HashFunction( hashType )										\
static int hash_ ## hashType( lua_State *state )						\
{																		\
	Lua::Interface &lua = GetLuaInterface( state );						\
	void *luadata = lua.NewUserdata( sizeof( CryptoPP::hashType ) );	\
	new( luadata ) CryptoPP::hashType( );								\
																		\
	lua.NewMetatable( HASHER_METATABLE );								\
	lua.SetMetaTable( -2 );												\
																		\
	lua.CreateTable( );													\
	lua.SetUserValue( -2 );												\
																		\
	return 1;															\
}

HashFunction( CRC32 );

HashFunction( SHA1 );
HashFunction( SHA224 );
HashFunction( SHA256 );
HashFunction( SHA384 );
HashFunction( SHA512 );

HashFunction( Tiger );

HashFunction( Whirlpool );

HashFunction( MD2 );
HashFunction( MD4 );
HashFunction( MD5 );

HashFunction( RIPEMD128 );
HashFunction( RIPEMD160 );
HashFunction( RIPEMD256 );
HashFunction( RIPEMD320 );

extern "C" int luaopen_crypt( lua_State *state )
{
	Lua::Interface &lua = GetLuaInterface( state );

	lua.NewMetatable( HASHER_METATABLE );

	lua.PushValue( -1 );
	lua.SetField( -2, "__metatable" );

	AddFunction( lua, "__tostring", hasher__tostring );
	AddFunction( lua, "__eq", hasher__eq );
	AddFunction( lua, "__index", hasher__index );
	AddFunction( lua, "__newindex", hasher__newindex );
	AddFunction( lua, "__gc", hasher__gc );

	AddFunction( lua, "Update", hasher_update );
	AddFunction( lua, "Final", hasher_final );
	AddFunction( lua, "Restart", hasher_restart );

	AddFunction( lua, "CalculateDigest", hasher_digest );

	AddFunction( lua, "AlgorythmName", hasher_name );
	AddFunction( lua, "DigestSize", hasher_size );
	AddFunction( lua, "OptimalBlockSize", hasher_blocksize );



	lua.CreateTable( );

	AddHashFunction( lua, "crc32", CRC32 );

	AddHashFunction( lua, "sha1", SHA1 );
	AddHashFunction( lua, "sha224", SHA224 );
	AddHashFunction( lua, "sha256", SHA256 );
	AddHashFunction( lua, "sha384", SHA384 );
	AddHashFunction( lua, "sha512", SHA512 );

	AddHashFunction( lua, "tiger", Tiger );

	AddHashFunction( lua, "whirlpool", Whirlpool );

	AddHashFunction( lua, "md2", MD2 );
	AddHashFunction( lua, "md4", MD4 );
	AddHashFunction( lua, "md5", MD5 );

	AddHashFunction( lua, "ripemd128", RIPEMD128 );
	AddHashFunction( lua, "ripemd160", RIPEMD160 );
	AddHashFunction( lua, "ripemd256", RIPEMD256 );
	AddHashFunction( lua, "ripemd320", RIPEMD320 );

	AddFunction( lua, "aesEncrypt", aesEncrypt );
	AddFunction( lua, "aesDecrypt", aesDecrypt );

	AddFunction( lua, "rsaGeneratePublicKey", rsaGeneratePublicKey );
	AddFunction( lua, "rsaEncrypt", rsaEncrypt );
	AddFunction( lua, "rsaDecrypt", rsaDecrypt );

	return 1;
}
