local args = {...}
local luaversion = args[1]
local reallib = args[2]
local fakelib = args[3]
local compiler = args[4]
local machine = args[5]

local extrasymbols = {}
for i = 6, #args do
	table.insert(extrasymbols, args[i])
end

local errmsg
local supported_compilers = {vc = true, gcc = true}
local supported_machines = {x86 = true, amd64 = true, ia64 = true}
if #args < 5 then
	errmsg = "not enough parameters"
elseif not supported_compilers[compiler:lower()] then
	errmsg = "'" .. compiler .. "' is not a recognized compiler"
elseif not supported_machines[machine:lower()] then
	errmsg = "'" .. machine .. "' is not a recognized machine architecture"
end

if errmsg then
	io.stderr:write([[
Error: ]] .. errmsg .. [[

Usage: mkforwardlib <luaversion> <reallib> <fakelib> <compiler> <machine> [<extrasymbols>...]

	luaversion		The version of Lua you use so the correct symbols are
					exported among:
					51, 52, jit
	reallib			The name of the existing dll that you want to use.
	fakelib			The name of the fake dll that you want to create and that
					will call the reallib instead.
	compiler		What compiler to use to make the dll among:
					vc, gcc
	machine			The hardware architecture of your windows version among:
					x86, amd64, ia64
	extrasymbols	Additionnal symbols to export beyond standard Lua symbols
					(if you have a very custom Lua dll).

Example: mkforwardlib 51 lua51 lua5.1 vc x86
]])
	os.exit(0)
end

local symbols = {
	common = {
		"luaL_addlstring",
		"luaL_addstring",
		"luaL_addvalue",
		"luaL_argerror",
		"luaL_buffinit",
		"luaL_callmeta",
		"luaL_checkany",
		"luaL_checkinteger",
		"luaL_checklstring",
		"luaL_checknumber",
		"luaL_checkoption",
		"luaL_checkstack",
		"luaL_checktype",
		"luaL_checkudata",
		"luaL_error",
		"luaL_getmetafield",
		"luaL_gsub",
		"luaL_loadstring",
		"luaL_newmetatable",
		"luaL_newstate",
		"luaL_openlib",
		"luaL_openlibs",
		"luaL_optinteger",
		"luaL_optlstring",
		"luaL_optnumber",
		"luaL_pushresult",
		"luaL_ref",
		"luaL_unref",
		"luaL_where",
		"lua_atpanic",
		"lua_checkstack",
		"lua_close",
		"lua_concat",
		"lua_createtable",
		"lua_dump",
		"lua_error",
		"lua_gc",
		"lua_getallocf",
		"lua_getfield",
		"lua_gethook",
		"lua_gethookcount",
		"lua_gethookmask",
		"lua_getinfo",
		"lua_getlocal",
		"lua_getmetatable",
		"lua_getstack",
		"lua_gettable",
		"lua_gettop",
		"lua_getupvalue",
		"lua_insert",
		"lua_iscfunction",
		"lua_isnumber",
		"lua_isstring",
		"lua_isuserdata",
		"lua_load",
		"lua_newstate",
		"lua_newthread",
		"lua_newuserdata",
		"lua_next",
		"lua_pushboolean",
		"lua_pushcclosure",
		"lua_pushfstring",
		"lua_pushinteger",
		"lua_pushlightuserdata",
		"lua_pushlstring",
		"lua_pushnil",
		"lua_pushnumber",
		"lua_pushstring",
		"lua_pushthread",
		"lua_pushvalue",
		"lua_pushvfstring",
		"lua_rawequal",
		"lua_rawget",
		"lua_rawgeti",
		"lua_rawset",
		"lua_rawseti",
		"lua_remove",
		"lua_replace",
		"lua_resume",
		"lua_setallocf",
		"lua_setfield",
		"lua_sethook",
		"lua_setlocal",
		"lua_setmetatable",
		"lua_settable",
		"lua_settop",
		"lua_setupvalue",
		"lua_status",
		"lua_toboolean",
		"lua_tocfunction",
		"lua_tolstring",
		"lua_topointer",
		"lua_tothread",
		"lua_touserdata",
		"lua_type",
		"lua_typename",
		"lua_xmove",
		--"luaopen_base",
		--"luaopen_debug",
		--"luaopen_io",
		--"luaopen_math",
		--"luaopen_os",
		--"luaopen_package",
		--"luaopen_string",
		--"luaopen_table"
	},

	["51"] = {
		"luaL_findtable",
		"luaL_loadbuffer",
		"luaL_loadfile",
		"luaL_prepbuffer",
		"luaL_register",
		"luaL_typerror",
		"lua_call",
		"lua_cpcall",
		"lua_equal",
		"lua_getfenv",
		"lua_lessthan",
		"lua_objlen",
		"lua_pcall",
		"lua_setfenv",
		"lua_tointeger",
		"lua_tonumber",
		"lua_yield"
	},

	["52"] = {
		"luaL_buffinitsize",
		"luaL_checkunsigned",
		"luaL_checkversion_",
		"luaL_execresult",
		"luaL_fileresult",
		"luaL_getsubtable",
		"luaL_len",
		"luaL_loadbufferx",
		"luaL_loadfilex",
		"luaL_optunsigned",
		"luaL_prepbuffsize",
		"luaL_pushmodule",
		"luaL_pushresultsize",
		"luaL_requiref",
		"luaL_setfuncs",
		"luaL_setmetatable",
		"luaL_testudata",
		"luaL_tolstring",
		"luaL_traceback",
		"lua_absindex",
		"lua_arith",
		"lua_callk",
		"lua_compare",
		"lua_copy",
		"lua_getctx",
		"lua_getglobal",
		"lua_getuservalue",
		"lua_len",
		"lua_pcallk",
		"lua_pushunsigned",
		"lua_rawgetp",
		"lua_rawlen",
		"lua_rawsetp",
		"lua_setglobal",
		"lua_setuservalue",
		"lua_tointegerx",
		"lua_tonumberx",
		"lua_tounsignedx",
		"lua_upvalueid",
		"lua_upvaluejoin",
		"lua_version",
		"lua_yieldk",
		--"luaopen_bit32",
		--"luaopen_coroutine"
	},

	["jit"] = {
		"",
		""
	}
}

local compilers = {
	vc = function()
		local command = "link -dll -nologo -noentry -machine:%s -incremental:no -nodefaultlib -out:%s.dll -implib:%s.lib"
		command = command:format(machine, fakelib, fakelib)

		local fmt = "%s /export:%s=%s.%s"
		for _, symbol in ipairs(symbols.common) do
			command = fmt:format(command, symbol, reallib, symbol)
		end

		for _, symbol in ipairs(symbols[luaversion]) do
			command = fmt:format(command, symbol, reallib, symbol)
		end

		for _, symbol in ipairs(extrasymbols) do
			command = fmt:format(command, symbol, reallib, symbol)
		end

		os.execute("\"" .. command .. "\"")
	end,

	gcc = function()
		local def = io.open("lib" .. fakelib .. ".def", "wb")

		def:write("EXPORTS\n")

		for _, symbol in ipairs(symbols.common) do
			def:write(symbol .. "=" .. reallib .. "." .. symbol .. "\n")
		end

		for _, symbol in ipairs(symbols[luaversion]) do
			def:write(symbol .. "=" .. reallib .. "." .. symbol .. "\n")
		end

		for _, symbol in ipairs(extrasymbols) do
			def:write(symbol .. "=" .. reallib .. "." .. symbol .. "\n")
		end

		def:close()

		os.execute("\"gcc -o " .. fakelib .. ".dll -nostartfiles lib" .. fakelib .. ".def\"")

		os.execute("\"strip --strip-unneeded " .. fakelib .. ".dll\"")
	end
}

compilers[compiler]()
