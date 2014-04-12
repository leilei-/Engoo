@echo off
set OUTPUT_DIR=lcc_lib_wq
cls
echo Step #2: Go through all .i files in %OUTPUT_DIR% and remove additional spaces from
echo A) Jump points, before the name and between it and the colon (:)
echo      Example: " _Invert24To16 :"
echo B) Calculations
echo      Example: "addl _paintbuffer + 0 - 8 *2(,%ecx,8),%edi"
echo.
echo After this you can go on to the next step.
echo.
echo Tips:
echo If you use UltraEdit use regular expressions:
echo   for A replace "^(_*^) :" with "^1:"
echo   for B search " +", " -", " *" and "+ ", "- ", "* "
echo.
echo URL:
echo UltraEdit - http://www.ultraedit.com/
echo.
