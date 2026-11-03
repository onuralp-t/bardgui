@echo off
if /i "%~1"=="clean" goto clean

set SDL3=-I./thirdparty/SDL3-devel-3.4.18-VC/include -L./thirdparty/SDL3-devel-3.4.18-VC/lib/x64 -lSDL3
set CFLAGS=-std=c23 -Wall -Wextra -pedantic -fsanitize=address,undefined -g3 -Og
@echo on
clang -o main.exe main.c %CFLAGS% %SDL3% || exit /b 1
@echo off
for /f "delims=" %%I in ('clang -print-resource-dir') do for /f "delims=" %%F in ('dir /b /s "%%~I\*asan*.dll" 2^>nul') do copy /y "%%F" . >nul
start /wait "" main.exe
exit /b %ERRORLEVEL%

:clean
del /f /q main.exe main.pdb *asan*.dll 2>nul
exit /b 0
