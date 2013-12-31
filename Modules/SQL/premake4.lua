project("LuaInterface-MySQL")
	kind("SharedLib")
	includedirs({"Source", LUA_INCLUDES, "Modules/SQL"})
	files({"Modules/SQL/ls_mysql.c", "Modules/SQL/luasql.c", "Modules/SQL/luasql.h"})
	vpaths({["Header files"] = {"**.h"}, ["Source files"] = {"**.c"}})
	links({"LuaInterface"})
	if os.is("windows") then
		includedirs({"Modules/SQL/mysql/include"})
		libdirs({"Modules/SQL/mysql/lib"})
		links({"libmysql"})
	else
		-- So much differences between Linuxes... This tires me.
		includedirs({"/usr/include/mysql"})
		libdirs({"/usr/lib/mysql"})
		links({"mysqlclient"})
	end
	targetprefix("")
	targetname("mysql")
	targetsuffix("")

project("LuaInterface-PgSQL")
	kind("SharedLib")
	includedirs({"Source", LUA_INCLUDES, "Modules/SQL"})
	files({"Modules/SQL/ls_postgres.c", "Modules/SQL/luasql.c", "Modules/SQL/luasql.h"})
	vpaths({["Header files"] = {"**.h"}, ["Source files"] = {"**.c"}})
	links({"LuaInterface"})
	if os.is("windows") then
		includedirs({"Modules/SQL/pgsql/include"})
		libdirs({"Modules/SQL/pgsql/lib"})
		links({"libpq"})
	else
		-- So much differences between Linuxes... This tires me.
		includedirs({"/usr/include/postgresql"})
		links({"pq"})
	end
	targetprefix("")
	targetname("postgres")
	targetsuffix("")

project("LuaInterface-SQLite")
	kind("SharedLib")
	includedirs({"Source", LUA_INCLUDES, "Modules/SQL", "Modules/SQL/sqlite"})
	files({"Modules/SQL/ls_sqlite3.c", "Modules/SQL/luasql.c", "Modules/SQL/luasql.h", "Modules/SQL/sqlite/*.c", "Modules/SQL/sqlite/*.h"})
	vpaths({["Header files"] = {"**.h"}, ["Source files"] = {"**.c"}})
	links({"LuaInterface"})
	if not os.is("windows") then
		links({"pthread"})
	end
	targetprefix("")
	targetname("sqlite")
	targetsuffix("")

project("LuaInterface-ODBC")
	kind("SharedLib")
	includedirs({"Source", LUA_INCLUDES, "Modules/SQL"})
	files({"Modules/SQL/ls_odbc.c", "Modules/SQL/luasql.c", "Modules/SQL/luasql.h"})
	vpaths({["Header files"] = {"**.h"}, ["Source files"] = {"**.c"}})
	links({"LuaInterface"})
	if not os.is("windows") then
		defines({"UNIXODBC"})
		links({"odbc"})
	end
	targetprefix("")
	targetname("odbc")
	targetsuffix("")