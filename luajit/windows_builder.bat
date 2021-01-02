@if "%FrameworkDir%" == "" @goto error

@cd "src"

@set "Platform=X86"

@call "msvcbuild2.bat"
@if not exist "..\build\windows\%Platform%\release\" @md "..\build\windows\%Platform%\release\"
@move "lua51.lib" "..\build\windows\%Platform%\release\lua51.lib"

@call "msvcbuild2.bat" static
@if not exist "..\build\windows\%Platform%\static release\" @md "..\build\windows\%Platform%\static release\"
@move "lua51.lib" "..\build\windows\%Platform%\static release\lua51.lib"

@call "msvcbuild2.bat" debug
@if not exist "..\build\windows\%Platform%\debug\" @md "..\build\windows\%Platform%\debug\"
@move "lua51.lib" "..\build\windows\%Platform%\debug\lua51.lib"
@move "vc140.pdb" "..\build\windows\%Platform%\debug\vc140.pdb"

@call "msvcbuild2.bat" debug static
@if not exist "..\build\windows\%Platform%\static debug\" @md "..\build\windows\%Platform%\static debug\"
@move "lua51.lib" "..\build\windows\%Platform%\static debug\lua51.lib"
@move "vc140.pdb" "..\build\windows\%Platform%\static debug\vc140.pdb"

@set "Platform=X64"

@call "msvcbuild2.bat"
@if not exist "..\build\windows\%Platform%\release\" @md "..\build\windows\%Platform%\release\"
@move "lua51.lib" "..\build\windows\%Platform%\release\lua51.lib"

@call "msvcbuild2.bat" static
@if not exist "..\build\windows\%Platform%\static release\" @md "..\build\windows\%Platform%\static release\"
@move "lua51.lib" "..\build\windows\%Platform%\static release\lua51.lib"

@call "msvcbuild2.bat" debug
@if not exist "..\build\windows\%Platform%\debug\" @md "..\build\windows\%Platform%\debug\"
@move "lua51.lib" "..\build\windows\%Platform%\debug\lua51.lib"
@move "vc140.pdb" "..\build\windows\%Platform%\debug\vc140.pdb"

@call "msvcbuild2.bat" debug static
@if not exist "..\build\windows\%Platform%\static debug\" @md "..\build\windows\%Platform%\static debug\"
@move "lua51.lib" "..\build\windows\%Platform%\static debug\lua51.lib"
@move "vc140.pdb" "..\build\windows\%Platform%\static debug\vc140.pdb"

@goto end

:error
@echo ERROR: You must call this file from a Visual Studio Developer Command Prompt.

:end
@cd ".."
