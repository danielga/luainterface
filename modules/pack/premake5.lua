project("LuaInterface-Pack")
	uuid("6cbdd83e-36ec-4cfb-be98-3b0eec433cab")
	kind("SharedLib")
	includedirs({INCLUDE_FOLDER, THIRDPARTY_FOLDER})
	files("*.c")
	vpaths({["Source files"] = "**.c"})
	links("LuaInterface")
	targetprefix("")
	targetname("pack")
	targetsuffix("")
