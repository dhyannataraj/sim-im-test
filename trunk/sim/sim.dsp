# Microsoft Developer Studio Project File - Name="sim" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=sim - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "sim.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "sim.mak" CFG="sim - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "sim - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "sim - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release_Wrk"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "$(QTDIR)\include" /I "sim" /I "sim\win32" /D "NDEBUG" /D "QT_DLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D VERSION=\"0.9.4\" /D PACKAGE=\"SIM\" /D "UNICODE" /D "USE_OPENSSL" /D LTDL_SHLIB_EXT=\".dll\" /D LTDL_OBJDIR=\"\" /D "HAVE_STDIO_H" /D "HAVE_STRING_H" /D "HAVE_MALLOC_H" /D "SIMAPI_EXPORTS" /D CVS_BUILD=1 /Fo"Release/" /Fd"Release/" /FD /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 sim\win32\libxml.lib sim\win32\libxslt.lib sim\win32\libz.a sim\win32\libjpeg.lib sim\win32\libeay32.lib sim\win32\ssleay32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(QTDIR)\lib\qt-mtnc321.lib $(QTDIR)\lib\qtmain.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sim___Win32_Debug"
# PROP BASE Intermediate_Dir "sim___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug_Wrk"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(QTDIR)\include" /I "sim" /I "sim\win32" /D "_DEBUG" /D "_USRDLL" /D "QT_DLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D VERSION=\"0.9.4\" /D PACKAGE=\"SIM\" /D "UNICODE" /D "USE_OPENSSL" /D LTDL_SHLIB_EXT=\".dll\" /D LTDL_OBJDIR=\"\" /D "HAVE_STDIO_H" /D "HAVE_STRING_H" /D "HAVE_MALLOC_H" /D "SIMAPI_EXPORTS" /D CVS_BUILD=1 /FR"Debug/" /Fo"Debug/" /Fd"Debug/" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 sim\win32\libxml.lib sim\win32\libxslt.lib sim\win32\libz.a sim\win32\libjpeg.lib sim\win32\libeay32.lib sim\win32\ssleay32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(QTDIR)\lib\$(QTLIB) $(QTDIR)\lib\qtmain.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "sim - Win32 Release"
# Name "sim - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\sim\aboutdata.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ballonmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\cfg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\cmddef.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\contacts.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\country.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\datepicker.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\editfile.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\exec.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\fetch.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\fontedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\html.cpp
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=.\sim\html.ll

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - Flex $(InputPath)
InputDir=.\sim
InputPath=.\sim\html.ll
InputName=html

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(InputDir)\$(InputName).cpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - Flex $(InputPath)
InputDir=.\sim
InputPath=.\sim\html.ll
InputName=html

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(InputDir)\$(InputName).cpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\icons.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\intedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\linklabel.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\listview.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\log.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libltdl\ltdl.c
# End Source File
# Begin Source File

SOURCE=.\sim\message.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_ballonmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_datepicker.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_editfile.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_exec.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_fetch.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_fontedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_icons.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_intedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_linklabel.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_listview.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_qchildwidget.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_qcolorbutton.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_qkeybutton.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_socket.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_sockfactory.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_textshow.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_toolbtn.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\plugins.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\preview.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qchildwidget.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qcolorbutton.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qcomplextext.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qjpegio.cpp
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=.\sim\qkeybutton.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qkeysequence.cpp

!IF  "$(CFG)" == "sim - Win32 Release"

# ADD CPP /W3

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qregexp.cpp

!IF  "$(CFG)" == "sim - Win32 Release"

# ADD CPP /W3

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qrichtext.cpp
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qrichtext_p.cpp

!IF  "$(CFG)" == "sim - Win32 Release"

# ADD CPP /W3

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qsimplerichtext.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qtextedit.cpp

!IF  "$(CFG)" == "sim - Win32 Release"

# ADD CPP /W3

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\qzip.cpp

!IF  "$(CFG)" == "sim - Win32 Release"

# ADD CPP /W3

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\sax.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\sim.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\simapi.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\socket.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\sockfactory.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\sslclient.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\textshow.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\toolbtn.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\translit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\unquot.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\unzip.c
# End Source File
# Begin Source File

SOURCE=.\sim\xsl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\sim\aboutdata.h
# End Source File
# Begin Source File

SOURCE=.\sim\ballonmsg.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\ballonmsg.h
InputName=ballonmsg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\ballonmsg.h
InputName=ballonmsg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\buffer.h
# End Source File
# Begin Source File

SOURCE=.\sim\cjk_variants.h
# End Source File
# Begin Source File

SOURCE=.\sim\compatqtversion.h
# End Source File
# Begin Source File

SOURCE=.\sim\datepicker.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\datepicker.h
InputName=datepicker

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\datepicker.h
InputName=datepicker

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\editfile.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\editfile.h
InputName=editfile

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\editfile.h
InputName=editfile

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\exec.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\exec.h
InputName=exec

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\exec.h
InputName=exec

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\fetch.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\fetch.h
InputName=fetch

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\fetch.h
InputName=fetch

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\fontedit.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\fontedit.h
InputName=fontedit

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\fontedit.h
InputName=fontedit

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\html.h
# End Source File
# Begin Source File

SOURCE=.\sim\icons.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\icons.h
InputName=icons

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\icons.h
InputName=icons

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\intedit.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\intedit.h
InputName=intedit

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\intedit.h
InputName=intedit

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\johab_hangul.h
# End Source File
# Begin Source File

SOURCE=.\sim\kdeisversion.h
# End Source File
# Begin Source File

SOURCE=.\sim\linklabel.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\linklabel.h
InputName=linklabel

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\linklabel.h
InputName=linklabel

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\listview.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\listview.h
InputName=listview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\listview.h
InputName=listview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\libltdl\ltdl.h
# End Source File
# Begin Source File

SOURCE=.\sim\md32_common.h
# End Source File
# Begin Source File

SOURCE=.\sim\md5.h
# End Source File
# Begin Source File

SOURCE=.\sim\md5_locl.h
# End Source File
# Begin Source File

SOURCE=.\sim\preview.h
# End Source File
# Begin Source File

SOURCE=.\sim\qchildwidget.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\qchildwidget.h
InputName=qchildwidget

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\qchildwidget.h
InputName=qchildwidget

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qcleanuphandler.h
# End Source File
# Begin Source File

SOURCE=.\sim\qcolorbutton.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\qcolorbutton.h
InputName=qcolorbutton

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\qcolorbutton.h
InputName=qcolorbutton

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qcomplextext_p.h
# End Source File
# Begin Source File

SOURCE=.\sim\qjpegio.h
# End Source File
# Begin Source File

SOURCE=.\sim\qkeybutton.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\qkeybutton.h
InputName=qkeybutton

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\qkeybutton.h
InputName=qkeybutton

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qkeysequence.h
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qregexp.h
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qrichtext_p.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\qt3
InputPath=.\sim\qt3\qrichtext_p.h
InputName=qrichtext_p

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\qt3
InputPath=.\sim\qt3\qrichtext_p.h
InputName=qrichtext_p

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qsimplerichtext.h
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qstylesheet.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\qt3
InputPath=.\sim\qt3\qstylesheet.h
InputName=qstylesheet

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\qt3
InputPath=.\sim\qt3\qstylesheet.h
InputName=qstylesheet

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qsyntaxhighlighter.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qt3stuff.h
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qtextedit.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\qt3
InputPath=.\sim\qt3\qtextedit.h
InputName=qtextedit

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\qt3
InputPath=.\sim\qt3\qtextedit.h
InputName=qtextedit

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\qzip.h
# End Source File
# Begin Source File

SOURCE=.\sim\sax.h
# End Source File
# Begin Source File

SOURCE=.\sim\simapi.h
# End Source File
# Begin Source File

SOURCE=.\sim\socket.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\socket.h
InputName=socket

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\socket.h
InputName=socket

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\sockfactory.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\sockfactory.h
InputName=sockfactory

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\sockfactory.h
InputName=sockfactory

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\stl.h
# End Source File
# Begin Source File

SOURCE=.\sim\textshow.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\textshow.h
InputName=textshow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\textshow.h
InputName=textshow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\toolbtn.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\toolbtn.h
InputName=toolbtn

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\toolbtn.h
InputName=toolbtn

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\translit.h
# End Source File
# Begin Source File

SOURCE=.\sim\unzip.h
# End Source File
# Begin Source File

SOURCE=.\sim\xsl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icq.ico
# End Source File
# Begin Source File

SOURCE=.\sim.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\plugins\_core\parse.ll
# End Source File
# End Target
# End Project
