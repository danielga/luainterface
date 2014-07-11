@if "%FrameworkDir%" == "" @goto error

@if "%Platform%" == "" @set "Platform=X86"

@cd "src"

@call "msvcbuild.bat"
@if not exist "..\build\windows\%Platform%\release\" @md "..\build\windows\%Platform%\release\"
@move "lua51.dll" "..\build\windows\%Platform%\release\lua51.dll"
@move "lua51.exp" "..\build\windows\%Platform%\release\lua51.exp"
@move "lua51.lib" "..\build\windows\%Platform%\release\lua51.lib"

@call "msvcbuild.bat" static
@if not exist "..\build\windows\%Platform%\static release\" @md "..\build\windows\%Platform%\static release\"
@move "lua51.lib" "..\build\windows\%Platform%\static release\lua51.lib"

@call "msvcbuild.bat" debug
@if not exist "..\build\windows\%Platform%\debug\" @md "..\build\windows\%Platform%\debug\"
@move "lua51.dll" "..\build\windows\%Platform%\debug\lua51.dll"
@move "lua51.exp" "..\build\windows\%Platform%\debug\lua51.exp"
@move "lua51.lib" "..\build\windows\%Platform%\debug\lua51.lib"
@move "lua51.pdb" "..\build\windows\%Platform%\debug\lua51.pdb"
@move "lua51.ilk" "..\build\windows\%Platform%\debug\lua51.ilk"
::@move "vc120.pdb" "..\build\windows\%Platform%\debug\vc120.pdb"

@call "msvcbuild.bat" debug static
@if not exist "..\build\windows\%Platform%\static debug\" @md "..\build\windows\%Platform%\static debug\"
@move "lua51.lib" "..\build\windows\%Platform%\static debug\lua51.lib"
::@move "vc120.pdb" "..\build\windows\%Platform%\static debug\vc120.pdb"

@cd ".."

@goto end

:error
@echo ERROR: You must call this file from a Visual Studio Developer Command Prompt.

:end