# Microsoft Developer Studio Project File - Name="winquake" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=winquake - Win32 Debug R
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WinQuake.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinQuake.mak" CFG="winquake - Win32 Debug R"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "winquake - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "winquake - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "winquake - Win32 GL Debug" (based on "Win32 (x86) Application")
!MESSAGE "winquake - Win32 GL Release" (based on "Win32 (x86) Application")
!MESSAGE "winquake - Win32 Debug R" (based on "Win32 (x86) Application")
!MESSAGE "winquake - Win32 95 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\release"
# PROP BASE Intermediate_Dir ".\release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\release"
# PROP Intermediate_Dir ".\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /GX /Ox /Ot /Op /I "..\scitech\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# SUBTRACT CPP /Oa /Ow /Og /Os
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 dxguid.lib winmm.lib wsock32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /machine:I386 /out:"bin\engwin.exe"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\debug"
# PROP BASE Intermediate_Dir ".\debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\debug"
# PROP Intermediate_Dir ".\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /ML /GX /ZI /Od /I "..\scitech\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 dxguid.lib winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"bin\engwin.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\winquake"
# PROP BASE Intermediate_Dir ".\winquake"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\debug_gl"
# PROP Intermediate_Dir ".\debug_gl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /ML /GX /Zi /Od /I "..\scitech\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /ML /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "GLQUAKE" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\scitech\lib\win32\vc\mgllt.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 dxguid.lib comctl32.lib winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"bin\engwin.exe"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\winquak0"
# PROP BASE Intermediate_Dir ".\winquak0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\release_gl"
# PROP Intermediate_Dir ".\release_gl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /GX /Ox /Ot /Ow /I "..\scitech\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# SUBTRACT BASE CPP /Oa /Og
# ADD CPP /nologo /G5 /GX /Ot /Ow /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "GLQUAKE" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\scitech\lib\win32\vc\mgllt.lib /nologo /subsystem:windows /profile /machine:I386
# SUBTRACT BASE LINK32 /map /debug
# ADD LINK32 dxguid.lib comctl32.lib winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /machine:I386 /out:"bin\engwin.exe"
# SUBTRACT LINK32 /map /debug

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "winquake___Win32_Debug_R"
# PROP BASE Intermediate_Dir "winquake___Win32_Debug_R"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "winquake___Win32_Debug_R"
# PROP Intermediate_Dir "winquake___Win32_Debug_R"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /ML /GX /ZI /Od /I "..\scitech\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /G5 /ML /GX /ZI /Od /I "..\scitech\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\scitech\lib\win32\vc\mgllt.lib dxguid.lib winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib alleg.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"bin\engdebug.exe"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 ..\scitech\lib\win32\vc\mgllt.lib dxguid.lib winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib alleg.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"bin\engwin.exe"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "winquake___Win32_95_Release"
# PROP BASE Intermediate_Dir "winquake___Win32_95_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "winquake___Win32_95_Release"
# PROP Intermediate_Dir "winquake___Win32_95_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /GX /Ox /Ot /Op /I "..\scitech\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# SUBTRACT BASE CPP /Oa /Ow /Og
# ADD CPP /nologo /G5 /GX /Ox /Ot /Op /I "..\scitech\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# SUBTRACT CPP /Oa /Ow /Og
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\scitech\lib\win32\vc\mgllt.lib dxguid.lib winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /machine:I386 /out:"bin\engwin.exe"
# SUBTRACT BASE LINK32 /map /debug
# ADD LINK32 ..\scitech\lib\win32\vc\mgllt.lib dxguid.lib winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /machine:I386 /out:"bin\eng95.exe"

!ENDIF 

# Begin Target

# Name "winquake - Win32 Release"
# Name "winquake - Win32 Debug"
# Name "winquake - Win32 GL Debug"
# Name "winquake - Win32 GL Release"
# Name "winquake - Win32 Debug R"
# Name "winquake - Win32 95 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "Midi"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\midilib\_midi.h
# End Source File
# Begin Source File

SOURCE=..\midilib\midi.c
# End Source File
# Begin Source File

SOURCE=..\midilib\midi.h
# End Source File
# Begin Source File

SOURCE=..\midilib\mpu401.c
# End Source File
# Begin Source File

SOURCE=..\midilib\mpu401.h
# End Source File
# Begin Source File

SOURCE=..\midilib\music.c
# End Source File
# Begin Source File

SOURCE=..\midilib\music.h
# End Source File
# Begin Source File

SOURCE=..\midilib\standard.h
# End Source File
# Begin Source File

SOURCE=..\midilib\usrhooks.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\bot.c
# End Source File
# Begin Source File

SOURCE=.\bot.h
# End Source File
# Begin Source File

SOURCE=.\cd_win.c
# End Source File
# Begin Source File

SOURCE=.\chase.c
# End Source File
# Begin Source File

SOURCE=.\cl_demo.c
# End Source File
# Begin Source File

SOURCE=.\cl_input.c
# End Source File
# Begin Source File

SOURCE=.\cl_main.c
# End Source File
# Begin Source File

SOURCE=.\cl_parse.c
# End Source File
# Begin Source File

SOURCE=.\cl_tent.c
# End Source File
# Begin Source File

SOURCE=.\cmd.c
# End Source File
# Begin Source File

SOURCE=.\common.c
# End Source File
# Begin Source File

SOURCE=.\conproc.c
# End Source File
# Begin Source File

SOURCE=.\console.c
# End Source File
# Begin Source File

SOURCE=.\crc.c
# End Source File
# Begin Source File

SOURCE=.\cvar.c
# End Source File
# Begin Source File

SOURCE=.\d_edge.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_fill.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_init.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_modech.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_part.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_polyse.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_scan.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# ADD CPP /Og /Oi /Oy /Ob2

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_sky.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_sprite.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_surf.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_vars.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_zpoint.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\draw.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_draw.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_mesh.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_model.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_refrag.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rlight.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rmain.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rmisc.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rsurf.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_screen.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_test.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_vidnt.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_warp.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\host.c
# End Source File
# Begin Source File

SOURCE=.\host_cmd.c
# End Source File
# Begin Source File

SOURCE=.\in_win.c
# End Source File
# Begin Source File

SOURCE=.\keys.c
# End Source File
# Begin Source File

SOURCE=.\mathlib.c
# End Source File
# Begin Source File

SOURCE=.\menu.c
# End Source File
# Begin Source File

SOURCE=.\model.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\model_common.c
# End Source File
# Begin Source File

SOURCE=.\net_dgrm.c
# End Source File
# Begin Source File

SOURCE=.\net_loop.c
# End Source File
# Begin Source File

SOURCE=.\net_main.c
# End Source File
# Begin Source File

SOURCE=.\net_vcr.c
# End Source File
# Begin Source File

SOURCE=.\net_win.c
# End Source File
# Begin Source File

SOURCE=.\net_wins.c
# End Source File
# Begin Source File

SOURCE=.\net_wipx.c
# End Source File
# Begin Source File

SOURCE=.\nvs_client.c
# End Source File
# Begin Source File

SOURCE=.\nvs_common.c
# End Source File
# Begin Source File

SOURCE=.\nvs_server.c
# End Source File
# Begin Source File

SOURCE=.\nvs_server_data.c
# End Source File
# Begin Source File

SOURCE=.\pr_cmds.c
# End Source File
# Begin Source File

SOURCE=.\pr_edict.c
# End Source File
# Begin Source File

SOURCE=.\pr_exec.c
# End Source File
# Begin Source File

SOURCE=.\r_aclip.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_alias.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_bsp.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_draw.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_edge.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_efrag.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_light.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_main.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_misc.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_part.c
# End Source File
# Begin Source File

SOURCE=.\r_sky.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_sprite.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_surf.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# ADD CPP /w /W0 /Og

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_vars.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sbar.c
# End Source File
# Begin Source File

SOURCE=.\screen.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\snd_dma.c
# End Source File
# Begin Source File

SOURCE=.\snd_mem.c
# End Source File
# Begin Source File

SOURCE=.\snd_mix.c
# End Source File
# Begin Source File

SOURCE=.\snd_win.c
# End Source File
# Begin Source File

SOURCE=.\sv_main.c
# End Source File
# Begin Source File

SOURCE=.\sv_move.c
# End Source File
# Begin Source File

SOURCE=.\sv_phys.c
# End Source File
# Begin Source File

SOURCE=.\sv_user.c
# End Source File
# Begin Source File

SOURCE=.\sys_win.c
# End Source File
# Begin Source File

SOURCE=.\vid_win.c

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vid_win_old.c

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\view.c
# End Source File
# Begin Source File

SOURCE=.\wad.c
# End Source File
# Begin Source File

SOURCE=.\world.c
# End Source File
# Begin Source File

SOURCE=.\zone.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\adivtab.h
# End Source File
# Begin Source File

SOURCE=..\allegro\allegro.h
# End Source File
# Begin Source File

SOURCE=.\anorm_dots.h
# End Source File
# Begin Source File

SOURCE=.\anorms.h
# End Source File
# Begin Source File

SOURCE=.\bspfile.h
# End Source File
# Begin Source File

SOURCE=.\cdaudio.h
# End Source File
# Begin Source File

SOURCE=.\client.h
# End Source File
# Begin Source File

SOURCE=.\cmd.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\conproc.h
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\cvar.h
# End Source File
# Begin Source File

SOURCE=.\d_iface.h
# End Source File
# Begin Source File

SOURCE=.\d_local.h
# End Source File
# Begin Source File

SOURCE=.\dosisms.h
# End Source File
# Begin Source File

SOURCE=.\draw.h
# End Source File
# Begin Source File

SOURCE=.\gl_model.h
# End Source File
# Begin Source File

SOURCE=.\gl_warp_sin.h
# End Source File
# Begin Source File

SOURCE=.\glquake.h
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\keys.h
# End Source File
# Begin Source File

SOURCE=.\mathlib.h
# End Source File
# Begin Source File

SOURCE=.\menu.h
# End Source File
# Begin Source File

SOURCE=.\model.h
# End Source File
# Begin Source File

SOURCE=.\model_common.h
# End Source File
# Begin Source File

SOURCE=.\modelgen.h
# End Source File
# Begin Source File

SOURCE=.\net.h
# End Source File
# Begin Source File

SOURCE=.\net_dgrm.h
# End Source File
# Begin Source File

SOURCE=.\net_loop.h
# End Source File
# Begin Source File

SOURCE=.\net_ser.h
# End Source File
# Begin Source File

SOURCE=.\net_vcr.h
# End Source File
# Begin Source File

SOURCE=.\net_wins.h
# End Source File
# Begin Source File

SOURCE=.\net_wipx.h
# End Source File
# Begin Source File

SOURCE=.\nvs_client.h
# End Source File
# Begin Source File

SOURCE=.\nvs_common.h
# End Source File
# Begin Source File

SOURCE=.\nvs_server.h
# End Source File
# Begin Source File

SOURCE=.\pr_comp.h
# End Source File
# Begin Source File

SOURCE=.\progdefs.h
# End Source File
# Begin Source File

SOURCE=.\progs.h
# End Source File
# Begin Source File

SOURCE=.\protocol.h
# End Source File
# Begin Source File

SOURCE=.\quakedef.h
# End Source File
# Begin Source File

SOURCE=.\r_local.h
# End Source File
# Begin Source File

SOURCE=.\r_shared.h
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# Begin Source File

SOURCE=.\sbar.h
# End Source File
# Begin Source File

SOURCE=.\screen.h
# End Source File
# Begin Source File

SOURCE=.\server.h
# End Source File
# Begin Source File

SOURCE=.\sound.h
# End Source File
# Begin Source File

SOURCE=.\spritegn.h
# End Source File
# Begin Source File

SOURCE=.\sys.h
# End Source File
# Begin Source File

SOURCE=.\vid.h
# End Source File
# Begin Source File

SOURCE=.\view.h
# End Source File
# Begin Source File

SOURCE=.\wad.h
# End Source File
# Begin Source File

SOURCE=.\winquake.h
# End Source File
# Begin Source File

SOURCE=.\world.h
# End Source File
# Begin Source File

SOURCE=.\zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\engoo.bmp
# End Source File
# Begin Source File

SOURCE=..\icons\k.ico
# End Source File
# Begin Source File

SOURCE=..\icons\mhquake_256.ico
# End Source File
# Begin Source File

SOURCE=..\icons\qplayer_256.ico
# End Source File
# Begin Source File

SOURCE=..\icons\qsymbol_16.ico
# End Source File
# Begin Source File

SOURCE=..\icons\qsymbol_16_alpha.ico
# End Source File
# Begin Source File

SOURCE=.\winquake_vc.rc
# End Source File
# End Group
# Begin Group "Assembler Files"

# PROP Default_Filter "s"
# Begin Source File

SOURCE=..\asm\asm_draw.h
# End Source File
# Begin Source File

SOURCE=..\asm\asm_i386.h
# End Source File
# Begin Source File

SOURCE=..\asm\block16.h
# End Source File
# Begin Source File

SOURCE=..\asm\d_draw.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\d_draw.s
InputName=d_draw

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\d_draw.s
InputName=d_draw

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\d_draw.s
InputName=d_draw

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\d_draw.s
InputName=d_draw

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\d_draw16.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\d_draw16.s
InputName=d_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\d_draw16.s
InputName=d_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\d_draw16.s
InputName=d_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\d_draw16.s
InputName=d_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\d_ifacea.h
# End Source File
# Begin Source File

SOURCE=..\asm\d_parta.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\d_parta.s
InputName=d_parta

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\d_parta.s
InputName=d_parta

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\d_parta.s
InputName=d_parta

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\d_parta.s
InputName=d_parta

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\d_polysa.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\d_polysa.s
InputName=d_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\d_polysa.s
InputName=d_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\d_polysa.s
InputName=d_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\d_polysa.s
InputName=d_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\d_scana.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\d_scana.s
InputName=d_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\d_scana.s
InputName=d_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\d_scana.s
InputName=d_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\d_scana.s
InputName=d_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\d_spr8.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\d_spr8.s
InputName=d_spr8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\d_spr8.s
InputName=d_spr8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\d_spr8.s
InputName=d_spr8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\d_spr8.s
InputName=d_spr8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\d_varsa.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\d_varsa.s
InputName=d_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\d_varsa.s
InputName=d_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\d_varsa.s
InputName=d_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\d_varsa.s
InputName=d_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\math.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug_gl
InputPath=..\asm\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release_gl
InputPath=..\asm\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\quakeasm.h
# End Source File
# Begin Source File

SOURCE=..\asm\r_aclipa.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\r_aclipa.s
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\r_aclipa.s
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\r_aclipa.s
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\r_aclipa.s
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\r_aliasa.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\r_aliasa.s
InputName=r_aliasa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\r_aliasa.s
InputName=r_aliasa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\r_aliasa.s
InputName=r_aliasa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\r_aliasa.s
InputName=r_aliasa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\r_drawa.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\r_drawa.s
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\r_drawa.s
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\r_drawa.s
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\r_drawa.s
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\r_edgea.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\r_edgea.s
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\r_edgea.s
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\r_edgea.s
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\r_edgea.s
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\r_varsa.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\r_varsa.s
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\r_varsa.s
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\r_varsa.s
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\r_varsa.s
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\snd_mixa.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug_gl
InputPath=..\asm\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release_gl
InputPath=..\asm\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\surf16.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\surf16.s
InputName=surf16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\surf16.s
InputName=surf16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\surf16.s
InputName=surf16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\surf16.s
InputName=surf16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\surf8.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\surf8.s
InputName=surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\surf8.s
InputName=surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\surf8.s
InputName=surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\surf8.s
InputName=surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\surf8b.s

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\debug
InputPath=..\asm\surf8b.s
InputName=surf8b

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\surf8fst.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\surf8fst.s
InputName=surf8fst

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\surf8fst.s
InputName=surf8fst

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\surf8fst.s
InputName=surf8fst

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\surf8fst.s
InputName=surf8fst

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\surf8g.s

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\debug
InputPath=..\asm\surf8g.s
InputName=surf8g

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\surf8r.s

!IF  "$(CFG)" == "winquake - Win32 Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\debug
InputPath=..\asm\surf8r.s
InputName=surf8r

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\surf8rgb.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolerbuild
OutDir=.\release
InputPath=..\asm\surf8rgb.s
InputName=surf8rgb

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolestbuild
OutDir=.\debug
InputPath=..\asm\surf8rgb.s
InputName=surf8rgb

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\sys_wina.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\sys_wina.s
InputName=sys_wina

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\sys_wina.s
InputName=sys_wina

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug_gl
InputPath=..\asm\sys_wina.s
InputName=sys_wina

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release_gl
InputPath=..\asm\sys_wina.s
InputName=sys_wina

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\sys_wina.s
InputName=sys_wina

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\sys_wina.s
InputName=sys_wina

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\asm\worlda.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release
InputPath=..\asm\worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug
InputPath=..\asm\worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\debug_gl
InputPath=..\asm\worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 GL Release"

# Begin Custom Build - mycoolbuild
OutDir=.\release_gl
InputPath=..\asm\worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug R"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_Debug_R
InputPath=..\asm\worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 95 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\winquake___Win32_95_Release
InputPath=..\asm\worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\release\gas2masm < $(OUTDIR)\$(InputName).spp > $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\progdefs.q1
# End Source File
# Begin Source File

SOURCE=.\progdefs.q2
# End Source File
# End Target
# End Project
