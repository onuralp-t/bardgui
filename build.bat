clang -o main.exe main.c -std=c23 -Wall -Wextra -pedantic -fsanitize=address,undefined -g3 -Og || exit /b 1
for /f "delims=" %%I in ('clang -print-resource-dir') do for /f "delims=" %%F in ('dir /b /s "%%~I\*asan*.dll" 2^>nul') do copy /y "%%F" . >nul
start /wait "" main.exe
set MAIN_EXIT=%ERRORLEVEL%
if %MAIN_EXIT% EQU 0 (
    del /f /q main.exe main.pdb *asan*.dll 2>nul
)
exit /b %MAIN_EXIT%
