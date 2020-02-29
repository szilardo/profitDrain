@echo off

:: cmd.exe /c build.bat

echo "==== Starting build."

pushd .build

set /p vcvarsall_path=<../.vcvarsall.bat.path
call "%vcvarsall_path%" x64
cl.exe /W3 ^
       /std:c++17 ^
       /Os ^
       /Oi %= enable intrinsics =% ^
       /GL %= enable link time optimization =% ^
       /GS- %= disable stack overflow security checks =% ^
       /nologo ^
       /Fe:"profitDrain" %= set executable name =% ^
       /I "../../sysroot/include/" %= set include search path =% ^
       /I "../code/public/include/profitDrain/" %= set include search path =% ^
       /I "../code/private/include/profitDrain/" %= set include search path =% ^
       "../code/src/resistance_is_futile.cpp" ^
       /link /LIBPATH:"../../sysroot/lib/" %= Search path for libraries =% ^
             /DEBUG ^
             /SUBSYSTEM:CONSOLE
SET build_result=%errorlevel%

copy /Y "..\code\public\include\*" "../../sysroot/include/"
copy /Y ".\profitDrain.exe" "../../sysroot/bin/"

popd

echo "==== Build finished with code: %build_result%"
exit %build_result%
