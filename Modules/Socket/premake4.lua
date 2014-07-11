project("LuaInterface-Socket")
	kind("SharedLib")
	includedirs({INCLUDE_FOLDER, THIRDPARTY_FOLDER, "./"})
	files({"*.c", "*.h"})
	vpaths({["Header files"] = "**.h", ["Source files"] = "**.c"})
	links({"LuaInterface"})
	if os.is("windows") then
		includedirs({"windows"})
		files({"windows/*.c", "windows/*.h"})
		links({"ws2_32"})
	else
		includedirs({"unix"})
		files({"unix/*.c", "unix/*.h"})
	end
	targetprefix("")
	targetname("socket")
	targetsuffix("")