project("LuaInterface-Thread")
	kind("SharedLib")
	includedirs({"source", "luajit/src", "modules/thread"})
	files({
		"modules/thread/*.c",
		"modules/thread/*.cpp",
		"modules/thread/*.h",
		"modules/thread/*.hpp"
	})
	vpaths({
		["Header files"] = {"**.h", "**.hpp"},
		["Source files"] = {"**.c", "**.cpp"}
	})
	links("LuaInterface")
	targetprefix("")
	targetname("llthreads")
	targetsuffix("")

	filter("system:not windows")
		links("pthread")
