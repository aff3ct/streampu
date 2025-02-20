@echo on

call ./ci/tools/threads.bat

set "VSCMD_START_DIR=%CD%"
call "c:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

cmake --version
mkdir build
cd build
cmake .. -G"Visual Studio 17 2022" -A x64  %CMAKE_OPT% -DCMAKE_CXX_FLAGS="%CFLAGS% /MP%THREADS%" -DCMAKE_INSTALL_PREFIX="%NAME%" -DCMAKE_INCLUDE_PATH=%CMAKE_INCLUDE_PATH% -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH%
if %ERRORLEVEL% neq 0 exit /B %ERRORLEVEL%
rem devenv /build Release streampu.sln
msbuild streampu.sln /t:Build /p:Configuration=Release
if %ERRORLEVEL% neq 0 exit /B %ERRORLEVEL%
cd ..
move build %NAME%

rem Comment : impossible to run the INSTALL target with MSBuild
rem rd /s /q %NAME%
rem devenv /build Release streampu.sln /project INSTALL > nul
rem if %ERRORLEVEL% neq 0 exit %ERRORLEVEL%
rem move %NAME% ..\
