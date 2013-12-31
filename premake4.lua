LUA_INCLUDES = "LuaJIT/src"
LUA_LIBRARIES = "LuaJIT/build"

solution("LuaInterface")
	language("C++")
	location("Projects/" .. os.get() .. "/" .. _ACTION)
	flags({"NoPCH", "StaticRuntime"})

	if os.is("macosx") then
		platforms({"universal32", "universal64"})
	else
		platforms({"x32", "x64"})
	end
	configurations({"Release", "Debug", "Static Release", "Static Debug"})

	configuration("Release")
		defines({"NDEBUG", "LUAINTERFACE_EXPORT"})
		flags({"Optimize", "EnableSSE"})
		kind("SharedLib")
		targetdir("Projects/" .. os.get() .. "/" .. _ACTION .. "/Release")
		objdir("Projects/" .. os.get() .. "/" .. _ACTION .. "/Intermediate")

	configuration("Debug")
		defines({"DEBUG", "LUAINTERFACE_EXPORT"})
		flags({"Symbols"})
		kind("SharedLib")
		targetdir("Projects/" .. os.get() .. "/" .. _ACTION .. "/Debug")
		objdir("Projects/" .. os.get() .. "/" .. _ACTION .. "/Intermediate")

	configuration("Static Release")
		defines({"NDEBUG", "LUAINTERFACE_STATIC"})
		flags({"Optimize", "EnableSSE"})
		kind("StaticLib")
		targetdir("Projects/" .. os.get() .. "/" .. _ACTION .. "/Static Release")
		objdir("Projects/" .. os.get() .. "/" .. _ACTION .. "/Intermediate")

	configuration("Static Debug")
		defines({"DEBUG", "LUAINTERFACE_STATIC"})
		flags({"Symbols"})
		kind("StaticLib")
		targetdir("Projects/" .. os.get() .. "/" .. _ACTION .. "/Static Debug")
		objdir("Projects/" .. os.get() .. "/" .. _ACTION .. "/Intermediate")

	local lnks = {"lua51", not os.is("windows") and "dl" or nil}
	project("LuaInterface Test")
		kind("ConsoleApp")
		includedirs({"Source"})
		files({"Testing/*.c", "Testing/*.cxx", "Testing/*.cpp", "Testing/*.h", "Testing/*.hxx", "Testing/*.hpp"})
		vpaths({["Header files"] = {"**.h", "**.hxx", "**.hpp"}, ["Source files"] = {"**.c", "**.cxx", "**.cpp"}})
		links({"LuaInterface"})
		
		configuration("not windows")
			linkoptions({"-Wl,-R,./"})

		configuration({"Static Release", "x32 or universal32"})
			links(lnks)
			libdirs({LUA_LIBRARIES .. "/" .. os.get() .. "/x32/static release"})

		configuration({"Static Release", "x64 or universal64"})
			links(lnks)
			libdirs({LUA_LIBRARIES .. "/" .. os.get() .. "/x64/static release"})

		configuration({"Static Debug", "x32 or universal32"})
			links(lnks)
			libdirs({LUA_LIBRARIES .. "/" .. os.get() .. "/x32/static debug"})

		configuration({"Static Debug", "x64 or universal64"})
			links(lnks)
			libdirs({LUA_LIBRARIES .. "/" .. os.get() .. "/x64/static debug"})

	project("LuaInterface")
		includedirs({"Source", LUA_INCLUDES})
		files({"Source/*.c", "Source/*.cxx", "Source/*.cpp", "Source/*.h", "Source/*.hxx", "Source/*.hpp"})
		vpaths({["Header files"] = {"**.h", "**.hxx", "**.hpp"}, ["Source files"] = {"**.c", "**.cxx", "**.cpp"}})

		configuration({"Release", "x32 or universal32"})
			links(lnks)
			libdirs({LUA_LIBRARIES .. "/" .. os.get() .. "/x32/static release"})

		configuration({"Release", "x64 or universal64"})
			links(lnks)
			libdirs({LUA_LIBRARIES .. "/" .. os.get() .. "/x64/static release"})

		configuration({"Debug", "x32 or universal32"})
			links(lnks)
			libdirs({LUA_LIBRARIES .. "/" .. os.get() .. "/x32/static debug"})

		configuration({"Debug", "x64 or universal64"})
			links(lnks)
			libdirs({LUA_LIBRARIES .. "/" .. os.get() .. "/x64/static debug"})

	local modules = os.matchdirs("Modules/*")
	for _, folder in pairs(modules) do
		local file = io.open(folder .. "/premake4.lua")
		assert(file)
		local data = file:read("*a")
		assert(data)
		local chunk = loadstring(data)
		assert(chunk)
		chunk()
		file:close()
	end
