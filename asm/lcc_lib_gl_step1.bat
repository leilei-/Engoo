@echo off
set OUTPUT_DIR=lcc_lib_gl
cls
echo This script assumes that LCC is in your path
echo.
echo Creating preprocessor assembler files (.i) of all GAS files (.s) for GLQuake in %OUTPUT_DIR%
pause
mkdir %OUTPUT_DIR%
del "%OUTPUT_DIR%\*.i"
@echo on
lcc -EP -DGLQUAKE math.s     -Fo%OUTPUT_DIR%/math.i
lcc -EP -DGLQUAKE snd_mixa.s -Fo%OUTPUT_DIR%/snd_mixa.i
lcc -EP -DGLQUAKE sys_wina.s -Fo%OUTPUT_DIR%/sys_wina.i
lcc -EP -DGLQUAKE worlda.s   -Fo%OUTPUT_DIR%/worlda.i
@echo off
echo.
echo Step #1 for GLQuake is done
echo.
