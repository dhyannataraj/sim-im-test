; Script generated by the My Inno Setup Extensions Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#include "simfiles.iss"

[Setup]
OutputBaseFilename=sim-0.9.2-full

[Files]
Source: "C:\qt\bin\qt-mt230nc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\openssl\bin\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\openssl\bin\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\qt\bin\libexpat.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\qt\bin\sablot.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "msvcrt.dll"; DestDir: "{sys}"; Flags: uninsneveruninstall onlyifdoesntexist
Source: "msvcp60.dll"; DestDir: "{sys}"; Flags: uninsneveruninstall onlyifdoesntexist
Source: "opengl32.dll"; DestDir: "{sys}"; Flags: uninsneveruninstall onlyifdoesntexist
Source: "glu32.dll"; DestDir: "{sys}"; Flags: uninsneveruninstall onlyifdoesntexist

