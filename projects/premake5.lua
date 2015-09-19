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

LUA_FOLDER = LUA_API

SOURCE_FOLDER = "../source"
INCLUDE_FOLDER = "../include"
THIRDPARTY_FOLDER = "../" .. LUA_FOLDER .. "/src"
MODULES_FOLDER = "../modules"
TESTING_FOLDER = "../testing"
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
	uuid("602804da-edd2-4767-93f0-8e3f905b06a7")
	language("C++")
	location(PROJECT_FOLDER)
	warnings("Extra")
	flags({"NoPCH", "Unicode"})
	platforms({"x86", "x64"})
	configurations({"Release", "Debug", "StaticRelease", "StaticDebug"})
	startproject("LuaInterface Test")

	filter("platforms:x86")
		architecture("x32")

	filter("platforms:x64")
		architecture("x64")

	filter("configurations:Release")
		kind("SharedLib")
		optimize("On")
		vectorextensions("SSE2")
		objdir(PROJECT_FOLDER .. "/intermediate")

		filter({"configurations:Release", "platforms:x86"})
			targetdir(PROJECT_FOLDER .. "/release x86")

		filter({"configurations:Release", "platforms:x64"})
			targetdir(PROJECT_FOLDER .. "/release x64")

	filter("configurations:Debug")
		kind("SharedLib")
		flags({"Symbols"})
		objdir(PROJECT_FOLDER .. "/intermediate")

		filter({"configurations:Debug", "platforms:x86"})
			targetdir(PROJECT_FOLDER .. "/debug x86")

		filter({"configurations:Debug", "platforms:x64"})
			targetdir(PROJECT_FOLDER .. "/debug x64")

	filter("configurations:StaticRelease")
		kind("StaticLib")
		optimize("On")
		vectorextensions("SSE2")
		objdir(PROJECT_FOLDER .. "/intermediate")

		filter({"configurations:StaticRelease", "options:static-runtime"})
			flags({"StaticRuntime"})

		filter({"configurations:StaticRelease", "platforms:x86"})
			targetdir(PROJECT_FOLDER .. "/static release x86")

		filter({"configurations:StaticRelease", "platforms:x64"})
			targetdir(PROJECT_FOLDER .. "/static release x64")

	filter("configurations:StaticDebug")
		kind("StaticLib")
		flags({"Symbols"})
		objdir(PROJECT_FOLDER .. "/intermediate")

		filter({"configurations:StaticDebug", "options:static-runtime"})
			flags({"StaticRuntime"})

		filter({"configurations:StaticDebug", "platforms:x86"})
			targetdir(PROJECT_FOLDER .. "/static debug x86")

		filter({"configurations:StaticDebug", "platforms:x64"})
			targetdir(PROJECT_FOLDER .. "/static debug x64")

	project("LuaInterface Test")
		uuid("0de8c190-5c82-448e-a1b7-504df8282d3a")
		kind("ConsoleApp")
		warnings("Extra")
		includedirs({INCLUDE_FOLDER})
		files({TESTING_FOLDER .. "/*.cpp", TESTING_FOLDER .. "/*.hpp"})
		vpaths({["Header files"] = "**.hpp", ["Source files"] = "**.cpp"})
		links({"LuaInterface"})

		filter("system:not windows")
			linkoptions({"-Wl,-R,./"})

			filter({"system:not windows", "configurations:Static*", "platforms:x86"})
				if LUA_API == "luajit" then
					libdirs({LIBS_FOLDER .. "/x86/static release"})
				end
				links(LIBS)

			filter({"system:not windows", "configurations:Static*", "platforms:x64"})
				if LUA_API == "luajit" then
					libdirs({LIBS_FOLDER .. "/x64/static release"})
				end
				links(LIBS)

	project("LuaInterface")
		uuid("41daacfe-a907-46f4-af95-ac52277ba07d")
		warnings("Extra")
		defines({"LUAINTERFACE_EXPORT"})
		includedirs({INCLUDE_FOLDER, THIRDPARTY_FOLDER, SOURCE_FOLDER})
		files({SOURCE_FOLDER .. "/*.cpp", INCLUDE_FOLDER .. "/*.hpp"})
		vpaths({["Header files"] = "**.hpp", ["Source files"] = "**.cpp"})

		filter({"configurations:not Static*", "platforms:x86"})
			if LUA_API == "luajit" then
				libdirs({LIBS_FOLDER .. "/x86/static release"})
			end
			links(LIBS)

		filter({"configurations:not Static*", "platforms:x64"})
			if LUA_API == "luajit" then
				libdirs({LIBS_FOLDER .. "/x64/static release"})
			end
			links(LIBS)

	if LUA_API == "lua" then
		project("Lua")
			uuid("43e3e08c-db0d-4d00-b05b-8bc264c4310f")
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
