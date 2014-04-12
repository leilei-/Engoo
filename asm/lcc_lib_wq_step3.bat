@echo off
set OUTPUT_DIR=lcc_lib_wq
cls
echo This script assumes that GAS2MASM is in ..\..\gas2masm\lcc_release
echo.
echo Creating MASM assembler files (.asm) of all precompiler files (.i) for WinQuake in %OUTPUT_DIR%
pause
@echo on
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\d_draw.i   >%OUTPUT_DIR%\d_draw.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\d_draw16.i >%OUTPUT_DIR%\d_draw16.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\d_parta.i  >%OUTPUT_DIR%\d_parta.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\d_polysa.i >%OUTPUT_DIR%\d_polysa.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\d_scana.i  >%OUTPUT_DIR%\d_scana.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\d_spr8.i   >%OUTPUT_DIR%\d_spr8.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\d_varsa.i  >%OUTPUT_DIR%\d_varsa.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\math.i     >%OUTPUT_DIR%\math.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\r_aclipa.i >%OUTPUT_DIR%\r_aclipa.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\r_aliasa.i >%OUTPUT_DIR%\r_aliasa.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\r_drawa.i  >%OUTPUT_DIR%\r_drawa.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\r_edgea.i  >%OUTPUT_DIR%\r_edgea.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\r_varsa.i  >%OUTPUT_DIR%\r_varsa.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\snd_mixa.i >%OUTPUT_DIR%\snd_mixa.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\surf16.i   >%OUTPUT_DIR%\surf16.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\surf8.i    >%OUTPUT_DIR%\surf8.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\sys_wina.i >%OUTPUT_DIR%\sys_wina.asm
..\gas2masm\lcc_release\gas2masm.exe <%OUTPUT_DIR%\worlda.i   >%OUTPUT_DIR%\worlda.asm
@echo off
echo.
echo Step #3 for WinQuake is done
echo.
