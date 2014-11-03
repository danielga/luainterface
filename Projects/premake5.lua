newoption({
	trigger = "static-runtime",
	description = "Force the use of the static C runtime (only works with static builds)"
})

newoption({
	trigger = "lua-api",
	value = "lua (default)",
	description = "Choose a particular Lua API to use internally",
	allowed = {
		{"lua", "Lua"},
		{"luajit", "LuaJIT"}
	}
})

LUA_API = _OPTIONS["lua-api"]
if not LUA_API then
	LUA_API = "lua"
end

LUA_FOLDER = "Lua"
if LUA_API == "luajit" then
	LUA_FOLDER = "LuaJIT"
end

SOURCE_FOLDER = "../Source"
INCLUDE_FOLDER = "../Include"
THIRDPARTY_FOLDER = "../" .. LUA_FOLDER .. "/src"
MODULES_FOLDER = "../Modules"
TESTING_FOLDER = "../Testing"
PROJECT_FOLDER = os.get() .. "/" .. _ACTION

if LUA_API == "lua" then
	if os.is("windows") then
		LIBS = {"Lua"}
	else
		LIBS = {"Lua", "dl"}
	end
elseif LUA_API == "luajit" then
	if os.is("windows") then
		LIBS = {"lua51"}
	else
		LIBS = {"luajit5.1", "dl"}
	end

	LIBS_FOLDER = "../" .. LUA_FOLDER .. "/build/" .. os.get()
end

solution("LuaInterface")
	language("C++")
	location(PROJECT_FOLDER)
	flags({"NoPCH"})

	if os.is("macosx") then
		platforms({"Universal32", "Universal64"})
	else
		platforms({"x32", "x64"})
	end

	configurations({"Release", "Debug", "Static Release", "Static Debug"})

	configuration("Release")
		kind("SharedLib")
		optimize("On")
		vectorextensions("SSE2")
		objdir(PROJECT_FOLDER .. "/Intermediate")

		configuration({"Release", "x32 or Universal32"})
			targetdir(PROJECT_FOLDER .. "/Release x86")

		configuration({"Release", "x64 or Universal64"})
			targetdir(PROJECT_FOLDER .. "/Release x64")

	configuration("Debug")
		kind("SharedLib")
		flags({"Symbols"})
		objdir(PROJECT_FOLDER .. "/Intermediate")

		configuration({"Debug", "x32 or Universal32"})
			targetdir(PROJECT_FOLDER .. "/Debug x86")

		configuration({"Debug", "x64 or Universal64"})
			targetdir(PROJECT_FOLDER .. "/Debug x64")

	configuration("Static Release")
		kind("StaticLib")
		optimize("On")
		vectorextensions("SSE2")
		objdir(PROJECT_FOLDER .. "/Intermediate")

		configuration({"Static Release", "static-runtime"})
			flags({"StaticRuntime"})

		configuration({"Static Release", "x32 or Universal32"})
			targetdir(PROJECT_FOLDER .. "/Static Release x86")

		configuration({"Static Release", "x64 or Universal64"})
			targetdir(PROJECT_FOLDER .. "/Static Release x64")

	configuration("Static Debug")
		kind("StaticLib")
		flags({"Symbols"})
		objdir(PROJECT_FOLDER .. "/Intermediate")

		configuration({"Static Debug", "static-runtime"})
			flags({"StaticRuntime"})

		configuration({"Static Debug", "x32 or Universal32"})
			targetdir(PROJECT_FOLDER .. "/Static Debug x86")

		configuration({"Static Debug", "x64 or Universal64"})
			targetdir(PROJECT_FOLDER .. "/Static Debug x64")

	project("LuaInterface Test")
		kind("ConsoleApp")
		warnings("Extra")
		includedirs({INCLUDE_FOLDER})
		files({TESTING_FOLDER .. "/*.cpp", TESTING_FOLDER .. "/*.hpp"})
		vpaths({["Header files"] = "**.hpp", ["Source files"] = "**.cpp"})
		links({"LuaInterface"})

		configuration("not windows")
			linkoptions({"-Wl,-R,./"})

			configuration({"Static *", "x32 or universal32"})
				if LUA_API == "luajit" then
					libdirs({LIBS_FOLDER .. "/x86/static release"})
				end
				links(LIBS)

			configuration({"Static *", "x64 or universal64"})
				if LUA_API == "luajit" then
					libdirs({LIBS_FOLDER .. "/x64/static release"})
				end
				links(LIBS)

	project("LuaInterface")
		warnings("Extra")
		defines({"LUAINTERFACE_EXPORT"})
		includedirs({INCLUDE_FOLDER, THIRDPARTY_FOLDER, SOURCE_FOLDER})
		files({SOURCE_FOLDER .. "/*.cpp", INCLUDE_FOLDER .. "/*.hpp"})
		vpaths({["Header files"] = "**.hpp", ["Source files"] = "**.cpp"})

		configuration({"not Static *", "x32 or universal32"})
			if LUA_API == "luajit" then
				libdirs({LIBS_FOLDER .. "/x86/static release"})
			end
			links(LIBS)

		configuration({"not Static *", "x64 or universal64"})
			if LUA_API == "luajit" then
				libdirs({LIBS_FOLDER .. "/x64/static release"})
			end
			links(LIBS)

	if LUA_API == "lua" then
		project("Lua")
			kind("StaticLib")
			defines({"LUA_COMPAT_MODULE"})
			includedirs({THIRDPARTY_FOLDER})
			files({THIRDPARTY_FOLDER .. "/*.c", THIRDPARTY_FOLDER .. "/*.h"})
			vpaths({["Header files"] = "**.h", ["Source files"] = "**.c"})
	end

	INCLUDE_FOLDER = "../" .. INCLUDE_FOLDER
	THIRDPARTY_FOLDER = "../" .. THIRDPARTY_FOLDER

	local modules = os.matchdirs(MODULES_FOLDER .. "/*")
	for _, folder in pairs(modules) do
		include(folder)
	end