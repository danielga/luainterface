call "D:\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86

cd "src"

call msvcbuild.bat
move "lua51.dll" "..\build\windows\x32\release\lua51.dll"
move "lua51.exp" "..\build\windows\x32\release\lua51.exp"
move "lua51.lib" "..\build\windows\x32\release\lua51.lib"

call msvcbuild.bat static
move "lua51.lib" "..\build\windows\x32\static release\lua51.lib"

call msvcbuild.bat debug
move "lua51.dll" "..\build\windows\x32\debug\lua51.dll"
move "lua51.exp" "..\build\windows\x32\debug\lua51.exp"
move "lua51.lib" "..\build\windows\x32\debug\lua51.lib"
move "lua51.pdb" "..\build\windows\x32\debug\lua51.pdb"
move "lua51.ilk" "..\build\windows\x32\debug\lua51.ilk"
move "vc100.pdb" "..\build\windows\x32\debug\vc100.pdb"

call msvcbuild.bat debug static
move "lua51.lib" "..\build\windows\x32\static debug\lua51.lib"
move "vc100.pdb" "..\build\windows\x32\static debug\vc100.pdb"

call "D:\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64

cd "src"

call msvcbuild.bat 
move "lua51.dll" "..\build\windows\x64\release\lua51.dll"
move "lua51.exp" "..\build\windows\x64\release\lua51.exp"
move "lua51.lib" "..\build\windows\x64\release\lua51.lib"

call msvcbuild.bat static
move "lua51.lib" "..\build\windows\x64\static release\lua51.lib"

call msvcbuild.bat debug
move "lua51.dll" "..\build\windows\x64\debug\lua51.dll"
move "lua51.exp" "..\build\windows\x64\debug\lua51.exp"
move "lua51.lib" "..\build\windows\x64\debug\lua51.lib"
move "lua51.pdb" "..\build\windows\x64\debug\lua51.pdb"
move "lua51.ilk" "..\build\windows\x64\debug\lua51.ilk"
move "vc100.pdb" "..\build\windows\x64\debug\vc100.pdb"

call msvcbuild.bat debug static
move "lua51.lib" "..\build\windows\x64\static debug\lua51.lib"
move "vc100.pdb" "..\build\windows\x64\static debug\vc100.pdb"

pause