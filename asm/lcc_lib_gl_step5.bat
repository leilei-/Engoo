@echo off
set OUTPUT_DIR=lcc_lib_gl
cls
echo This script assumes that LCC is in your path
echo.
echo Creating library (.lib) of object files for GLQuake in %OUTPUT_DIR%
pause
cd %OUTPUT_DIR%
@echo on
lcclib quakeasm_gl.lib *.obj
@echo off
cd ..
echo.
echo Step #5 for GLQuake is done
echo.
