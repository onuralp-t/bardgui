@echo off
if /i "%~1"=="clean" goto clean

set SDL3=-I./thirdparty/SDL3-devel-3.4.18-VC/include -L./thirdparty/SDL3-devel-3.4.18-VC/lib/x64 -lSDL3
set SDL3_ttf=-I./thirdparty/SDL3_ttf-3.2.2/include -L./thirdparty/SDL3_ttf-3.2.2/lib/x64 -lSDL3_ttf
set CFLAGS=-std=c23 -Wall -Wextra -pedantic -fsanitize=address,undefined -g3 -Og

@echo on
clang -o main.exe main.c %CFLAGS% %SDL3% %SDL3_ttf% || exit /b 1
@echo off

for /f "delims=" %%I in ('clang -print-resource-dir') do for /f "delims=" %%F in ('dir /b /s "%%~I\*asan*.dll" 2^>nul') do set "ASAN_DIR=%%~I\lib\windows"
set "PATH=%ASAN_DIR%;%~dp0thirdparty\SDL3-devel-3.4.18-VC\lib\x64;%~dp0thirdparty\SDL3_ttf-3.2.2\lib\x64;%PATH%"

rem start /wait "" main.exe
main.exe
exit /b %ERRORLEVEL%

:clean
del /f /q main.exe main.pdb 2>nul
exit /b 0
