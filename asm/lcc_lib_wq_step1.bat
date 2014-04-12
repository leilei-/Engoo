@echo off
set OUTPUT_DIR=lcc_lib_wq
cls
echo This script assumes that LCC is in your path
echo.
echo Creating preprocessor assembler files (.i) of all GAS files (.s) for WinQuake in %OUTPUT_DIR%
pause
mkdir %OUTPUT_DIR%
del "%OUTPUT_DIR%\*.i"
@echo on
lcc -EP d_draw.s   -Fo%OUTPUT_DIR%/d_draw.i
lcc -EP d_draw16.s -Fo%OUTPUT_DIR%/d_draw16.i
lcc -EP d_parta.s  -Fo%OUTPUT_DIR%/d_parta.i
lcc -EP d_polysa.s -Fo%OUTPUT_DIR%/d_polysa.i
lcc -EP d_scana.s  -Fo%OUTPUT_DIR%/d_scana.i
lcc -EP d_spr8.s   -Fo%OUTPUT_DIR%/d_spr8.i
lcc -EP d_varsa.s  -Fo%OUTPUT_DIR%/d_varsa.i
lcc -EP math.s     -Fo%OUTPUT_DIR%/math.i
lcc -EP r_aclipa.s -Fo%OUTPUT_DIR%/r_aclipa.i
lcc -EP r_aliasa.s -Fo%OUTPUT_DIR%/r_aliasa.i
lcc -EP r_drawa.s  -Fo%OUTPUT_DIR%/r_drawa.i
lcc -EP r_edgea.s  -Fo%OUTPUT_DIR%/r_edgea.i
lcc -EP r_varsa.s  -Fo%OUTPUT_DIR%/r_varsa.i
lcc -EP snd_mixa.s -Fo%OUTPUT_DIR%/snd_mixa.i
lcc -EP surf16.s   -Fo%OUTPUT_DIR%/surf16.i
lcc -EP surf8.s    -Fo%OUTPUT_DIR%/surf8.i
lcc -EP sys_wina.s -Fo%OUTPUT_DIR%/sys_wina.i
lcc -EP worlda.s   -Fo%OUTPUT_DIR%/worlda.i
@echo off
echo.
echo Step #1 for WinQuake is done
echo.
