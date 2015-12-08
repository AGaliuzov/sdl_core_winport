@echo off

:runcmake
pushd "%BUILDDIR%" > NUL
if %errorlevel% == 1 goto builddir_error
shift

set QT_TOOLCHAIN="msvc2010"
set QT_VERSION="5.5"

@echo on
pushd "%BUILDDIR%" > NUL
IF %_QT_PORT% == 1 (
   cmake -G %_SDL_GEN% -DCMAKE_INSTALL_PREFIX:PATH="%QT_HOME_DIR%\\%QT_VERSION%\\%QT_TOOLCHAIN%" -DQT_PORT=1 %SDL_ROOT% 
   @echo off
   popd
   goto end
) ELSE (
   cmake -G %_SDL_GEN% %SDL_ROOT% 
   @echo off
   popd
  goto end
)

:builddir_error
echo ERROR: Could not change to %BUILDDIR%
exit /b 1

:end
