# Microsoft Developer Studio Project File - Name="weather" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=weather - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "weather.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "weather.mak" CFG="weather - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "weather - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "weather - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "weather - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Release"
# PROP Intermediate_Dir "..\..\Release_Wrk"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "weather_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /Gi /GX /O2 /Ob2 /I "$(QTDIR)\include" /I "$(LIBXML_DIR)\include" /I "..\..\sim\api" /I "..\..\sim\ui" /I "..\_core" /D "_WINDOWS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D VERSION=\"0.9.4\" /D PACKAGE=\"SIM\" /D USE_QT=1 /D "CVS_BUILD" /D "WIN32" /D "_MBCS" /FR"Debug/" /Fo"Debug/" /Fd"Debug/" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x417 /i "..\.." /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ..\..\Release\simui.lib ..\..\Release\simapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib $(QTDIR)\lib\$(QT_LIB) $(LIBXML_DIR)\lib\libxml2.lib /nologo /dll /pdb:none /machine:I386 /out:"../../Release/plugins/weather.dll"

!ELSEIF  "$(CFG)" == "weather - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Debug"
# PROP Intermediate_Dir "..\..\Debug_Wrk"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "weather_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(QTDIR)\include" /I "$(LIBXML_DIR)\include" /I "..\..\sim\api" /I "..\..\sim\ui" /I "..\_core" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D VERSION=\"0.9.4\" /D PACKAGE=\"SIM\" /D USE_QT=1 /D "DEBUG" /D CVS_BUILD=1 /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x417 /i "..\.." /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\..\Debug\simapi.lib ..\..\Debug\simui.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib winmm.lib $(QTDIR)\lib\$(QT_LIB) $(LIBXML_DIR)\lib\libxml2.lib /nologo /dll /debug /machine:I386 /out:"../../Debug/plugins/weather.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "weather - Win32 Release"
# Name "weather - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\moc_weather.cpp
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=.\moc_weathercfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_weathercfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_wifacecfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_wifacecfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\weather.cpp
# End Source File
# Begin Source File

SOURCE=.\weathercfg.cpp
# End Source File
# Begin Source File

SOURCE=.\weathercfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\wifacecfg.cpp
# End Source File
# Begin Source File

SOURCE=.\wifacecfgbase.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\weather.h

!IF  "$(CFG)" == "weather - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\weather.h
InputName=weather

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "weather - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\weather.h
InputName=weather

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\weathercfg.h

!IF  "$(CFG)" == "weather - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\weathercfg.h
InputName=weathercfg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "weather - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\weathercfg.h
InputName=weathercfg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\weathercfgbase.h
# End Source File
# Begin Source File

SOURCE=.\wifacecfg.h

!IF  "$(CFG)" == "weather - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\wifacecfg.h
InputName=wifacecfg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "weather - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\wifacecfg.h
InputName=wifacecfg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wifacecfgbase.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\weather.rc
# End Source File
# End Group
# Begin Group "xpm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xpm\0.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\1.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\10.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\11.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\12.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\13.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\14.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\15.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\16.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\17.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\18.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\19.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\2.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\20.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\21.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\22.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\23.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\24.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\25.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\26.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\27.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\28.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\29.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\3.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\30.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\31.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\32.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\33.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\34.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\35.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\36.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\37.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\38.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\39.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\4.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\40.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\41.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\42.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\43.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\44.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\45.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\46.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\47.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\5.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\6.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\7.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\8.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\9.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\na.xpm
# End Source File
# End Group
# Begin Group "Interfaces"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=.\weathercfgbase.ui

!IF  "$(CFG)" == "weather - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.
InputPath=.\weathercfgbase.ui
InputName=weathercfgbase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "weather - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.
InputPath=.\weathercfgbase.ui
InputName=weathercfgbase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wifacecfgbase.ui

!IF  "$(CFG)" == "weather - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.
InputPath=.\wifacecfgbase.ui
InputName=wifacecfgbase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "weather - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.
InputPath=.\wifacecfgbase.ui
InputName=wifacecfgbase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
