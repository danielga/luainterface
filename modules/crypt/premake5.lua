project("LuaInterface-Crypt")
	uuid("688176ae-b413-48e4-b6b8-fa296de20a4e")
	kind("SharedLib")
	includedirs({INCLUDE_FOLDER, "cryptopp/include"})
	files({"*.cpp"})
	vpaths({["Source files"] = "**.cpp"})
	links({"LuaInterface", "cryptopp"})
	targetprefix("")
	targetname("crypt")
	targetsuffix("")

project("cryptopp")
	uuid("106c54a8-9f04-4b50-9f29-baa26fc04ae0")
	kind("StaticLib")
	includedirs({"cryptopp/include/cryptopp", "cryptopp/src"})
	files({"cryptopp/include/cryptopp/*.h", "cryptopp/src/*.cpp"})
	vpaths({["Header files"] = "**.h", ["Source files"] = "**.cpp"})