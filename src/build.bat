@echo off

if not exist ..\build mkdir ..\build
pushd ..\build

set IncludeDirectory=E:\Libs\SDL2\include
set LibraryDirectory=E:\Libs\SDL2\lib\x64

set Libraries=SDL2.lib SDL2main.lib

REM Zi(Generate Debug information), FC(Full Path To Source), O2(Fast Code)

set CompilerOptions=-FC -Zi -EHsc -MD /I%IncludeDirectory% /IE:\Projekty\jpb /FeSoftRenderer.exe /nologo
set LinkerOptions=/link /SUBSYSTEM:windows /LIBPATH:%LibraryDirectory%

set FilesToCompile=..\src\main.cpp ..\src\Game.cpp

cl %CompilerOptions% %FilesToCompile% %Libraries% %LinkerOptions%

popd