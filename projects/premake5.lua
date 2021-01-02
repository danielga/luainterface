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
	_OPTIONS["lua-api"] = "lua"
	LUA_API = "lua"
end

LUA_FOLDER = LUA_API

SOURCE_FOLDER = "../source"
INCLUDE_FOLDER = "../include"
THIRDPARTY_FOLDER = "../" .. LUA_FOLDER .. "/src"
MODULES_FOLDER = "../modules"
TESTING_FOLDER = "../testing"
PROJECT_FOLDER = os.target() .. "/" .. _ACTION
LIBS_FOLDER = "../" .. LUA_FOLDER .. "/build/" .. os.target()

if LUA_API == "lua" then
	if os.istarget("windows") then
		LIBS = "Lua"
	else
		LIBS = {"Lua", "dl"}
	end
elseif LUA_API == "luajit" then
	if os.istarget("windows") then
		LIBS = "lua51"
	else
		LIBS = {"luajit5.1", "dl"}
	end
end

solution("LuaInterface")
	uuid("602804da-edd2-4767-93f0-8e3f905b06a7")
	language("C++")
	location(PROJECT_FOLDER)
	warnings("Extra")
	flags("NoPCH")
	characterset("MBCS")
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
		symbols("On")
		objdir(PROJECT_FOLDER .. "/intermediate")

		filter({"configurations:Debug", "platforms:x86"})
			targetdir(PROJECT_FOLDER .. "/debug x86")

		filter({"configurations:Debug", "platforms:x64"})
			targetdir(PROJECT_FOLDER .. "/debug x64")

	filter("configurations:StaticRelease")
		kind("StaticLib")
		defines("LUAINTERFACE_STATIC")
		optimize("On")
		vectorextensions("SSE2")
		objdir(PROJECT_FOLDER .. "/intermediate")

		filter({"configurations:StaticRelease", "options:static-runtime"})
			flags("StaticRuntime")

		filter({"configurations:StaticRelease", "platforms:x86"})
			targetdir(PROJECT_FOLDER .. "/static release x86")

		filter({"configurations:StaticRelease", "platforms:x64"})
			targetdir(PROJECT_FOLDER .. "/static release x64")

	filter("configurations:StaticDebug")
		kind("StaticLib")
		defines("LUAINTERFACE_STATIC")
		symbols("On")
		objdir(PROJECT_FOLDER .. "/intermediate")

		filter({"configurations:StaticDebug", "options:static-runtime"})
			flags("StaticRuntime")

		filter({"configurations:StaticDebug", "platforms:x86"})
			targetdir(PROJECT_FOLDER .. "/static debug x86")

		filter({"configurations:StaticDebug", "platforms:x64"})
			targetdir(PROJECT_FOLDER .. "/static debug x64")

	project("LuaInterface Test")
		uuid("0de8c190-5c82-448e-a1b7-504df8282d3a")
		kind("ConsoleApp")
		includedirs(INCLUDE_FOLDER)
		files({TESTING_FOLDER .. "/*.cpp", TESTING_FOLDER .. "/*.hpp"})
		vpaths({
			["Header files"] = TESTING_FOLDER .. "/**.hpp",
			["Source files"] = TESTING_FOLDER .. "/**.cpp"
		})
		links("LuaInterface")

		--filter("system:not windows")
		--	linkoptions("-Wl,-R,./")

		filter("configurations:StaticDebug or StaticRelease")
			links(LIBS)

		filter({"system:windows", "options:lua-api=luajit", "platforms:x86", "configurations:StaticDebug", "options:static-runtime"})
			libdirs(LIBS_FOLDER .. "/x86/static debug")

		filter({"system:windows", "options:lua-api=luajit", "platforms:x86", "configurations:StaticDebug", "options:not static-runtime"})
			libdirs(LIBS_FOLDER .. "/x86/debug")

		filter({"system:windows", "options:lua-api=luajit", "platforms:x86", "configurations:StaticRelease", "options:static-runtime"})
			libdirs(LIBS_FOLDER .. "/x86/static release")

		filter({"system:windows", "options:lua-api=luajit", "platforms:x86", "configurations:StaticRelease", "options:not static-runtime"})
			libdirs(LIBS_FOLDER .. "/x86/release")

		filter({"system:windows", "options:lua-api=luajit", "platforms:x64", "configurations:StaticDebug", "options:static-runtime"})
			libdirs(LIBS_FOLDER .. "/x64/static debug")

		filter({"system:windows", "options:lua-api=luajit", "platforms:x64", "configurations:StaticDebug", "options:not static-runtime"})
			libdirs(LIBS_FOLDER .. "/x64/debug")

		filter({"system:windows", "options:lua-api=luajit", "platforms:x64", "configurations:StaticRelease", "options:static-runtime"})
			libdirs(LIBS_FOLDER .. "/x64/static release")

		filter({"system:windows", "options:lua-api=luajit", "platforms:x64", "configurations:StaticRelease", "options:not static-runtime"})
			libdirs(LIBS_FOLDER .. "/x64/release")

	project("LuaInterface")
		uuid("41daacfe-a907-46f4-af95-ac52277ba07d")
		includedirs({INCLUDE_FOLDER, THIRDPARTY_FOLDER, SOURCE_FOLDER})
		files({SOURCE_FOLDER .. "/*.cpp", INCLUDE_FOLDER .. "/*.hpp"})
		vpaths({
			["Header files"] = INCLUDE_FOLDER .. "/**.hpp",
			["Source files"] = SOURCE_FOLDER .. "/**.cpp"
		})

		filter("configurations:Debug or Release")
			defines("LUAINTERFACE_EXPORT")
			links(LIBS)

		filter({"options:lua-api=luajit", "platforms:x86", "configurations:Debug"})
			libdirs(LIBS_FOLDER .. "/x86/debug")

		filter({"options:lua-api=luajit", "platforms:x86", "configurations:Release"})
			libdirs(LIBS_FOLDER .. "/x86/release")

		filter({"options:lua-api=luajit", "platforms:x64", "configurations:Debug"})
			libdirs(LIBS_FOLDER .. "/x64/debug")

		filter({"options:lua-api=luajit", "platforms:x64", "configurations:Release"})
			libdirs(LIBS_FOLDER .. "/x64/release")

		filter("options:lua-api=lua")
			defines("LUA_COMPAT_MODULE")

	if LUA_API == "lua" then
		project("Lua")
			uuid("43e3e08c-db0d-4d00-b05b-8bc264c4310f")
			kind("StaticLib")
			compileas("C++")
			defines("LUA_COMPAT_MODULE")
			includedirs(THIRDPARTY_FOLDER)
			files({THIRDPARTY_FOLDER .. "/*.c", THIRDPARTY_FOLDER .. "/*.h"})
			vpaths({
				["Header files"] = THIRDPARTY_FOLDER .. "/**.h",
				["Source files"] = THIRDPARTY_FOLDER .. "/**.c"
			})
	end

	INCLUDE_FOLDER = "../" .. INCLUDE_FOLDER
	THIRDPARTY_FOLDER = "../" .. THIRDPARTY_FOLDER

	local modules = os.matchdirs(MODULES_FOLDER .. "/*")
	for _, folder in pairs(modules) do
		include(folder)
	end
