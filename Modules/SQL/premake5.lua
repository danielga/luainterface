project("LuaInterface-MySQL")
	kind("SharedLib")
	includedirs({INCLUDE_FOLDER, THIRDPARTY_FOLDER})
	files({"ls_mysql.c", "luasql.c", "luasql.h"})
	vpaths({["Header files"] = "**.h", ["Source files"] = "**.c"})
	links({"LuaInterface", "mysql"})

	targetprefix("")
	targetname("mysql")
	targetsuffix("")

	configuration({"windows", "x32"})
		includedirs({"mysql/include"})
		libdirs({"mysql/lib/x86"})

	configuration({"windows", "x64"})
		includedirs({"mysql/include"})
		libdirs({"mysql/lib/x64"})

project("LuaInterface-PgSQL")
	kind("SharedLib")
	includedirs({INCLUDE_FOLDER, THIRDPARTY_FOLDER})
	files({"ls_postgres.c", "luasql.c", "luasql.h"})
	vpaths({["Header files"] = "**.h", ["Source files"] = "**.c"})
	links({"LuaInterface", "pq"})

	targetprefix("")
	targetname("postgres")
	targetsuffix("")

	configuration({"windows", "x32"})
		includedirs({"pgsql/include"})
		libdirs({"pgsql/lib/x86"})

	configuration({"windows", "x64"})
		includedirs({"pgsql/include"})
		libdirs({"pgsql/lib/x64"})

project("LuaInterface-SQLite")
	kind("SharedLib")
	includedirs({INCLUDE_FOLDER, THIRDPARTY_FOLDER, "sqlite"})
	files({"ls_sqlite3.c", "luasql.c", "luasql.h", "sqlite/*.c", "sqlite/*.h"})
	vpaths({["Header files"] = "**.h", ["Source files"] = "**.c"})
	links({"LuaInterface"})

	targetprefix("")
	targetname("sqlite")
	targetsuffix("")

	configuration("not windows")
		links({"pthread"})

project("LuaInterface-ODBC")
	kind("SharedLib")
	includedirs({INCLUDE_FOLDER, THIRDPARTY_FOLDER})
	files({"ls_odbc.c", "luasql.c", "luasql.h"})
	vpaths({["Header files"] = "**.h", ["Source files"] = "**.c"})
	links({"LuaInterface"})

	targetprefix("")
	targetname("odbc")
	targetsuffix("")

	configuration("windows")
		links({"odbc32"})

	configuration("not windows")
		defines({"UNIXODBC"})
		links({"odbc"})