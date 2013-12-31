project("LuaInterface-Crypt")
	kind("SharedLib")
	includedirs({"Source", "Modules/Crypt", "Modules/Crypt/cryptopp/include"})
	files({"Modules/Crypt/*.c", "Modules/Crypt/*.cpp", "Modules/Crypt/*.h", "Modules/Crypt/*.hpp"})
	vpaths({["Header files"] = {"**.h", "**.hpp"}, ["Source files"] = {"**.c", "**.cpp"}})
	links({"LuaInterface"})

	configuration("Debug or Static Debug")
		links({"cryptoppd"})

	configuration("Release or Static Release")
		links({"cryptopp"})

	targetprefix("")
	targetname("crypt")
	targetsuffix("")

	configuration("windows")
		libdirs({"Modules/Crypt/cryptopp/lib"})