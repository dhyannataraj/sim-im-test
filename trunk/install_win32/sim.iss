; Script generated by the My Inno Setup Extensions Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=Simple Instant Messenger
AppVerName=SIM 0.8.1
AppPublisher=shutoff@mail.ru
AppPublisherURL=http://sim-icq.sourceforge.net/
AppSupportURL=http://sim-icq.sourceforge.net/
AppUpdatesURL=http://sim-icq.sourceforge.net/
DefaultDirName={pf}\SIM
DisableProgramGroupPage=yes
DisableReadyPage=yes
LicenseFile=C:\sim\COPYING
Compression=bzip/9
AppId=SIM
AppMutex=SIM_Mutex
AppCopyright=Copyright � 2002, Vladimir Shutoff

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Dirs]
Name: "{app}\po"
Name: "{app}\sounds"
Name: "{app}\pict"
Name: "{app}\icons"

[Files]
Source: "C:\sim\Release\sim.exe"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "C:\sim\Release\xpstyle.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "C:\sim\Release\IdleTrac.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "C:\sim\Release\po\zh_TW.qm"; DestDir: "{app}\po"; CopyMode: alwaysoverwrite
Source: "C:\sim\Release\po\de.qm"; DestDir: "{app}\po"; CopyMode: alwaysoverwrite
Source: "C:\sim\Release\po\it.qm"; DestDir: "{app}\po"; CopyMode: alwaysoverwrite
Source: "C:\sim\Release\po\ru.qm"; DestDir: "{app}\po"; CopyMode: alwaysoverwrite
Source: "C:\sim\Release\po\tr.qm"; DestDir: "{app}\po"; CopyMode: alwaysoverwrite
Source: "C:\sim\Release\po\uk.qm"; DestDir: "{app}\po"; CopyMode: alwaysoverwrite
Source: "C:\sim\sounds\url.wav"; DestDir: "{app}\sounds"; CopyMode: alwaysoverwrite
Source: "C:\sim\sounds\alert.wav"; DestDir: "{app}\sounds"; CopyMode: alwaysoverwrite
Source: "C:\sim\sounds\auth.wav"; DestDir: "{app}\sounds"; CopyMode: alwaysoverwrite
Source: "C:\sim\sounds\chat.wav"; DestDir: "{app}\sounds"; CopyMode: alwaysoverwrite
Source: "C:\sim\sounds\file.wav"; DestDir: "{app}\sounds"; CopyMode: alwaysoverwrite
Source: "C:\sim\sounds\filedone.wav"; DestDir: "{app}\sounds"; CopyMode: alwaysoverwrite
Source: "C:\sim\sounds\message.wav"; DestDir: "{app}\sounds"; CopyMode: alwaysoverwrite
Source: "C:\sim\sounds\sms.wav"; DestDir: "{app}\sounds"; CopyMode: alwaysoverwrite
Source: "C:\sim\sounds\startup.wav"; DestDir: "{app}\sounds"; CopyMode: alwaysoverwrite
Source: "C:\sim\pict\splash.png"; DestDir: "{app}\pict"; CopyMode: alwaysoverwrite
Source: "C:\qt\bin\qt-mt230nc.dll"; DestDir: "{app}"
Source: "C:\openssl\bin\libeay32.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "C:\openssl\bin\ssleay32.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "C:\windows\system32\msvcrt.dll"; DestDir: "{sys}"; Flags: uninsneveruninstall

[Icons]
Name: "{commonprograms}\SIM"; Filename: "{app}\sim.exe"
Name: "{userdesktop}\Simple Instant Messenger"; Filename: "{app}\sim.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\sim.exe"; Description: "Launch Simple Instant Messenger"; Flags: nowait postinstall skipifsilent

