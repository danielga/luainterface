project("LuaInterface-Socket")
	kind("SharedLib")
	includedirs({"Source", LUA_INCLUDES, "Modules/Socket"})
	files({"Modules/Socket/*.c", "Modules/Socket/*.h"})
	vpaths({["Header files"] = {"**.h"}, ["Source files"] = {"**.c"}})
	links({"LuaInterface"})
	if os.is("windows") then
		includedirs({"Modules/Socket/windows"})
		files({"Modules/Socket/windows/*.c", "Modules/Socket/windows/*.h"})
		links({"ws2_32"})
	else
		includedirs({"Modules/Socket/unix"})
		files({"Modules/Socket/unix/*.c", "Modules/Socket/unix/*.h"})
	end
	targetprefix("")
	targetname("socket")
	targetsuffix("")