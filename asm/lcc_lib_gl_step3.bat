@echo off
set OUTPUT_DIR=lcc_lib_gl
cls
echo This script assumes that GAS2MASM is in ..\..\gas2masm\lcc_release
echo.
echo Creating MASM assembler files (.asm) of all precompiler files (.i) for GLQuake in %OUTPUT_DIR%
pause
@echo on
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\math.i     >%OUTPUT_DIR%\math.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\snd_mixa.i >%OUTPUT_DIR%\snd_mixa.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\sys_wina.i >%OUTPUT_DIR%\sys_wina.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\worlda.i   >%OUTPUT_DIR%\worlda.asm
@echo off
echo.
echo Step #3 for GLQuake is done
echo.
