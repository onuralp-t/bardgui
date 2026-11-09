for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64

cmake -S thirdparty/SDL-release-3.4.20 -B thirdparty/SDL-release-3.4.20/build ^
  -DSDL_STATIC=ON ^
  -DSDL_SHARED=OFF ^
  -DSDL_LIBC=OFF ^
  -DSDL_TESTS=OFF ^
  -DSDL_TEST_LIBRARY=OFF ^
  -DSDL_EXAMPLES=OFF ^
  -DCMAKE_INSTALL_PREFIX="thirdparty/install" ^
  -DCMAKE_C_FLAGS_RELEASE="/O2 /Oi /Gy /GS- /Zl /DNDEBUG"

cmake --build thirdparty/SDL-release-3.4.20/build --config Release --parallel

cmake --install thirdparty/SDL-release-3.4.20/build --config Release

if not exist "thirdparty\SDL_ttf-release-3.2.2\external\freetype\CMakeLists.txt" (
    echo Downloading SDL_ttf external dependencies...
    powershell -ExecutionPolicy Bypass -File "thirdparty\SDL_ttf-release-3.2.2\external\Get-GitModules.ps1"
)

cmake -S thirdparty/SDL_ttf-release-3.2.2 -B thirdparty/SDL_ttf-release-3.2.2/build ^
  -DSDL3_DIR="thirdparty/SDL-release-3.4.20/build" ^
  -DBUILD_SHARED_LIBS=OFF ^
  -DSDLTTF_VENDORED=ON ^
  -DSDL_LIBC=OFF ^
  -DCMAKE_INSTALL_PREFIX="thirdparty/install" ^
  -DCMAKE_C_FLAGS_RELEASE="/O2 /Oi /Gy /GS- /Zl /DNDEBUG"

cmake --build thirdparty/SDL_ttf-release-3.2.2/build --config Release --parallel

cmake --install thirdparty/SDL-release-3.4.20/install --config Release
