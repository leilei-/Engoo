@echo off
set OUTPUT_DIR=lcc_lib_gl
cls
echo This script assumes that MASM is in your path
echo.
echo Creating object files (.obj) of all MASM assembler files (.asm) for GLQuake in %OUTPUT_DIR%
pause
cd %OUTPUT_DIR%
@echo on
ml /c /Cp /coff /Zm /Zi math.asm
ml /c /Cp /coff /Zm /Zi snd_mixa.asm
ml /c /Cp /coff /Zm /Zi sys_wina.asm
ml /c /Cp /coff /Zm /Zi worlda.asm
@echo off
cd ..
echo.
echo Step #4 for GLQuake is done
echo.
