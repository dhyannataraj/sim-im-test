# Microsoft Developer Studio Project File - Name="sim" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=sim - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sim.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sim.mak" CFG="sim - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sim - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "sim - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O1 /I "$(QTDIR)\include" /I "sim" /I "sim\libicq" /I "sim\qt3" /I "\openssl\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D "NO_DEBUG" /D PACKAGE=\"SIM\" /D VERSION=\"0.8.1\" /D USE_OPENSSL=1 /D USE_QT=1 /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib winmm.lib \openssl\lib\ssleay32.lib \openssl\lib\libeay32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sim___Win32_Debug"
# PROP BASE Intermediate_Dir "sim___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /ZI /Od /I "$(QTDIR)\include" /I "sim" /I "sim\libicq" /I "sim\qt3" /I "\openssl\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D VERSION=\"0.8.2\" /D PACKAGE=\"SIM\" /D USE_OPENSSL=1 /D USE_QT=1 /D "DEBUG" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib \openssl\lib\ssleay32.lib \openssl\lib\libeay32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /profile /incremental:no /nodefaultlib

!ENDIF 

# Begin Target

# Name "sim - Win32 Release"
# Name "sim - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\sim\about.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\aboutdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\aboutdlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\aboutinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\aboutinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\accept.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\acceptbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\addphone.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\addphonebase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\alertdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\alertdialogbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\alertmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\alertmsgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\autoreply.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\autoreplybase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\ballonmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\bos.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\buddy.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\cfg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\chatwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\clearhtml.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\clearhtml.ll

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - Flex on $(InputPath)
InputDir=.\sim\libicq
InputPath=.\sim\libicq\clearhtml.ll

"$(InputDir)\clearhtml.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(InputDir)\clearhtml.cpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - Flex on $(InputPath)
InputDir=.\sim\libicq
InputPath=.\sim\libicq\clearhtml.ll

"$(InputDir)\clearhtml.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(InputDir)\clearhtml.cpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\client.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\contactlist.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\control.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\country.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\cuser.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\direct.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\dock.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\editfile.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\editspell.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\effect.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\emailedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\emaileditbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\enable.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\exec.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\filetransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\filetransferbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\fontedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\fontsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\fontsetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\generalsec.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\generalsecbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\genrtf.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\genrtf.ll

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - Flex on $(InputPath)
InputDir=.\sim\libicq
InputPath=.\sim\libicq\genrtf.ll

"$(InputDir)\genrtf.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(InputDir)\genrtf.cpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - Flex on $(InputPath)
InputDir=.\sim\libicq
InputPath=.\sim\libicq\genrtf.ll

"$(InputDir)\genrtf.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(InputDir)\genrtf.cpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\history.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\homeinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\homeinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\icmb.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\icons.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\icqclient.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\icqhttp.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\icqssl.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\intedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\interestsinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\interestsinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\keys.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\keysetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\keysetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\kpopup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\lists.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\listsec.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\listsecbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\location.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\log.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\login.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\logindlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\logindlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\maininfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\maininfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\mainwin.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\message.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\mgrep.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_aboutdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_aboutdlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_aboutinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_aboutinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_accept.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_acceptbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_addphone.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_addphonebase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_alertdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_alertdialogbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_alertmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_alertmsgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_autoreply.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_autoreplybase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_ballonmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_chatwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_client.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_control.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_dock.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_editfile.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_editspell.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_emailedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_emaileditbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_exec.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_filetransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_filetransferbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_fontedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_fontsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_fontsetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_generalsec.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_generalsecbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_homeinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_homeinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_intedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_interestsinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_interestsinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_keys.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_keysetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_keysetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_kpopup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_listsec.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_listsecbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_logindlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_logindlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_maininfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_maininfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_mainwin.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_monitor.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_moreinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_moreinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_msgdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_msgdialogbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_msgedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_msgview.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_network.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_networkbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_pagerbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_pagerdetails.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_pastinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_pastinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_phonebase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_phonebook.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_phonebookbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_phonedetails.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_proxydlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_proxydlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_qcolorbutton.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_qkeybutton.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\moc_qrichtext_p.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\moc_qstylesheet.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\moc_qtextbrowser.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\moc_qtextedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\moc_qtooltip.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_searchbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_searchdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_securedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_securedlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_setupdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_setupdlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_smssetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_smssetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_sockets.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_soundsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_soundsetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_statussetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_statussetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_themes.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_themesetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_themesetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_tmpl.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_toolbtn.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_toolsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_toolsetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_transparent.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_userautoreply.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_userbox.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_userinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_userinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_usertbl.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_userview.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_workinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moc_workinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\moc_xosd.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\monitor.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moreinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\moreinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\msgdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\msgdialogbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\msgedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\msgview.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\network.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\networkbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\pagerbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\pagerdetails.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\parse.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\parse.ll

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - Flex $(InputPath)
InputDir=.\sim
InputPath=.\sim\parse.ll

"$(InputDir)\parse.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(InputDir)\parse.cpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Flex $(InputPath)
InputDir=.\sim
InputPath=.\sim\parse.ll

"$(InputDir)\parse.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(InputDir)\parse.cpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\pastinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\pastinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\phonebase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\phonebook.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\phonebookbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\phonedetails.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\ping.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\proxy.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\proxydlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\proxydlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\qcolorbutton.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qcomplextext.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\qkeybutton.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qkeysequence.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qrichtext.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qrichtext_p.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qsimplerichtext.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qstylesheet.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qtextbrowser.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qtextedit.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\qt3\qtooltip.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\rtf.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\rtf.ll

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - Flex $(InputPath)
InputDir=.\sim\libicq
InputPath=.\sim\libicq\rtf.ll

"$(InputDir)\rtf.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(InputDir)\rtf.cpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Flex $(InputPath)
InputDir=.\sim\libicq
InputPath=.\sim\libicq\rtf.ll

"$(InputDir)\rtf.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(InputDir)\rtf.cpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\searchbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\searchdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\securedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\securedlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\service.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\setupdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\setupdlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\sim.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\smssetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\smssetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\socket.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\sockets.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\soundsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\soundsetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\splash.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\statussetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\statussetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\themes.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\themesetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\themesetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\tmpl.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\toolbtn.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\toolsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\toolsetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\translator.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\transparent.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\userautoreply.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\userbox.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\userinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\userinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\usertbl.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\userview.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\various.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\workinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\ui\workinfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\libicq\xml.cpp
# End Source File
# Begin Source File

SOURCE=.\sim\xosd.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\sim\ui\aboutdlg.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\aboutdlgbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\aboutinfo.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\aboutinfo.h
InputName=aboutinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\aboutinfo.h
InputName=aboutinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\aboutinfobase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\accept.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\accept.h
InputName=accept

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\accept.h
InputName=accept

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\acceptbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\addphone.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\addphone.h
InputName=addphone

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\addphone.h
InputName=addphone

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\addphonebase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\alertdialog.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\alertdialog.h
InputName=alertdialog

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\alertdialog.h
InputName=alertdialog

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\alertdialogbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\alertmsg.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\alertmsg.h
InputName=alertmsg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\alertmsg.h
InputName=alertmsg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\alertmsgbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\autoreply.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\autoreply.h
InputName=autoreply

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\autoreply.h
InputName=autoreply

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\autoreplybase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\ballonmsg.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\ballonmsg.h
InputName=ballonmsg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\ballonmsg.h
InputName=ballonmsg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\chatwnd.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\chatwnd.h
InputName=chatwnd

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\chatwnd.h
InputName=chatwnd

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\client.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\client.h
InputName=client

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\client.h
InputName=client

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\control.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\control.h
InputName=control

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\control.h
InputName=control

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\cuser.h
# End Source File
# Begin Source File

SOURCE=.\sim\defs.h
# End Source File
# Begin Source File

SOURCE=.\sim\dock.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\dock.h
InputName=dock

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\dock.h
InputName=dock

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\editfile.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\editfile.h
InputName=editfile

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\editfile.h
InputName=editfile

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\editspell.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\editspell.h
InputName=editspell

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\editspell.h
InputName=editspell

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\emailedit.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\emailedit.h
InputName=emailedit

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\emailedit.h
InputName=emailedit

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\emaileditbase.h
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

SOURCE=.\sim\ui\filetransfer.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\filetransfer.h
InputName=filetransfer

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\filetransfer.h
InputName=filetransfer

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\filetransferbase.h
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

SOURCE=.\sim\ui\fontsetup.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\fontsetup.h
InputName=fontsetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\fontsetup.h
InputName=fontsetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\fontsetupbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\generalsec.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\generalsec.h
InputName=generalsec

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\generalsec.h
InputName=generalsec

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\generalsecbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\history.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\homeinfo.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\homeinfo.h
InputName=homeinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\homeinfo.h
InputName=homeinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\homeinfobase.h
# End Source File
# Begin Source File

SOURCE=.\sim\icons.h
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

SOURCE=.\sim\ui\interestsinfo.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\interestsinfo.h
InputName=interestsinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\interestsinfo.h
InputName=interestsinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\interestsinfobase.h
# End Source File
# Begin Source File

SOURCE=.\sim\keys.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\keys.h
InputName=keys

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\keys.h
InputName=keys

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\keysetup.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\keysetup.h
InputName=keysetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\keysetup.h
InputName=keysetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\keysetupbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\kpopup.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\kpopup.h
InputName=kpopup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\kpopup.h
InputName=kpopup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\listsec.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\listsec.h
InputName=listsec

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\listsec.h
InputName=listsec

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\listsecbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\logindlg.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\logindlg.h
InputName=logindlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\logindlg.h
InputName=logindlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\logindlgbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\maininfo.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\maininfo.h
InputName=maininfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\maininfo.h
InputName=maininfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\maininfobase.h
# End Source File
# Begin Source File

SOURCE=.\sim\mainwin.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\mainwin.h
InputName=mainwin

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\mainwin.h
InputName=mainwin

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\monitor.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\monitor.h
InputName=monitor

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\monitor.h
InputName=monitor

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\moreinfo.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\moreinfo.h
InputName=moreinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\moreinfo.h
InputName=moreinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\moreinfobase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\msgdialog.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\msgdialog.h
InputName=msgdialog

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\msgdialog.h
InputName=msgdialog

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\msgdialogbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\msgedit.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\msgedit.h
InputName=msgedit

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\msgedit.h
InputName=msgedit

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\msgview.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\msgview.h
InputName=msgview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\msgview.h
InputName=msgview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\network.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\network.h
InputName=network

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\network.h
InputName=network

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\networkbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\pagerbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\pagerdetails.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\pagerdetails.h
InputName=pagerdetails

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\pagerdetails.h
InputName=pagerdetails

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\pastinfo.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\pastinfo.h
InputName=pastinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\pastinfo.h
InputName=pastinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\pastinfobase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\phonebase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\phonebook.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\phonebook.h
InputName=phonebook

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\phonebook.h
InputName=phonebook

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\phonebookbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\phonedetails.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\phonedetails.h
InputName=phonedetails

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\phonedetails.h
InputName=phonedetails

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\proxydlg.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\proxydlg.h
InputName=proxydlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\proxydlg.h
InputName=proxydlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\proxydlgbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\qcolorbutton.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\qcolorbutton.h
InputName=qcolorbutton

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\qcolorbutton.h
InputName=qcolorbutton

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\qkeybutton.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\qkeybutton.h
InputName=qkeybutton

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\qkeybutton.h
InputName=qkeybutton

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

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

SOURCE=.\sim\qt3\qtextbrowser.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\qt3
InputPath=.\sim\qt3\qtextbrowser.h
InputName=qtextbrowser

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\qt3
InputPath=.\sim\qt3\qtextbrowser.h
InputName=qtextbrowser

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

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

SOURCE=.\sim\qt3\qtooltip.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\qt3
InputPath=.\sim\qt3\qtooltip.h
InputName=qtooltip

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\qt3
InputPath=.\sim\qt3\qtooltip.h
InputName=qtooltip

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\searchbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\searchdlg.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\searchdlg.h
InputName=searchdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\searchdlg.h
InputName=searchdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\securedlg.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\securedlg.h
InputName=securedlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\securedlg.h
InputName=securedlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\securedlgbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\setupdlg.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\setupdlg.h
InputName=setupdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\setupdlg.h
InputName=setupdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\setupdlgbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\smssetup.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\smssetup.h
InputName=smssetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\smssetup.h
InputName=smssetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\smssetupbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\sockets.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\sockets.h
InputName=sockets

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\sockets.h
InputName=sockets

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\soundsetup.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\soundsetup.h
InputName=soundsetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\soundsetup.h
InputName=soundsetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\soundsetupbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\ui\statussetup.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\statussetup.h
InputName=statussetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\statussetup.h
InputName=statussetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\statussetupbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\themes.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\themes.h
InputName=themes

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\themes.h
InputName=themes

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\themesetup.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\themesetup.h
InputName=themesetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\themesetup.h
InputName=themesetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\themesetupbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\tmpl.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\tmpl.h
InputName=tmpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\tmpl.h
InputName=tmpl

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

SOURCE=.\sim\ui\toolsetup.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\toolsetup.h
InputName=toolsetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\toolsetup.h
InputName=toolsetup

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\toolsetupbase.h
# End Source File
# Begin Source File

SOURCE=.\sim\transparent.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\transparent.h
InputName=transparent

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\transparent.h
InputName=transparent

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\userautoreply.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\userautoreply.h
InputName=userautoreply

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\userautoreply.h
InputName=userautoreply

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\userbox.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\userbox.h
InputName=userbox

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\userbox.h
InputName=userbox

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\userinfo.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\userinfo.h
InputName=userinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\userinfo.h
InputName=userinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\userinfobase.h
# End Source File
# Begin Source File

SOURCE=.\sim\usertbl.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\usertbl.h
InputName=usertbl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\usertbl.h
InputName=usertbl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\userview.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\userview.h
InputName=userview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\userview.h
InputName=userview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\workinfo.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\workinfo.h
InputName=workinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim\ui
InputPath=.\sim\ui\workinfo.h
InputName=workinfo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\ui\workinfobase.h
# End Source File
# Begin Source File

SOURCE=.\sim\xosd.h

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\xosd.h
InputName=xosd

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\sim
InputPath=.\sim\xosd.h
InputName=xosd

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

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
# Begin Group "Interfaces"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=.\sim\ui\aboutdlgbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\aboutdlgbase.ui
InputName=aboutdlgbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\aboutdlgbase.ui
InputName=aboutdlgbase

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

SOURCE=.\sim\ui\aboutinfobase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\aboutinfobase.ui
InputName=aboutinfobase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\aboutinfobase.ui
InputName=aboutinfobase

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

SOURCE=.\sim\ui\acceptbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\acceptbase.ui
InputName=acceptbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\acceptbase.ui
InputName=acceptbase

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

SOURCE=.\sim\ui\addphonebase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\addphonebase.ui
InputName=addphonebase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\addphonebase.ui
InputName=addphonebase

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

SOURCE=.\sim\ui\alertdialogbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\alertdialogbase.ui
InputName=alertdialogbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\alertdialogbase.ui
InputName=alertdialogbase

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

SOURCE=.\sim\ui\alertmsgbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\alertmsgbase.ui
InputName=alertmsgbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\alertmsgbase.ui
InputName=alertmsgbase

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

SOURCE=.\sim\ui\autoreplybase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\autoreplybase.ui
InputName=autoreplybase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\autoreplybase.ui
InputName=autoreplybase

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

SOURCE=.\sim\ui\emaileditbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\emaileditbase.ui
InputName=emaileditbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\emaileditbase.ui
InputName=emaileditbase

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

SOURCE=.\sim\ui\filetransferbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\filetransferbase.ui
InputName=filetransferbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\filetransferbase.ui
InputName=filetransferbase

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

SOURCE=.\sim\ui\fontsetupbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\fontsetupbase.ui
InputName=fontsetupbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\fontsetupbase.ui
InputName=fontsetupbase

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

SOURCE=.\sim\ui\generalsecbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\generalsecbase.ui
InputName=generalsecbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\generalsecbase.ui
InputName=generalsecbase

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

SOURCE=.\sim\ui\homeinfobase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\homeinfobase.ui
InputName=homeinfobase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\homeinfobase.ui
InputName=homeinfobase

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

SOURCE=.\sim\ui\interestsinfobase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\interestsinfobase.ui
InputName=interestsinfobase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\interestsinfobase.ui
InputName=interestsinfobase

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

SOURCE=.\sim\ui\keysetupbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\keysetupbase.ui
InputName=keysetupbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\keysetupbase.ui
InputName=keysetupbase

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

SOURCE=.\sim\ui\listsecbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\listsecbase.ui
InputName=listsecbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\listsecbase.ui
InputName=listsecbase

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

SOURCE=.\sim\ui\logindlgbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\logindlgbase.ui
InputName=logindlgbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\logindlgbase.ui
InputName=logindlgbase

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

SOURCE=.\sim\ui\maininfobase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\maininfobase.ui
InputName=maininfobase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\maininfobase.ui
InputName=maininfobase

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

SOURCE=.\sim\ui\moreinfobase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\moreinfobase.ui
InputName=moreinfobase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\moreinfobase.ui
InputName=moreinfobase

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

SOURCE=.\sim\ui\msgdialogbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\msgdialogbase.ui
InputName=msgdialogbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\msgdialogbase.ui
InputName=msgdialogbase

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

SOURCE=.\sim\ui\networkbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\networkbase.ui
InputName=networkbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\networkbase.ui
InputName=networkbase

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

SOURCE=.\sim\ui\pagerbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\pagerbase.ui
InputName=pagerbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\pagerbase.ui
InputName=pagerbase

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

SOURCE=.\sim\ui\pastinfobase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\pastinfobase.ui
InputName=pastinfobase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\pastinfobase.ui
InputName=pastinfobase

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

SOURCE=.\sim\ui\phonebase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\phonebase.ui
InputName=phonebase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\phonebase.ui
InputName=phonebase

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

SOURCE=.\sim\ui\phonebookbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\phonebookbase.ui
InputName=phonebookbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\phonebookbase.ui
InputName=phonebookbase

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

SOURCE=.\sim\ui\proxydlgbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\proxydlgbase.ui
InputName=proxydlgbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\proxydlgbase.ui
InputName=proxydlgbase

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

SOURCE=.\sim\ui\searchbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\searchbase.ui
InputName=searchbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\searchbase.ui
InputName=searchbase

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

SOURCE=.\sim\ui\securedlgbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\securedlgbase.ui
InputName=securedlgbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\securedlgbase.ui
InputName=securedlgbase

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

SOURCE=.\sim\ui\setupdlgbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\setupdlgbase.ui
InputName=setupdlgbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\setupdlgbase.ui
InputName=setupdlgbase

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

SOURCE=.\sim\ui\smssetupbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\smssetupbase.ui
InputName=smssetupbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\smssetupbase.ui
InputName=smssetupbase

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

SOURCE=.\sim\ui\soundsetupbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\soundsetupbase.ui
InputName=soundsetupbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\soundsetupbase.ui
InputName=soundsetupbase

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

SOURCE=.\sim\ui\statussetupbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\statussetupbase.ui
InputName=statussetupbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\statussetupbase.ui
InputName=statussetupbase

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

SOURCE=.\sim\ui\themesetupbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\themesetupbase.ui
InputName=themesetupbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\themesetupbase.ui
InputName=themesetupbase

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

SOURCE=.\sim\ui\toolsetupbase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\toolsetupbase.ui
InputName=toolsetupbase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\toolsetupbase.ui
InputName=toolsetupbase

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

SOURCE=.\sim\ui\userinfobase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\userinfobase.ui
InputName=userinfobase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\userinfobase.ui
InputName=userinfobase

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

SOURCE=.\sim\ui\workinfobase.ui

!IF  "$(CFG)" == "sim - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\workinfobase.ui
InputName=workinfobase

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

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\sim\ui
InputPath=.\sim\ui\workinfobase.ui
InputName=workinfobase

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
# Begin Group "po"

# PROP Default_Filter "po"
# Begin Source File

SOURCE=.\po\cs.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\cs.po

"$(OutDir)\po\cs.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\cs.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\cs.po

"$(OutDir)\po\cs.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\cs.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po\de.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\de.po

"$(OutDir)\po\de.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\de.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\de.po

"$(OutDir)\po\de.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\de.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po\es.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\es.po

"$(OutDir)\po\es.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\es.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\es.po

"$(OutDir)\po\es.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\es.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po\fr.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\fr.po

"$(OutDir)\po\fr.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\fr.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\fr.po

"$(OutDir)\po\fr.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\fr.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po\it.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\it.po

"$(OutDir)\po\it.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\it.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\it.po

"$(OutDir)\po\it.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\it.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po\nl.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\nl.po

"$(OutDir)\po\nl.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\nl.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\nl.po

"$(OutDir)\po\nl.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\nl.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po\pl.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\pl.po

"$(OutDir)\po\pl.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\pl.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\pl.po

"$(OutDir)\po\pl.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\pl.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po\ru.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\ru.po

"$(OutDir)\po\ru.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\ru.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\ru.po

"$(OutDir)\po\ru.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\ru.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po\tr.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\tr.po

"$(OutDir)\po\tr.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\tr.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\tr.po

"$(OutDir)\po\tr.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\tr.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po\uk.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\uk.po

"$(OutDir)\po\uk.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\uk.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\uk.po

"$(OutDir)\po\uk.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\uk.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po\zh_TW.po

!IF  "$(CFG)" == "sim - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Release
InputPath=.\po\zh_TW.po

"$(OutDir)\po\zh_TW.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\zh_TW.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sim - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\Debug
InputPath=.\po\zh_TW.po

"$(OutDir)\po\zh_TW.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo "" > tr.qm 
	del tr.qm 
	$(QTDIR)\bin\msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\zh_TW.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
