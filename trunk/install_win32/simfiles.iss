; Script generated by the My Inno Setup Extensions Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=Simple Instant Messenger
AppVerName=SIM 0.9.2
AppPublisher=shutoff@mail.ru
AppPublisherURL=http://sim-icq.sourceforge.net/
AppSupportURL=http://sim-icq.sourceforge.net/
AppUpdatesURL=http://sim-icq.sourceforge.net/
DefaultDirName={pf}\SIM
DisableProgramGroupPage=yes
DisableReadyPage=yes
LicenseFile=..\COPYING
Compression=bzip/9
AppId=SIM
AppMutex=SIM_Mutex
AppCopyright=Copyright � 2002-2003, Vladimir Shutoff
DetectLanguageUsingLocale=yes
ShowLanguageDialog=no
ChangesAssociations=yes

[Tasks]
Name: startup; Description: "Launch SIM on &startup"; GroupDescription: "Startup:"
Name: startup\common; Description: "For all users"; GroupDescription: "Startup:"; Flags: exclusive
Name: startup\user; Description: "For the current user only"; GroupDescription: "Startup:"; Flags: exclusive unchecked
Name: desktopicon; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"
Name: desktopicon\common; Description: "For all users"; GroupDescription: "Additional icons:"; Flags: exclusive
Name: desktopicon\user; Description: "For the current user only"; GroupDescription: "Additional icons:"; Flags: exclusive unchecked

[Dirs]
Name: "{app}\po"
Name: "{app}\sounds"
Name: "{app}\pict"
Name: "{app}\styles"
Name: "{app}\icons"
Name: "{app}\plugins"
Name: "{app}\plugins\styles"

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "ru"; MessagesFile: "Russian.isl"
Name: "it"; MessagesFile: "Italian.isl"
Name: "de"; MessagesFile: "German.isl"
Name: "pt"; MessagesFile: "BrazilianPortuguese.isl"

[Files]
Source: "..\Release\sim.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\simapi.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\simui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\qjpegio.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\COPYING"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\plugins\__homedir.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\__migrate.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\_core.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\about.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\autoaway.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\background.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\dock.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\filter.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\floaty.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\forward.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\gpg.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\icons.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\icq.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\jabber.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\loger.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\msn.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\navigate.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\netmonitor.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\ontop.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\osd.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\proxy.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\remote.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\shortcuts.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\splash.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\sound.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\spell.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\styles.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\transparent.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\update.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\windock.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "..\Release\plugins\styles\metal.dll"; DestDir: "{app}\plugins\styles"; Flags: ignoreversion
Source: "..\Release\plugins\styles\wood.dll"; DestDir: "{app}\plugins\styles"; Flags: ignoreversion
Source: "..\Release\plugins\styles\xpstyle.dll"; DestDir: "{app}\plugins\styles"; Flags: ignoreversion
Source: "..\Release\simremote.dll"; DestDir: "{app}"; Flags: ignoreversion regserver
Source: "..\Release\sounds\startup.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\filedone.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\message.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\file.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\sms.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\auth.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\alert.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\url.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\contacts.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\web.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\mailpager.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\sounds\error.wav"; DestDir: "{app}\sounds"; Flags: ignoreversion
Source: "..\Release\pict\splash.png"; DestDir: "{app}\pict"; Flags: ignoreversion
Source: "..\Release\pict\connect.gif"; DestDir: "{app}\pict"; Flags: ignoreversion
Source: "..\Release\styles\SIM.xsl"; DestDir: "{app}\styles"; Flags: ignoreversion
Source: "..\Release\styles\XChat.xsl"; DestDir: "{app}\styles"; Flags: ignoreversion
Source: "..\Release\styles\XChat2seconds.xsl"; DestDir: "{app}\styles"; Flags: ignoreversion
Source: "..\Release\styles\XChat3fullDate.xsl"; DestDir: "{app}\styles"; Flags: ignoreversion
Source: "..\Release\po\bg.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\ca.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\cs.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\de.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\es.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\fr.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\he.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\hu.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\it.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\nl.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\pl.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\pt_BR.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\ru.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\sk.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\sw.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\tr.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\uk.qm"; DestDir: "{app}\po"; Flags: ignoreversion
Source: "..\Release\po\zh_TW.qm"; DestDir: "{app}\po"; Flags: ignoreversion

[Icons]
Name: "{commonprograms}\SIM"; Filename: "{app}\sim.exe"
Name: "{userdesktop}\Simple Instant Messenger"; Filename: "{app}\sim.exe"; Tasks: desktopicon/user
Name: "{userstartup}\SIM"; Filename: "{app}\sim.exe"; Tasks: startup/user
Name: "{commondesktop}\Simple Instant Messenger"; Filename: "{app}\sim.exe"; Tasks: desktopicon/common
Name: "{commonstartup}\SIM"; Filename: "{app}\sim.exe"; Tasks: startup/common

[Run]
Filename: "{app}\sim.exe"; Description: "Launch Simple Instant Messenger"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: {userappdata}\sim

[Registry]
Root: HKCR; Subkey: ".uin"; ValueType: string; ValueName: ""; ValueData: "sim"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".uin"; ValueType: string; ValueName: "Content Type"; ValueData: "application/x-icq"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "sim"; ValueType: string; ValueName: ""; ValueData: "SIM File"; Flags: uninsdeletekey
Root: HKCR; Subkey: "sim\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\sim.exe,0"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "sim\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\simctrl.exe"" ""-f %1"""; Flags: uninsdeletevalue







