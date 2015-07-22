@echo off

if not exist ..\build mkdir ..\build
pushd ..\build

set IncludeDirectory=E:\Libs\SDL2\include
set LibraryDirectory=E:\Libs\SDL2\lib\x64

set Libraries=SDL2.lib SDL2main.lib

REM Zi(Generate Debug information), FC(Full Path To Source), O2(Fast Code)

set CompilerOptions=-FC -O2x -Zi -EHsc -MD /I%IncludeDirectory% /IE:\Projekty\jpb /FeSoftRenderer.exe /nologo
set LinkerOptions=/link /SUBSYSTEM:windows /LIBPATH:%LibraryDirectory%

REM /HEAP:1000000000

set FilesToCompile=^
..\src\main.cpp ^
..\src\Game.cpp ^
..\src\SoftRenderer.cpp ^
..\src\RenderPrimitives.cpp ^
..\src\Camera.cpp 

cl %CompilerOptions% %FilesToCompile% %Libraries% %LinkerOptions%

popd