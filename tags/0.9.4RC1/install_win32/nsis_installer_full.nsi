; SIMINSTALL.nsi
;
; Installer written by Tobias Franz (noragen@gmx.net), 2002 - 2006 
;
; SetCompressing Options:
SetCompress auto
SetCompressor lzma

BGGradient topc

!include "MUI.nsh"

!macro BIMAGE IMAGE PARMS
	Push $0
	GetTempFileName $0
	File /oname=$0 "${IMAGE}"
	SetBrandingImage ${PARMS} $0
	Delete $0
	Pop $0
!macroend

!macro Print text
  DetailPrint "${text}"
!macroend

;.NET 2005 Distributable Installation

!define BASE_URL http://download.microsoft.com/download
; .NET Framework
; English
!define URL_DOTNET_1033 "${BASE_URL}/5/6/7/567758a3-759e-473e-bf8f-52154438565a/dotnetfx.exe"
; German
!define URL_DOTNET_1031 "${BASE_URL}/5/6/7/567758a3-759e-473e-bf8f-52154438565a/dotnetfx.exe"
; Spanish
!define URL_DOTNET_1034 "${BASE_URL}/5/6/7/567758a3-759e-473e-bf8f-52154438565a/dotnetfx.exe"
; French
!define URL_DOTNET_1036 "${BASE_URL}/5/6/7/567758a3-759e-473e-bf8f-52154438565a/dotnetfx.exe"
; Russian
!define URL_DOTNET_1037 "${BASE_URL}/5/6/7/567758a3-759e-473e-bf8f-52154438565a/dotnetfx.exe"
; Chinese (Simplified)
!define URL_DOTNET_2052 "${BASE_URL}/5/6/7/567758a3-759e-473e-bf8f-52154438565a/dotnetfx.exe"

; ... If you need one not listed above you will have to visit the Microsoft Download site,
; select the language you are after and scan the page source to obtain the link. 

Var "LANGUAGE_DLL_TITLE"
Var "LANGUAGE_DLL_INFO"
Var "URL_DOTNET"
Var "OSLANGUAGE"
Var "DOTNET_RETURN_CODE"

;.NET 2005 Distributable Installation/


;Languages

  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  ;!insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Japanese"
  !insertmacro MUI_LANGUAGE "Korean"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
  ;!insertmacro MUI_LANGUAGE "Danish"
  ;!insertmacro MUI_LANGUAGE "Swedish"
  ;!insertmacro MUI_LANGUAGE "Norwegian"
  ;!insertmacro MUI_LANGUAGE "Finnish"
  ;!insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Portuguese"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Ukrainian"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Slovak"
  !insertmacro MUI_LANGUAGE "Croatian"
  !insertmacro MUI_LANGUAGE "Bulgarian"
  !insertmacro MUI_LANGUAGE "Hungarian"
  ;!insertmacro MUI_LANGUAGE "Thai"
  !insertmacro MUI_LANGUAGE "Romanian"
  !insertmacro MUI_LANGUAGE "Latvian"
  !insertmacro MUI_LANGUAGE "Macedonian"
  !insertmacro MUI_LANGUAGE "Estonian"
  !insertmacro MUI_LANGUAGE "Turkish"
  !insertmacro MUI_LANGUAGE "Lithuanian"
  ;!insertmacro MUI_LANGUAGE "Catalan"
  ;!insertmacro MUI_LANGUAGE "Slovenian"
  ;!insertmacro MUI_LANGUAGE "Serbian"
  ;!insertmacro MUI_LANGUAGE "SerbianLatin"
  ;!insertmacro MUI_LANGUAGE "Arabic"
  ;!insertmacro MUI_LANGUAGE "Farsi"
  ;!insertmacro MUI_LANGUAGE "Hebrew"
  ;!insertmacro MUI_LANGUAGE "Indonesian"
  ;!insertmacro MUI_LANGUAGE "Mongolian"
  ;!insertmacro MUI_LANGUAGE "Luxembourgish"




;.NET 2005 Distributable Installation /
;German
LangString DESC_REMAINING ${LANG_GERMAN} " (%d %s%s remaining)"
LangString DESC_PROGRESS ${LANG_GERMAN} "%d.%01dkB/s" ;"%dkB (%d%%) of %dkB @ %d.%01dkB/s"
LangString DESC_PLURAL ${LANG_GERMAN} "s"
LangString DESC_HOUR ${LANG_GERMAN} "hour"
LangString DESC_MINUTE ${LANG_GERMAN} "minute"
LangString DESC_SECOND ${LANG_GERMAN} "second"
LangString DESC_CONNECTING ${LANG_GERMAN} "Connecting..."
LangString DESC_DOWNLOADING ${LANG_GERMAN} "Downloading %s"
LangString DESC_SHORTDOTNET ${LANG_GERMAN} "Microsoft .Net Framework 2.0"
LangString DESC_LONGDOTNET ${LANG_GERMAN} "Microsoft .Net Framework 2.0"
LangString DESC_DOTNET_DECISION ${LANG_GERMAN} "$(DESC_SHORTDOTNET) is required.$\nIt is strongly \
  advised that you install$\n$(DESC_SHORTDOTNET) before continuing.$\nIf you choose to continue, \
  you will need to connect$\nto the internet before proceeding.$\nWould you like to download and install \
  the package now?"
LangString SEC_DOTNET ${LANG_GERMAN} "$(DESC_SHORTDOTNET) "
LangString DESC_INSTALLING ${LANG_GERMAN} "Installing"
LangString DESC_DOWNLOADING1 ${LANG_GERMAN} "Downloading"
LangString DESC_DOWNLOADFAILED ${LANG_GERMAN} "Download Failed:"
LangString ERROR_DOTNET_DUPLICATE_INSTANCE ${LANG_GERMAN} "The $(DESC_SHORTDOTNET) Installer is \
  already running."
LangString ERROR_NOT_ADMINISTRATOR ${LANG_GERMAN} "$(DESC_000022)"
LangString ERROR_INVALID_PLATFORM ${LANG_GERMAN} "$(DESC_000023)"
LangString DESC_DOTNET_TIMEOUT ${LANG_GERMAN} "The installation of the $(DESC_SHORTDOTNET) \
  has timed out."
LangString ERROR_DOTNET_INVALID_PATH ${LANG_GERMAN} "The $(DESC_SHORTDOTNET) Installation$\n\
  was not found in the following location:$\n"
LangString ERROR_DOTNET_FATAL ${LANG_GERMAN} "A fatal error occurred during the installation$\n\
  of the $(DESC_SHORTDOTNET)."
LangString FAILED_DOTNET_INSTALL ${LANG_GERMAN} "The installation of $(PRODUCT_NAME) will$\n\
  continue. However, it may not function properly$\nuntil $(DESC_SHORTDOTNET)$\nis installed."

LangString DESC_REMAINING ${LANG_RUSSIAN} " (%d %s%s remaining)"
LangString DESC_PROGRESS ${LANG_RUSSIAN} "%d.%01dkB/s" ;"%dkB (%d%%) of %dkB @ %d.%01dkB/s"
LangString DESC_PLURAL ${LANG_RUSSIAN} "s"
LangString DESC_HOUR ${LANG_RUSSIAN} "hour"
LangString DESC_MINUTE ${LANG_RUSSIAN} "minute"
LangString DESC_SECOND ${LANG_RUSSIAN} "second"
LangString DESC_CONNECTING ${LANG_RUSSIAN} "Connecting..."
LangString DESC_DOWNLOADING ${LANG_RUSSIAN} "Downloading %s"
LangString DESC_SHORTDOTNET ${LANG_RUSSIAN} "Microsoft .Net Framework 2.0"
LangString DESC_LONGDOTNET ${LANG_RUSSIAN} "Microsoft .Net Framework 2.0"
LangString DESC_DOTNET_DECISION ${LANG_RUSSIAN} "$(DESC_SHORTDOTNET) is required.$\nIt is strongly \
  advised that you install$\n$(DESC_SHORTDOTNET) before continuing.$\nIf you choose to continue, \
  you will need to connect$\nto the internet before proceeding.$\nWould you like to download and install \
  the package now?"
LangString SEC_DOTNET ${LANG_RUSSIAN} "$(DESC_SHORTDOTNET) "
LangString DESC_INSTALLING ${LANG_RUSSIAN} "Installing"
LangString DESC_DOWNLOADING1 ${LANG_RUSSIAN} "Downloading"
LangString DESC_DOWNLOADFAILED ${LANG_RUSSIAN} "Download Failed:"
LangString ERROR_DOTNET_DUPLICATE_INSTANCE ${LANG_RUSSIAN} "The $(DESC_SHORTDOTNET) Installer is \
  already running."
LangString ERROR_NOT_ADMINISTRATOR ${LANG_RUSSIAN} "$(DESC_000022)"
LangString ERROR_INVALID_PLATFORM ${LANG_RUSSIAN} "$(DESC_000023)"
LangString DESC_DOTNET_TIMEOUT ${LANG_RUSSIAN} "The installation of the $(DESC_SHORTDOTNET) \
  has timed out."
LangString ERROR_DOTNET_INVALID_PATH ${LANG_RUSSIAN} "The $(DESC_SHORTDOTNET) Installation$\n\
  was not found in the following location:$\n"
LangString ERROR_DOTNET_FATAL ${LANG_RUSSIAN} "A fatal error occurred during the installation$\n\
  of the $(DESC_SHORTDOTNET)."
LangString FAILED_DOTNET_INSTALL ${LANG_RUSSIAN} "The installation of $(PRODUCT_NAME) will$\n\
  continue. However, it may not function properly$\nuntil $(DESC_SHORTDOTNET)$\nis installed."




;.NET 2005 Distributable Installation /





; The name of the installer
Name "SIM CVS/SVN VC8 FULL-Version ${__DATE__}, ${__TIME__}"

Function .onInit
  SetSilent silent
  !insertmacro MUI_LANGDLL_DISPLAY
  SetSilent normal
  
  
  ;.NET 2005 Distributable Installation
  
    StrCpy $LANGUAGE_DLL_TITLE "Installer Language"
    StrCpy $LANGUAGE_DLL_INFO "Please select a language:"
    StrCpy $URL_DOTNET "${URL_DOTNET_1033}"
    StrCpy $OSLANGUAGE "1033"
   
  ; Insert other Language Blocks Here
   
    ;StrCpy $MUI_LANGDLL_WINDOWTITLE "Installer Language"
    ;!define MUI_LANGDLL_INFO "$LANGUAGE_DLL_INFO"
    !undef MUI_LANGDLL_WINDOWTITLE
    !undef MUI_LANGDLL_INFO
    InitPluginsDir
    SetOutPath "$PLUGINSDIR"
    ;File "Common\Plugins\*.*"
    File /r "${NSISDIR}\Plugins\*.*"
  
  ;.NET 2005 Distributable Installation /
FunctionEnd

; The file to write
OutFile "SIM-IM_SVN_${__DATE__}.exe"

; The default installation directory
InstallDir $PROGRAMFILES\SIM

; The text to prompt the user to enter a directory
LangString welcome ${LANG_ENGLISH} "SIM-ICQ - Simple Instant Messenger, written by Vladimir Shutoff, continued by SIM-IM Development Team, is licensed under the Terms of the GPL.$\nThis SIM-IM - SVN Setup was created by Tobias Franz.$\nIt is a full-functional setup and it should contain all you need to run SIM-IM.$\n$\nPlease choose your SIM-IM - Rootdirectory now, where your sim.exe is stored:"
LangString welcome ${LANG_GERMAN}  "SIM-ICQ - Simple Instant Messenger, geschrieben von Vladimir Shutoff, weitergefьhrt vom SIM-IM Development Team, wird unter den Bedingungen der GPL lizensiert.$\nDieses SIM-IM - SVN Setup wurde von Tobias Franz erstellt.$\nEs handelt sich um ein vollstдndig funktionsfдhiges Setup und es sollte alles Nцtige enthalten, um SIM-IM auszufьhren$\n$\nBitte wдhlen Sie jetzt das SIM-IM-Installationsverzeichnis aus, wo die sim.exe liegen soll:"
LangString welcome ${LANG_RUSSIAN} "SIM-ICQ - Simple Instant Messenger, создан Владимиром Шутовым и развиваемый SIM-IM Development Team под лицензией GNU GPL.$\nДанная сборка SIM - SVN Setup выполнена Tobias Franz.$\nЭто версия содержит всё необходимое для использования SIM-IM.$\n$\nПожалуйста, выберите папку, куда следует установить SIM:"
DirText "$(welcome)"

; SetXP Style
XPStyle On
;BGGradient 000000 8000A0 FFFFFF
BGGradient 000000 B400E1 FFFFFF
InstallColors FF8080 000030
AddBrandingImage left 100
Page directory dirImage
Page instfiles 

LangString message ${LANG_ENGLISH} "I can run SIM for you now.$\nShould I do this for you?"
LangString message ${LANG_GERMAN} "Ich kann SIM fьr Sie ausfьhren.$\nSoll ich das fьr Sie ьbernehmen?"
LangString message ${LANG_RUSSIAN} "SIM-IM установлен и готов к работe.$\nЗапустить SIM-IM?"
; The stuff to install


;.NET 2005 Distributable Installation

  Section $(SEC_DOTNET) SECDOTNET
    SectionIn RO
    IfSilent lbl_IsSilent
    !define DOTNETFILESDIR "Common\Files\MSNET"
    StrCpy $DOTNET_RETURN_CODE "0"
!ifdef DOTNET_ONCD_1033
    StrCmp "$OSLANGUAGE" "1033" 0 lbl_Not1033
    SetOutPath "$PLUGINSDIR"
    file /r "${DOTNETFILESDIR}\dotnetfx1033.exe"
    !insertmacro PRINT "$(DESC_INSTALLING) $(DESC_SHORTDOTNET)..."
    Banner::show /NOUNLOAD "$(DESC_INSTALLING) $(DESC_SHORTDOTNET)..."
    nsExec::ExecToStack '"$PLUGINSDIR\dotnetfx1033.exe" /q /c:"install.exe /noaspupgrade /q"'
    pop $DOTNET_RETURN_CODE
    Banner::destroy
    SetRebootFlag true
    Goto lbl_NoDownloadRequired
    lbl_Not1033:
!endif

!ifdef DOTNET_ONCD_1033
    StrCmp "$OSLANGUAGE" "1033" 0 lbl_Not1033
    SetOutPath "$PLUGINSDIR"
    file /r "${DOTNETFILESDIR}\dotnetfx1033.exe"
    !insertmacro PRINT "$(DESC_INSTALLING) $(DESC_SHORTDOTNET)..."
    Banner::show /NOUNLOAD "$(DESC_INSTALLING) $(DESC_SHORTDOTNET)..."
    nsExec::ExecToStack '"$PLUGINSDIR\dotnetfx1033.exe" /q /c:"install.exe /noaspupgrade /q"'
    pop $DOTNET_RETURN_CODE
    Banner::destroy
    SetRebootFlag true
    Goto lbl_NoDownloadRequired
    lbl_Not1033:
!endif



; Insert Other language blocks here
 
    ; the following Goto and Label is for consistencey.
    Goto lbl_DownloadRequired
    lbl_DownloadRequired:
    !insertmacro PRINT "$(DESC_DOWNLOADING1) $(DESC_SHORTDOTNET)..."
    MessageBox MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 "$(DESC_DOTNET_DECISION)" /SD IDNO \
      IDYES +2 IDNO 0
    Goto lbl_ContinueInstall
    ; "Downloading Microsoft .Net Framework"
    AddSize 153600
    nsisdl::download /TRANSLATE "$(DESC_DOWNLOADING)" "$(DESC_CONNECTING)" \
       "$(DESC_SECOND)" "$(DESC_MINUTE)" "$(DESC_HOUR)" "$(DESC_PLURAL)" \
       "$(DESC_PROGRESS)" "$(DESC_REMAINING)" \
       /TIMEOUT=30000 "$URL_DOTNET" "$PLUGINSDIR\dotnetfx.exe"
    Pop $0
    StrCmp "$0" "success" lbl_continue
    !insertmacro PRINT "$(DESC_DOWNLOADFAILED) $0"
    Abort
 
    lbl_continue:
      !insertmacro PRINT "$(DESC_INSTALLING) $(DESC_SHORTDOTNET)..."
      Banner::show /NOUNLOAD "$(DESC_INSTALLING) $(DESC_SHORTDOTNET)..."
      nsExec::ExecToStack '"$PLUGINSDIR\dotnetfx.exe" /q /c:"install.exe /noaspupgrade /q"'
      pop $DOTNET_RETURN_CODE
      Banner::destroy
      SetRebootFlag true
      ; silence the compiler
      Goto lbl_NoDownloadRequired
      lbl_NoDownloadRequired:
 
      ; obtain any error code and inform the user ($DOTNET_RETURN_CODE)
      ; If nsExec is unable to execute the process,
      ; it will return "error"
      ; If the process timed out it will return "timeout"
      ; else it will return the return code from the executed process.
      StrCmp "$DOTNET_RETURN_CODE" "" lbl_NoError
      StrCmp "$DOTNET_RETURN_CODE" "0" lbl_NoError
      StrCmp "$DOTNET_RETURN_CODE" "3010" lbl_NoError
      StrCmp "$DOTNET_RETURN_CODE" "8192" lbl_NoError
      StrCmp "$DOTNET_RETURN_CODE" "error" lbl_Error
      StrCmp "$DOTNET_RETURN_CODE" "timeout" lbl_TimeOut
      ; It's a .Net Error
      StrCmp "$DOTNET_RETURN_CODE" "4101" lbl_Error_DuplicateInstance
      StrCmp "$DOTNET_RETURN_CODE" "4097" lbl_Error_NotAdministrator
      StrCmp "$DOTNET_RETURN_CODE" "1633" lbl_Error_InvalidPlatform lbl_FatalError
      ; all others are fatal
 
    lbl_Error_DuplicateInstance:
    !insertmacro PRINT "$(ERROR_DOTNET_DUPLICATE_INSTANCE)"
    GoTo lbl_Done
 
    lbl_Error_NotAdministrator:
    !insertmacro PRINT "$(ERROR_NOT_ADMINISTRATOR)"
    GoTo lbl_Done
 
    lbl_Error_InvalidPlatform:
    !insertmacro PRINT "$(ERROR_INVALID_PLATFORM)"
    GoTo lbl_Done
 
    lbl_TimeOut:
    !insertmacro PRINT "$(DESC_DOTNET_TIMEOUT)"
    GoTo lbl_Done
 
    lbl_Error:
    !insertmacro PRINT "$(ERROR_DOTNET_INVALID_PATH)"
    GoTo lbl_Done
 
    lbl_FatalError:
    !insertmacro PRINT "$(ERROR_DOTNET_FATAL)[$DOTNET_RETURN_CODE]"
    GoTo lbl_Done
 
    lbl_Done:
    !insertmacro PRINT "$(FAILED_DOTNET_INSTALL)"
    lbl_NoError:
    lbl_IsSilent:
    lbl_ContinueInstall:
  SectionEnd
 
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SECDOTNET} $(DESC_LONGDOTNET)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;.NET 2005 Distributable Installation /



Section "Install"
  ;Page instfiles instImage
 
  ;SetBrandingImage /RESIZETOFIT ..\Release\sim-window-small.bmp
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  SetOverwrite on
  
  File ..\Release\sim.exe
  File ..\Release\simctrl.exe
  
  
  UnRegDLL  $INSTDIR\simremote.dll
  UnRegDLL  $INSTDIR\simext.dll
  
  File ..\Release\*.dll

  SetOutPath $INSTDIR\po
  File ..\Release\po\*.qm

  SetOutPath $INSTDIR\plugins

  File ..\Release\plugins\*.dll

  SetOutPath $INSTDIR\plugins\styles

  ;File ..\Release\plugins\styles\*.dll

  SetOutPath $INSTDIR\styles
  File ..\Release\styles\*.xsl

  SetOutPath $INSTDIR\icons
  File ..\Release\icons\*.jisp
  
  SetOutPath $INSTDIR\pict
  File ..\Release\pict\*.gif
  File ..\Release\pict\*.png
  
  SetOutPath $INSTDIR\copyright
  File ..\Release\copyright\COPYING
  
  SetOutPath $INSTDIR\sounds
  
  File ..\Release\sounds\*.wav
  
;Currently not working:
  Delete $INSTDIR\plugins\styles\wood.dll
  Delete $INSTDIR\plugins\styles\metal.dll
  ;Delete $INSTDIR\plugins\styles.dll
  Delete $INSTDIR\simremote.dll
  Delete $INSTDIR\simext.dll
  Delete $INSTDIR\sounds\*.dll
  ;RegDLL  ..\Release\simremote.dll
  ;RegDLL  ..\Release\simext.dll
  
;System:
SetOutPath $SYSDIR

File $%QTDIR%\lib\$%DLLQT%

File $%WINDIR%\system32\libeay32.dll
File $%WINDIR%\system32\ssleay32.dll


SectionEnd ; end the section

Section "Verknьpfungen"
CreateDirectory "$SMPROGRAMS\SIM"
  WriteIniStr "$INSTDIR\SIM.url" "InternetShortcut" "URL" "http://sim-im.berlios.de"
  WriteIniStr "$INSTDIR\Updates.url" "InternetShortcut" "URL" "http://www.sim-icq.de"
  CreateShortCut "$SMPROGRAMS\SIM\SIM-IM.lnk" "$INSTDIR\SIM.url" "" "$INSTDIR\SIM.url" 0
  CreateShortCut "$SMPROGRAMS\SIM\Updates.lnk" "$INSTDIR\Updates.url" "" "$INSTDIR\Updates.url" 0
  
  CreateShortCut "$SMPROGRAMS\SIM\SIM.lnk" "$INSTDIR\sim.exe" \
    "" "$INSTDIR\sim.exe" 0 SW_SHOWNORMAL
  ;CreateShortCut "$SMPROGRAMS\RERSVC\Readme.lnk" "$INSTDIR\Readme.txt" \
  ;  "" "%SystemRoot%\notepad.exe" 1 SW_SHOWMAXIMIZED 
  CreateShortCut "$SMPROGRAMS\SIM\Uninstaller.lnk" "$INSTDIR\uninst.exe" \
    "" "$INSTDIR\uninst.exe" 0 SW_SHOWNORMAL

WriteUninstaller "$INSTDIR\Uninst.exe"
SectionEnd ; end the section

 
Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) wurde erfolgreich deinstalliert.."
FunctionEnd
  
Function un.onInit 
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Mцchten Sie $(^Name) und alle seinen Komponenten deinstallieren?" IDYES +2
  Abort
FunctionEnd
 
Section "Uninstall" 

Delete $INSTDIR\Uninst.exe ; delete self (temporarily copying in temp directory ;))
Delete "$INSTDIR\sim.exe"
Delete "$INSTDIR\simctrl.exe"

UnRegDLL  "$INSTDIR\simremote.dll"
UnRegDLL  "$INSTDIR\simext.dll"
  
Delete "$INSTDIR\*.dll"
Delete "$INSTDIR\po\*.qm"
Delete "$INSTDIR\plugins\*.dll"
Delete "$INSTDIR\plugins\styles\*.dll"
Delete "$INSTDIR\styles\*.xsl"
Delete "$INSTDIR\icons\*.jisp"
Delete "$INSTDIR\copyright\COPYING"
Delete "$INSTDIR\SIM.url"
Delete "$INSTDIR\Updates.url"
Delete "$INSTDIR\pict\*.gif"
Delete "$INSTDIR\pict\*.png"
Delete "$INSTDIR\sounds\*.wav"

Delete "$SMPROGRAMS\SIM\SIM.lnk"
Delete "$SMPROGRAMS\SIM\SIM-IM.lnk"
Delete "$SMPROGRAMS\SIM\Updates.lnk"
Delete "$SMPROGRAMS\SIM\Uninstaller.lnk"
Delete "$SMPROGRAMS\SIM"

RMDir $INSTDIR\po
RMDir $INSTDIR\plugins\styles
RMDir $INSTDIR\plugins
RMDir $INSTDIR\styles
RMDir $INSTDIR\icons
RMDir $INSTDIR\copyright
RMDir $INSTDIR\sounds
RMDir $INSTDIR\pict
RMDir $INSTDIR

Quit
SectionEnd

Section Start
  MessageBox MB_YESNO "$(message)" IDNO done
	Exec '"$INSTDIR\sim.exe"'
	
	done:
  Quit
SectionEnd ; end the section
   
Function dirImage
	!insertmacro BIMAGE "..\Release\sim-window-small.bmp" /RESIZETOFIT
FunctionEnd
; eof