; Script generated by the My Inno Setup Extensions Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=SSL
AppVerName=SSL 0.9.1.6
DefaultDirName={pf}\SIM
DisableProgramGroupPage=yes
DisableStartupPrompt=yes
DisableReadyPage=yes
Compression=bzip/9
AppId=SIM
AppMutex=SIM_Mutex
OutputBaseFilename=ssl

[Files]
Source: "C:\openssl\bin\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\openssl\bin\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion

