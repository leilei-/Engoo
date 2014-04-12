@echo off
set OUTPUT_DIR=lcc_lib_wq
cls
echo This script assumes that MASM is in your path
echo.
echo Creating object files (.obj) of all MASM assembler files (.asm) for WinQuake in %OUTPUT_DIR%
pause
cd %OUTPUT_DIR%
@echo on
ml /c /Cp /coff /Zm /Zi d_draw.asm  
ml /c /Cp /coff /Zm /Zi d_draw16.asm
ml /c /Cp /coff /Zm /Zi d_parta.asm 
ml /c /Cp /coff /Zm /Zi d_polysa.asm
ml /c /Cp /coff /Zm /Zi d_scana.asm 
ml /c /Cp /coff /Zm /Zi d_spr8.asm  
ml /c /Cp /coff /Zm /Zi d_varsa.asm 
ml /c /Cp /coff /Zm /Zi math.asm    
ml /c /Cp /coff /Zm /Zi r_aclipa.asm
ml /c /Cp /coff /Zm /Zi r_aliasa.asm
ml /c /Cp /coff /Zm /Zi r_drawa.asm 
ml /c /Cp /coff /Zm /Zi r_edgea.asm 
ml /c /Cp /coff /Zm /Zi r_varsa.asm 
ml /c /Cp /coff /Zm /Zi snd_mixa.asm
ml /c /Cp /coff /Zm /Zi surf16.asm  
ml /c /Cp /coff /Zm /Zi surf8.asm   
ml /c /Cp /coff /Zm /Zi sys_wina.asm
ml /c /Cp /coff /Zm /Zi worlda.asm  
@echo off
cd ..
echo.
echo Step #4 for WinQuake is done
echo.
