# Microsoft Developer Studio Project File - Name="_core" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=_core - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "_core.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "_core.mak" CFG="_core - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "_core - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "_core - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "_core - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_core_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O1 /I "$(QTDIR)\include" /I "..\..\sim\api" /I "..\..\sim\ui" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D VERSION=\"0.9\" /D PACKAGE=\"SIM\" /D USE_QT=1 /FR"Debug/" /Fo"Debug/" /Fd"Debug/" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib $(QTDIR)\lib\qt-mt230nc.lib ..\..\Release\simapi.lib ..\..\Release\simui.lib /nologo /dll /pdb:none /machine:I386 /out:"../../Release/plugins/_core.dll"

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_core_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /ZI /Od /I "$(QTDIR)\include" /I "..\..\sim\api" /I "..\..\sim\ui" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D VERSION=\"0.9\" /D PACKAGE=\"SIM\" /D USE_QT=1 /D "DEBUG" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib $(QTDIR)\lib\qt-mt230nc.lib ..\..\Debug\simapi.lib ..\..\Debug\simui.lib /nologo /dll /debug /machine:I386 /out:"../../Debug/plugins/_core.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "_core - Win32 Release"
# Name "_core - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\arcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\arcfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\autoreply.cpp
# End Source File
# Begin Source File

SOURCE=.\autoreplybase.cpp
# End Source File
# Begin Source File

SOURCE=.\cfgdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\cfgdlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\cmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\commands.cpp
# End Source File
# Begin Source File

SOURCE=.\connectionsettings.cpp
# End Source File
# Begin Source File

SOURCE=.\connectionsettingsbase.cpp
# End Source File
# Begin Source File

SOURCE=.\connectwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\connectwndbase.cpp
# End Source File
# Begin Source File

SOURCE=.\container.cpp
# End Source File
# Begin Source File

SOURCE=.\core.cpp
# End Source File
# Begin Source File

SOURCE=.\editmail.cpp
# End Source File
# Begin Source File

SOURCE=.\editmailbase.cpp
# End Source File
# Begin Source File

SOURCE=.\editphone.cpp
# End Source File
# Begin Source File

SOURCE=.\editphonebase.cpp
# End Source File
# Begin Source File

SOURCE=.\exec.cpp
# End Source File
# Begin Source File

SOURCE=.\filetransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\filetransferbase.cpp
# End Source File
# Begin Source File

SOURCE=.\history.cpp
# End Source File
# Begin Source File

SOURCE=.\historycfg.cpp
# End Source File
# Begin Source File

SOURCE=.\historycfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\historywnd.cpp
# End Source File
# Begin Source File

SOURCE=.\icons.cpp
# End Source File
# Begin Source File

SOURCE=.\intedit.cpp
# End Source File
# Begin Source File

SOURCE=.\interfacecfg.cpp
# End Source File
# Begin Source File

SOURCE=.\interfacecfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\kpopup.cpp
# End Source File
# Begin Source File

SOURCE=.\logindlg.cpp
# End Source File
# Begin Source File

SOURCE=.\logindlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\maininfo.cpp
# End Source File
# Begin Source File

SOURCE=.\maininfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\mainwin.cpp
# End Source File
# Begin Source File

SOURCE=.\manager.cpp
# End Source File
# Begin Source File

SOURCE=.\managerbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_arcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_arcfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_autoreply.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_autoreplybase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_cfgdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_cfgdlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_cmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_commands.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_connectionsettings.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_connectionsettingsbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_connectwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_connectwndbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_container.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_core.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_editmail.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_editmailbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_editphone.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_editphonebase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_exec.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_filetransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_filetransferbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_historycfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_historycfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_historywnd.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_icons.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_intedit.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_interfacecfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_interfacecfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_kpopup.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_logindlg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_logindlgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_maininfo.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_maininfobase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_mainwin.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_managerbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_msgauth.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_msgcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_msgcfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_msgedit.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_msgfile.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_msggen.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_msgrecv.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_msgsms.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_msgview.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_newprotocol.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_newprotocolbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_pagerbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_pagerdetails.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_phonebase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_phonedetails.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_plugincfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_plugincfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_prefcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_prefcfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_qchildwidget.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_search.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_searchbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_smscfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_smscfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_status.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_statuswnd.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_tmpl.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_toolsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_toolsetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_usercfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_userlist.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_userview.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_userviewcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_userviewcfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_userwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\msgauth.cpp
# End Source File
# Begin Source File

SOURCE=.\msgcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\msgcfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\msgedit.cpp
# End Source File
# Begin Source File

SOURCE=.\msgfile.cpp
# End Source File
# Begin Source File

SOURCE=.\msggen.cpp
# End Source File
# Begin Source File

SOURCE=.\msgrecv.cpp
# End Source File
# Begin Source File

SOURCE=.\msgsms.cpp
# End Source File
# Begin Source File

SOURCE=.\msgview.cpp
# End Source File
# Begin Source File

SOURCE=.\newprotocol.cpp
# End Source File
# Begin Source File

SOURCE=.\newprotocolbase.cpp
# End Source File
# Begin Source File

SOURCE=.\pagerbase.cpp
# End Source File
# Begin Source File

SOURCE=.\pagerdetails.cpp
# End Source File
# Begin Source File

SOURCE=.\parse.cpp
# End Source File
# Begin Source File

SOURCE=.\parse.ll
# End Source File
# Begin Source File

SOURCE=.\phonebase.cpp
# End Source File
# Begin Source File

SOURCE=.\phonedetails.cpp
# End Source File
# Begin Source File

SOURCE=.\plugincfg.cpp
# End Source File
# Begin Source File

SOURCE=.\plugincfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\prefcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\prefcfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\qchildwidget.cpp
# End Source File
# Begin Source File

SOURCE=.\search.cpp
# End Source File
# Begin Source File

SOURCE=.\searchbase.cpp
# End Source File
# Begin Source File

SOURCE=.\smscfg.cpp
# End Source File
# Begin Source File

SOURCE=.\smscfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\status.cpp
# End Source File
# Begin Source File

SOURCE=.\statuswnd.cpp
# End Source File
# Begin Source File

SOURCE=.\tmpl.cpp
# End Source File
# Begin Source File

SOURCE=.\toolsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\toolsetupbase.cpp
# End Source File
# Begin Source File

SOURCE=.\usercfg.cpp
# End Source File
# Begin Source File

SOURCE=.\userlist.cpp
# End Source File
# Begin Source File

SOURCE=.\userview.cpp
# End Source File
# Begin Source File

SOURCE=.\userviewcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\userviewcfgbase.cpp
# End Source File
# Begin Source File

SOURCE=.\userwnd.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\arcfg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\arcfgbase.h
# End Source File
# Begin Source File

SOURCE=.\autoreply.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\autoreplybase.h
# End Source File
# Begin Source File

SOURCE=.\cfgdlg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\cfgdlgbase.h
# End Source File
# Begin Source File

SOURCE=.\cmenu.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\commands.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\connectionsettings.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\connectionsettingsbase.h
# End Source File
# Begin Source File

SOURCE=.\connectwnd.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\connectwndbase.h
# End Source File
# Begin Source File

SOURCE=.\container.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\core.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\editmail.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\editmailbase.h
# End Source File
# Begin Source File

SOURCE=.\editphone.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\editphonebase.h
# End Source File
# Begin Source File

SOURCE=.\exec.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\filetransfer.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\filetransferbase.h
# End Source File
# Begin Source File

SOURCE=.\history.h
# End Source File
# Begin Source File

SOURCE=.\historycfg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\historycfgbase.h
# End Source File
# Begin Source File

SOURCE=.\historywnd.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\icons.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\intedit.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\interfacecfg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\interfacecfgbase.h
# End Source File
# Begin Source File

SOURCE=.\kpopup.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\logindlg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\logindlgbase.h
# End Source File
# Begin Source File

SOURCE=.\maininfo.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\maininfobase.h
# End Source File
# Begin Source File

SOURCE=.\mainwin.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\manager.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\managerbase.h
# End Source File
# Begin Source File

SOURCE=.\msgauth.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\msgcfg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\msgcfgbase.h
# End Source File
# Begin Source File

SOURCE=.\msgedit.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\msgfile.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\msggen.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\msgrecv.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\msgsms.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\msgview.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\newprotocol.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\newprotocolbase.h
# End Source File
# Begin Source File

SOURCE=.\pagerbase.h
# End Source File
# Begin Source File

SOURCE=.\pagerdetails.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\phonebase.h
# End Source File
# Begin Source File

SOURCE=.\phonedetails.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\plugincfg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\plugincfgbase.h
# End Source File
# Begin Source File

SOURCE=.\prefcfg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\prefcfgbase.h
# End Source File
# Begin Source File

SOURCE=.\qchildwidget.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\search.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\searchbase.h
# End Source File
# Begin Source File

SOURCE=.\smscfg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\smscfgbase.h
# End Source File
# Begin Source File

SOURCE=.\status.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\statuswnd.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\tmpl.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\toolsetup.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\toolsetupbase.h
# End Source File
# Begin Source File

SOURCE=.\usercfg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\userlist.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\userview.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\userviewcfg.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\userviewcfgbase.h
# End Source File
# Begin Source File

SOURCE=.\userwnd.h
# PROP Ignore_Default_Tool 1
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Interfaces"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=.\arcfgbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\autoreplybase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\cfgdlgbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\connectionsettingsbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\connectwndbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\editmailbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\editphonebase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\filetransferbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\historycfgbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\interfacecfgbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\logindlgbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\maininfobase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\managerbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\msgcfgbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\newprotocolbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\pagerbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\phonebase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\plugincfgbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\prefcfgbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\searchbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\smscfgbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\toolsetupbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\userviewcfgbase.ui
# PROP Ignore_Default_Tool 1
# End Source File
# End Group
# Begin Group "xpm"

# PROP Default_Filter "xpm"
# Begin Source File

SOURCE=.\xpm\1downarrow.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\1leftarrow.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\1rightarrow.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\1uparrow.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\button_cancel.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\button_ok.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\cell.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\collapsed.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\configure.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\editcopy.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\editcut.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\editpaste.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\encrypted.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\exit.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\expanded.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\fax.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\file.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\fileopen.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\filesave.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\find.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\grp_collapse.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\grp_create.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\grp_expand.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\grp_off.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\grp_on.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\grp_rename.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\info.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\licq.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\licq_big.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\mail_generic.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\message.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\nonim.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\online_off.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\online_on.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\pager.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\phone.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\remove.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smile0.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smile1.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smile2.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smile3.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smile4.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smile5.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smile6.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smile7.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smile8.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smile9.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smileA.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smileB.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smileC.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smileD.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smileE.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\smileF.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\sms.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\sort_increase.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\stop.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\text.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\translit.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\typing.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\url.xpm
# End Source File
# Begin Source File

SOURCE=.\xpm\webpress.xpm
# End Source File
# End Group
# Begin Group "po"

# PROP Default_Filter "po"
# Begin Source File

SOURCE=..\..\po\bg.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\bg.po

"$(OutDir)\po\bg.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\bg.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\bg.po

"$(OutDir)\po\bg.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\bg.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\cs.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\cs.po

"$(OutDir)\po\cs.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\cs.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\cs.po

"$(OutDir)\po\cs.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\cs.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\de.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\de.po

"$(OutDir)\po\de.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\de.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\de.po

"$(OutDir)\po\de.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\de.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\es.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\es.po

"$(OutDir)\po\es.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\es.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\es.po

"$(OutDir)\po\es.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\es.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\fr.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\fr.po

"$(OutDir)\po\fr.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\fr.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\fr.po

"$(OutDir)\po\fr.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\fr.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\he.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\he.po

"$(OutDir)\po\he.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\he.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\he.po

"$(OutDir)\po\he.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\he.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\it.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\it.po

"$(OutDir)\po\it.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\it.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\it.po

"$(OutDir)\po\it.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\it.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\nl.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\nl.po

"$(OutDir)\po\nl.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\nl.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\nl.po

"$(OutDir)\po\nl.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\nl.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\pl.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\pl.po

"$(OutDir)\po\pl.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\pl.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\pl.po

"$(OutDir)\po\pl.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\pl.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\ru.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\ru.po

"$(OutDir)\po\ru.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\ru.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\ru.po

"$(OutDir)\po\ru.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\ru.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\sk.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\sk.po

"$(OutDir)\po\sk.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\sk.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\sk.po

"$(OutDir)\po\sk.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\sk.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\tr.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\tr.po

"$(OutDir)\po\tr.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\tr.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\tr.po

"$(OutDir)\po\tr.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\tr.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\uk.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\uk.po

"$(OutDir)\po\uk.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\uk.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\uk.po

"$(OutDir)\po\uk.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\uk.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\po\zh_TW.po

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Release
InputPath=..\..\po\zh_TW.po

"$(OutDir)\po\zh_TW.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\zh_TW.qm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - msg2qm on $(InputPath)
OutDir=.\..\..\Debug
InputPath=..\..\po\zh_TW.po

"$(OutDir)\po\zh_TW.qm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	msg2qm $(InputPath) 
	move tr.qm $(OutDir)\po\zh_TW.qm 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Pict"

# PROP Default_Filter "gif"
# Begin Source File

SOURCE=.\pict\connect.gif

!IF  "$(CFG)" == "_core - Win32 Release"

# Begin Custom Build - Copy $(InputPath)
OutDir=.\..\..\Release
InputPath=.\pict\connect.gif

"$(OutDir)\$(InputPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /Y $(InputPath) $(OutDir)\$(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "_core - Win32 Debug"

# Begin Custom Build - Copy $(InputPath)
OutDir=.\..\..\Debug
InputPath=.\pict\connect.gif

"$(OutDir)\$(InputPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /Y $(InputPath) $(OutDir)\$(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
