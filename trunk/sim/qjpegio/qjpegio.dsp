# Microsoft Developer Studio Project File - Name="qjpegio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=qjpegio - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "qjpegio.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "qjpegio.mak" CFG="qjpegio - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "qjpegio - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "qjpegio - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qjpegio - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "qjpegio_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O1 /I "$(QTDIR)\include" /I "libjpeg" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D "QJPEGIO_EXPORTS" /FR"Debug/" /Fo"Debug/" /Fd"Debug/" /FD /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x417 /i "..\.." /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib $(QTDIR)\lib\qt-mt230nc.lib /nologo /dll /pdb:none /machine:I386

!ELSEIF  "$(CFG)" == "qjpegio - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Debug"
# PROP Intermediate_Dir "..\..\Debug_Wrk"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "qjpegio_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "$(QTDIR)\include" /I "libjpeg" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D "QJPEGIO_EXPORTS" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x417 /i "..\.." /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib $(QTDIR)\lib\qt-mt230nc.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "qjpegio - Win32 Release"
# Name "qjpegio - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\libjpeg\jcapimin.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcapistd.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jccoefct.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jccolor.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jchuff.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcinit.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcmainct.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcmarker.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcmaster.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcomapi.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcparam.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcphuff.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcprepct.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jcsample.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jctrans.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdapimin.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdapistd.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdatadst.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdcolor.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdhuff.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdinput.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdmainct.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdmarker.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdmaster.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdmerge.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdphuff.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdpostct.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdsample.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jdtrans.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jerror.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jfdctint.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jidctflt.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jidctfst.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jidctint.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jidctred.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jmemansi.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jquant1.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jquant2.c
# End Source File
# Begin Source File

SOURCE=.\libjpeg\jutils.c
# End Source File
# Begin Source File

SOURCE=.\qjpegio.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\qjpegio.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
