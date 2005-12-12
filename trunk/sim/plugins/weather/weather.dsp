# Microsoft Developer Studio Project File - Name="weather" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=weather - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "weather.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "weather.mak" CFG="weather - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "weather - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "weather - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
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
# ADD CPP /nologo /MD /W3 /GX /O1 /I "$(QTDIR)\include" /I "..\..\sim" /I "..\_core" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D VERSION=\"0.9.4\" /D PACKAGE=\"SIM\" /D USE_QT=1 /D "CVS_BUILD" /FR"Debug/" /Fo"Debug/" /Fd"Debug/" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x417 /i "..\.." /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ..\..\Release\sim.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib $(QTDIR)\lib\$(QTLIB) /nologo /dll /pdb:none /machine:I386 /out:"../../Release/plugins/weather.dll"

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
# ADD CPP /nologo /MDd /W4 /WX /Gm /ZI /Od /I "$(QTDIR)\include" /I "..\..\sim" /I "..\_core" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D VERSION=\"0.9.4\" /D PACKAGE=\"SIM\" /D USE_QT=1 /D "DEBUG" /D CVS_BUILD=1 /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x417 /i "..\.." /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\..\Debug\sim.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib $(QTDIR)\lib\$(QTLIB) /nologo /dll /debug /machine:I386 /out:"../../Debug/plugins/weather.dll" /pdbtype:sept

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
# Begin Group "jisp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\jisp\0.png
# End Source File
# Begin Source File

SOURCE=.\jisp\1.png
# End Source File
# Begin Source File

SOURCE=.\jisp\10.png
# End Source File
# Begin Source File

SOURCE=.\jisp\11.png
# End Source File
# Begin Source File

SOURCE=.\jisp\12.png
# End Source File
# Begin Source File

SOURCE=.\jisp\13.png
# End Source File
# Begin Source File

SOURCE=.\jisp\14.png
# End Source File
# Begin Source File

SOURCE=.\jisp\15.png
# End Source File
# Begin Source File

SOURCE=.\jisp\16.png
# End Source File
# Begin Source File

SOURCE=.\jisp\17.png
# End Source File
# Begin Source File

SOURCE=.\jisp\18.png
# End Source File
# Begin Source File

SOURCE=.\jisp\19.png
# End Source File
# Begin Source File

SOURCE=.\jisp\2.png
# End Source File
# Begin Source File

SOURCE=.\jisp\20.png
# End Source File
# Begin Source File

SOURCE=.\jisp\21.png
# End Source File
# Begin Source File

SOURCE=.\jisp\22.png
# End Source File
# Begin Source File

SOURCE=.\jisp\23.png
# End Source File
# Begin Source File

SOURCE=.\jisp\24.png
# End Source File
# Begin Source File

SOURCE=.\jisp\25.png
# End Source File
# Begin Source File

SOURCE=.\jisp\26.png
# End Source File
# Begin Source File

SOURCE=.\jisp\27.png
# End Source File
# Begin Source File

SOURCE=.\jisp\28.png
# End Source File
# Begin Source File

SOURCE=.\jisp\29.png
# End Source File
# Begin Source File

SOURCE=.\jisp\3.png
# End Source File
# Begin Source File

SOURCE=.\jisp\30.png
# End Source File
# Begin Source File

SOURCE=.\jisp\31.png
# End Source File
# Begin Source File

SOURCE=.\jisp\32.png
# End Source File
# Begin Source File

SOURCE=.\jisp\33.png
# End Source File
# Begin Source File

SOURCE=.\jisp\34.png
# End Source File
# Begin Source File

SOURCE=.\jisp\35.png
# End Source File
# Begin Source File

SOURCE=.\jisp\36.png
# End Source File
# Begin Source File

SOURCE=.\jisp\37.png
# End Source File
# Begin Source File

SOURCE=.\jisp\38.png
# End Source File
# Begin Source File

SOURCE=.\jisp\39.png
# End Source File
# Begin Source File

SOURCE=.\jisp\4.png
# End Source File
# Begin Source File

SOURCE=.\jisp\40.png
# End Source File
# Begin Source File

SOURCE=.\jisp\41.png
# End Source File
# Begin Source File

SOURCE=.\jisp\42.png
# End Source File
# Begin Source File

SOURCE=.\jisp\43.png
# End Source File
# Begin Source File

SOURCE=.\jisp\44.png
# End Source File
# Begin Source File

SOURCE=.\jisp\45.png
# End Source File
# Begin Source File

SOURCE=.\jisp\46.png
# End Source File
# Begin Source File

SOURCE=.\jisp\47.png
# End Source File
# Begin Source File

SOURCE=.\jisp\5.png
# End Source File
# Begin Source File

SOURCE=.\jisp\6.png
# End Source File
# Begin Source File

SOURCE=.\jisp\7.png
# End Source File
# Begin Source File

SOURCE=.\jisp\8.png
# End Source File
# Begin Source File

SOURCE=.\jisp\9.png
# End Source File
# Begin Source File

SOURCE=.\jisp\icondef.xml

!IF  "$(CFG)" == "weather - Win32 Release"

# Begin Custom Build - Make weather.jisp
InputDir=.\jisp
IntDir=.\..\..\Release_Wrk
OutDir=.\..\..\Release
InputPath=.\jisp\icondef.xml

"$(OutDir)\icons\weather.jisp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir $(IntDir)\jisp 
	copy $(InputDir)\*.png $(IntDir)\jisp 
	copy $(InputDir)\icondef.xml $(IntDir)\jisp 
	jar -cMf $(OutDir)\icons\weather.jisp -C $(IntDir)\jisp . 
	del /Q $(IntDir)\jisp\*.* 
	rmdir /Q $(IntDir)\jisp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "weather - Win32 Debug"

# Begin Custom Build - Make weather.jisp
InputDir=.\jisp
IntDir=.\..\..\Debug_Wrk
OutDir=.\..\..\Debug
InputPath=.\jisp\icondef.xml

"$(OutDir)\icons\weather.jisp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir $(IntDir)\jisp 
	copy $(InputDir)\*.png $(IntDir)\jisp 
	copy $(InputDir)\icondef.xml $(IntDir)\jisp 
	jar -cMf $(OutDir)\icons\weather.jisp -C $(IntDir)\jisp . 
	del /Q $(IntDir)\jisp\*.* 
	rmdir /Q $(IntDir)\jisp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jisp\na.png
# End Source File
# End Group
# End Target
# End Project
