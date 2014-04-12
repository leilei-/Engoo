@echo off
set OUTPUT_DIR=lcc_lib_wq
cls
echo This script assumes that LCC is in your path
echo.
echo Creating library (.lib) of object files for WinQuake in %OUTPUT_DIR%
pause
cd %OUTPUT_DIR%
@echo on
lcclib quakeasm_wq.lib *.obj
@echo off
cd ..
echo.
echo Step #5 for WinQuake is done
echo.
