; Translation made with Translator 1.32 (http://www2.arnes.si/~sopjsimo/translator.html)
; $Translator:NL=%n:TB=%t
;
; *** Inno Setup version 4.0.0 Catalan messages ***
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; Traduït a Català per José Manuel Pérez (Xose) - Barcelona
; e-mail: xosem@cablecat.com
; Revisat a partir de la versió 2.08 i posteriors per Gerard Visent Molné
; amb l'ajuda inestimable d'Albert P. Martí (Larry)
; e-mail: gerard@zootec.ad
; Actualitzat des de la versió 3.0.5+ a la 4.0.0 per Vicent LLácer Gil
; e-mail: llacer@users.sourceforge.net
;

[LangOptions]
LanguageName=Català
LanguageID=$0403
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=MS Shell Dlg
;DialogFontSize=8
;DialogFontStandardHeight=13
;TitleFontName=Arial
;TitleFontSize=29
;WelcomeFontName=Verdana
;WelcomeFontSize=12
;CopyrightFontName=Arial
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=Instal·lació
SetupWindowTitle=Instal·lació - %1
UninstallAppTitle=Desinstal·lar
UninstallAppFullTitle=Desinstal·lar %1

; *** Icons

; *** Misc. common
InformationTitle=Informació
ConfirmTitle=Confirma
ErrorTitle=Error

; *** SetupLdr messages
SetupLdrStartupMessage=Aquest programa instal·larà %1. Voleu continuar?
LdrCannotCreateTemp=No s'ha pogut crear el directori temporal. Instal·lació cancel·lada
LdrCannotExecTemp=No s'ha pogut executar el fitxer en el directori temporal. Instal·lació cancel·lada

; *** Startup error messages
LastErrorMessage=%1.%n%nError %2: %3
SetupFileMissing=L'arxiu %1 no es troba al directori d'instal·lació. Si us plau, solucioneu el problema o obteniu una nova còpia del programa.
SetupFileCorrupt=Els arxius d'instal·lació estan corromputs. Si us plau, obteniu una nova còpia del programa.
SetupFileCorruptOrWrongVer=Els arxius d'instal·lació estan danyats o són incompatibles amb aquesta versió del programa. Si us plau, solucioneu el problema o obteniu una nova còpia del programa.
NotOnThisPlatform=Aquest programa no funcionarà sota %1.
OnlyOnThisPlatform=Aquest programa només pot ser executat sota %1.
WinVersionTooLowError=Aquest programa requereix %1 versió %2 o posterior.
WinVersionTooHighError=Aquest programa no pot ser instal·lat sota %1 versió %2 o posterior.
AdminPrivilegesRequired=Heu de tenir privilegis d'administrador per poder instal·lar aquest programa
PowerUserPrivilegesRequired=Cal ser un administrador del sistema o bé un membre del grup d'usuaris amb privilegis per insta·lar aquest programa.
SetupAppRunningError=El programa d'instal·lació ha detectat que %1 s'està executant actualment.%n%nSi us plau, tanqueu el programa i premeu 'Següent' per continuar o 'Cancel·lar' per sortir.
UninstallAppRunningError=El programa de desinstal·lació ha detectat que %1 s'està executant actualment.%n%nSi us plau, tanqueu el programa i premeu 'Següent' per continuar o 'Cancel·lar' per sortir.

; *** Misc. errors
ErrorCreatingDir=El programa d'instal·lació no ha pogut crear el directori "%1"
ErrorTooManyFilesInDir=No s'ha pogut crear un arxiu al directori "%1" perque aquest conté massa arxius

; *** Setup common messages
ExitSetupTitle=Sortir
ExitSetupMessage=La instal·lació no s'ha completat. Si sortiu ara, el programa no serà instal·lat.%n%nPodeu tornar a executar el programa d'instal·lació quan vulgueu per completar-la.%n%nVoleu sortir?
AboutSetupMenuItem=&Sobre la instal·lació...
AboutSetupTitle=Sobre la instal·lació
AboutSetupMessage=%1 versió %2%n%3%n%nPàgina web de %1:%n%4
AboutSetupNote=

; *** Buttons
ButtonBack=< &Tornar
ButtonNext=&Següent >
ButtonInstall=&Instal·lar
ButtonOK=Següent
ButtonCancel=Cancel·lar
ButtonYes=&Sí
ButtonYesToAll=Sí a &Tot
ButtonNo=&No
ButtonNoToAll=N&o a tot
ButtonFinish=&Finalitzar
ButtonBrowse=&Explorar...

; *** "Select Language" dialog messages
SelectLanguageTitle=Elegisca l'idioma de l'instal·lador
SelectLanguageLabel=Elegisca l'idioma a utilitzar durant la instal·lació:

; *** Common wizard text
ClickNext=Premeu 'Següent' per continuar o 'Cancel·lar' per sortir del programa.

; *** "Welcome" wizard page
BeveledLabel=

; *** "Password" wizard page
WelcomeLabel1=Benvingut a l'assistent d'instal·lació de [name]
WelcomeLabel2=S'instal·larà [name/ver] al seu ordinador.%n%nEs recomana que es tanqueu totes les aplicacions que s'estan executant abans de continuar. Això ajudarà a prevenir conflictes durant el procés d'instal·lació.
WizardPassword=Codi d'accés
PasswordLabel1=Aquesta instal·lació està protegida amb un codi d'accés.
PasswordLabel3=Indiqueu el codi d'accés i premeu 'Següent' per continuar. El codi és sensible a les majúscules/minúscules.
PasswordEditLabel=&Codi:
IncorrectPassword=El codi introduït no és correcte. Torneu-ho a intentar.

; *** "License Agreement" wizard page
WizardLicense=Acceptació de la llicencia d'ús.
LicenseLabel=Si us plau, llegiu aquesta informació important abans de continuar.

; *** "Information" wizard pages
LicenseLabel3=Si us plau, llegiu el següent Acord de Llicència. Heu d'acceptar els termes d'aquest acord abans de continuar amb l'instal·lació.
LicenseAccepted=&Accepto l'acord
LicenseNotAccepted=&No accepto l'acord
WizardInfoBefore=Informació
InfoBeforeLabel=Si us plau, llegiu la següent informació abans de continuar.
InfoBeforeClickLabel=Quan esteu preparat per continuar, premeu 'Següent >'
WizardInfoAfter=Informació
InfoAfterLabel=Si us plau, llegiu la següent informació abans de continuar.
InfoAfterClickLabel=Quan esteu preparat per continuar, premeu 'Següent >'

; *** "Select Destination Directory" wizard page
WizardUserInfo=Informació sobre l'usuari
UserInfoDesc=Si us plau, entreu la vostra informació.
UserInfoName=&Nom de l'usuari:
UserInfoOrg=&Organització
UserInfoSerial=&Número de sèrie:
UserInfoNameRequired=Heu d'entrar un nom.
WizardSelectDir=Triar Directori de Destí
; the %1 below is changed to either DirectoryOld or DirectoryNew
; depending on whether the user is running Windows 3.x, or 95 or NT 4.0
SelectDirDesc=On s'ha d'instal·lar [name]?
SelectDirLabel=Trieu el directori on voleu instal·lar [name], i després premeu 'Següent'.
DiskSpaceMBLabel=Aquest programa necessita un mínim de [mb] MB d'espai en disc.
ToUNCPathname=El programa d'instal·lació no pot instal·lar el programa en un directori UNC. Si esteu  provant d'instal·lar-lo en xarxa, haureu d'assignar una lletra (D:,E:,etc...) al disc de destí.
InvalidPath=Cal informar un camí complet amb lletra d'unitat, per exemple:%n%nC:\Aplicació%n%no bé un camí UNC en la forma:%n%n\\servidor\compartit
InvalidDrive=El disc o camí de xarxa seleccionat no existeix, si us plau trieu-ne un altre.
DiskSpaceWarningTitle=No hi ha prou espai en disc
DiskSpaceWarning=El programa d'instal·lació necessita com a mínim %1 KB d'espai lliure, però el disc seleccionat només té %2 KB disponibles.%n%nTot i així, voleu continuar?
BadDirName32=Un nom de directori no pot contenir cap dels caracters següents:%n%n%1
DirExistsTitle=El directori existeix
DirExists=El directori:%n%n%1%n%nja existeix. Voleu instal·lar el programa en aquest directori?
DirDoesntExistTitle=El directori no existeix
DirDoesntExist=El directori:%n%n%1%n%nno existeix. Voleu crear-lo?

; *** "Select Program Group" wizard page
WizardSelectComponents=Triar Components
SelectComponentsDesc=Quins components cal instal·lar?
SelectComponentsLabel2=Seleccioneu únicament els components que voleu instal·lar. Premeu 'Següent' per continuar.
FullInstallation=Instal·lació completa
CompactInstallation=Instal·lació mínima
CustomInstallation=Instal·lació personalitzada
NoUninstallWarningTitle=Els components ja existeixen
NoUninstallWarning=El programa d'instal·lació ha detectat que els següents components ja es troben al seu ordinador:%n%n%1%n%nEl fet que no estiguin seleccionats no els desinstal·larà.%n%nVoleu continuar?
ComponentSize1=%1 Kb
ComponentSize2=%1 Mb
ComponentsDiskSpaceMBLabel=La selecció de components actual requereix un mínim de [mb] mb d'espai lliure en el disc.
WizardSelectTasks=Triar tasques adicionals
SelectTasksDesc=Quines tasques adicionals cal executar?
SelectTasksLabel2=Treu les tasques adicionals que voleu que s'executin durant la instal·lació de [name], després premeu 'Següent'.
WizardSelectProgramGroup=Seleccionar un Grup de Programes
; the %1 below is changed to either ProgramManagerOld or ProgramManagerNew
; depending on whether the user is running Windows 3.x, or 95 or NT 4.0
SelectStartMenuFolderDesc=On voleu que el programa d'instal·lació creï els enllaços?
SelectStartMenuFolderLabel=Trieu la carpeta del Menú Inici on voleu que el programa d'instal·lació creï els enllaços, després premeu 'Següent'.
NoIconsCheck=&No crear cap icona
MustEnterGroupName=Heu d'informar un nom pel grup de programes.
BadGroupName=El nom del grup no pot contenir cap dels següents caràcters:%n%n%1

; *** "Ready to Install" wizard page
NoProgramGroupCheck2=&No crear una carpeta al Menú Inici
WizardReady=Preparat per instal·lar
ReadyLabel1=El programa d'instal·lació començarà ara la instal·lació de [name] al seu ordinador.
ReadyLabel2a=Premeu 'Instal·lar' per continuar amb la instal·lació, o 'Tornar' si voleu revisar o modificar les opcions d'instal·lació.
ReadyLabel2b=Premeu 'Instal·lar' per continuar amb la instal·lació.

; *** "Setup Completed" wizard page
ReadyMemoUserInfo=Informació sobre l'usuari:
ReadyMemoDir=Directori de destí:
ReadyMemoType=Tipus d'instal·lació:
ReadyMemoComponents=Components seleccionats:
ReadyMemoGroup=Carpeta del Menú Inici:
ReadyMemoTasks=Tasques adicionals:
WizardPreparing=Preparant l'instal·lació
PreparingDesc=Preparant l'instal·lació de [name] al seu ordinador.
PreviousInstallNotCompleted=L'instal·lació o desinstal·lació anterior s'ha dut a terme. Cal que reinicicieu el vostre ordinador per  finalitzar aquesta l'instal·lació o desintal·lació.%n%nDesprés de reiniciar el vostre ordinador, executeu aquest programa de nou per completar l'instal·lació de [name].
CannotContinue=L'instal·lació no pot continuar. Si us plau, premeu el botó 'Cancel·lar' per sortir.
WizardInstalling=Instal·lant
InstallingLabel=Si us plau, espereu mentre s'instal·la [name] al seu ordinador.
FinishedHeadingLabel=Finalitzant l'assistent d'instal·lació de [name]
FinishedLabelNoIcons=El programa ha finalitzat la instal·lació de [name] al seu ordinador.
FinishedLabel=El programa ha finalitzat la instal·lació de [name] al seu ordinador. Podeu executar l'aplicació seleccionant les icones instal·lades.
ClickFinish=Premeu 'Finalitzar' per sortir de la instal·lació.
FinishedRestartLabel=Per completar la instal·lació de [name], el programa ha de reiniciar el seu ordinador. Voleu que ho faci ara?
FinishedRestartMessage=Per completar la instal·lació de [name] cal reiniciar el seu ordinador.%n%nVoleu  que ho faci ara?
ShowReadmeCheck=Sí, vull veure el fitxer LLEGIUME.TXT
YesRadio=&Sí, reiniciar ara
NoRadio=&No, reiniciaré l'ordinador més tard

; *** "Setup Needs the Next Disk" stuff
RunEntryExec=Executar %1
RunEntryShellExec=Visualitzar %1
ChangeDiskTitle=El programa d'instal·lació necessita el següent disc
SelectDirectory=Seleccioneu directori
; the %2 below is changed to either SDirectoryOld or SDirectoryNew
; depending on whether the user is running Windows 3.x, or 95 or NT 4.0
SelectDiskLabel2=Si us plau, introduiu el disc %1 i premeu 'Continuar'.%n%nSi els fitxers d'aquest disc es poden trobar en un directori diferent de l'indicat a continuació, indiqueu el camí correcte o bé premeu 'Explorar' per trobar-los.
PathLabel=&Camí:
; the %3 below is changed to either SDirectoryOld or SDirectoryNew
; depending on whether the user is running Windows 3.x, or 95 or NT 4.0
FileNotInDir2=El fitxer "%1" no s'ha pogut trobar a "%2". Si us plau, introduiu el disc correcte o seleccioneu un altre directori.
SelectDirectoryLabel=Si us plau, indiqueu on es troba el següent disc.

; *** Installation phase messages
SetupAborted=La instal·lació no s'ha completat.%n%n%Solucioneu el problema abans d'executar de nou el programa d'instal·lació.
EntryAbortRetryIgnore=Trieu 'Reintentar' per tornar-ho a intentar, 'Ignorar' per continuar, o 'Cancel·lar' per cancel·lar la instal·lació.

; *** Installation status messages
StatusCreateDirs=Creant directoris...
StatusExtractFiles=Extraient arxius...
StatusCreateIcons=Creant icones de programa...
StatusCreateIniEntries=Creant entrades del fitxer INI...
StatusCreateRegistryEntries=Creant entrades de registre...
StatusRegisterFiles=Registrant arxius...
StatusSavingUninstall=Desant informació de desinstal·lació...

; *** Misc. errors
StatusRunProgram=Finalitzant la instal·lació...
StatusRollback=Retrocedint els canvis...
ErrorInternal2=Error intern: %1
ErrorFunctionFailedNoCode=%1 ha fallat
ErrorFunctionFailed=%1 ha fallat; codi %2
ErrorFunctionFailedWithMessage=%1 ha fallat; codi %2.%n%3
ErrorExecutingProgram=No és possible executar el fitxer:%n%1

; *** DDE errors

; *** Registry errors
ErrorRegOpenKey=Error obrint la clau de registre:%n%1\%2
ErrorRegCreateKey=Error creant la clau de registre:%n%1\%2
ErrorRegWriteKey=Error escrivint a la clau de registre:%n%1\%2

; *** INI errors
ErrorIniEntry=Error creant la entrada INI al fitxer "%1".

; *** File copying errors
FileAbortRetryIgnore=Trieu 'Reintentar' per tornar-ho a intentar, 'Ignorar' per continuar sense aquest fitxer (no recomanat), o 'Cancel·lar' per cancel·lar la instal·lació.
FileAbortRetryIgnore2=Trieu 'Reintentar' per tornar-ho a intentar, 'Ignorar' per continuar (no recomenat), o 'Cancel·lar' per cancel·lar la instal·lació.
SourceIsCorrupted=El fitxer d'origen està corrupte
SourceDoesntExist=El fitxer d'origen "%1" no existeix
ExistingFileReadOnly=El fitxer és només de lectura.%n%nTrieu 'Reintentar' per treure l'atribut de només lectura i tornar-ho a intentar, 'Ignorar' per continuar sense aquest fitxer, o 'Cancel·lar' per cancel·lar la instal·lació.
ErrorReadingExistingDest=S'ha produit ha hagut un error llegint el fitxer:
FileExists=El fitxer ja existeix.%n%nVoleu sobreescriure'l?
ExistingFileNewer=El fitxer existent és més actual que el que s'intenta instal·lar. Es recomana mantenir el fitxer existent.%n%nVoleu mantenir el fitxer existent?
ErrorChangingAttr=Hi ha hagut un error canviant els atributs del fitxer:
ErrorCreatingTemp=Hi ha hagut un error creant un fitxer en el directori de destí:
ErrorReadingSource=Hi ha hagut un error llegint el fitxer d'origen:
ErrorCopying=Hi ha hagut un error copiant el fitxer:
ErrorReplacingExistingFile=Hi ha hagut un error reemplaçant el fitxer:
ErrorRestartReplace=Reemplaçar ha fallat:
ErrorRenamingTemp=Hi ha hagut un error renombrant un fitxer en el directori de destí:
ErrorRegisterServer=No s'ha pogut registrar el DLL/OCX: %1
ErrorRegisterServerMissingExport=No s'ha trobat l'exportador DllRegisterServer
ErrorRegisterTypeLib=No s'ha pogut registrar la biblioteca de tipus: %1

; *** Post-installation errors
ErrorOpeningReadme=Hi ha hagut un error obrint el fitxer LLEGIUME.TXT.
ErrorRestartingComputer=El programa d'instal·lació no ha pogut reiniciar l'ordinador. Si us plau, feu-ho manualment.

; *** Uninstaller messages
UninstallNotFound=L'arxiu "%1" no existeix. No es pot desinstal·lar.
UninstallUnsupportedVer=El fitxer de desinstal·lació "%1" està en un format no reconegut per aquesta versió del desinstal·lador. No es pot desinstal·lar
UninstallUnknownEntry=S'ha trobat una entrada desconeguda (%1) al fitxer de desinstal·lació.
ConfirmUninstall=Esteu segur de voler eliminar completament %1 i tots els seus components?
OnlyAdminCanUninstall=Aquest programa només pot ser desinstal·lat per un usuari amb privilegis d'administrador.
UninstallStatusLabel=Si us plau, espereu mentre s'elimina %1 del seu ordinador.
UninstalledAll=%1 ha estat desinstal·lat correctament del seu ordinador.
UninstalledMost=Desintal·lació de %1 completada.%n%nAlguns elements no s'han pogut eliminar. Poden ser eliminats manualment.
UninstalledAndNeedsRestart=Per completar l'instal·lació de %1, cal reiniciar el vostre ordinador.%n%nVoleu reiniciar-lo ara?
UninstallDataCorrupted=El fitxer "%1" està corromput. No es pot desinstal·lar.
UninstallOpenError=El fitxer "%1" no es va poder obrir. No es pot desinstal·lar.

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Eliminar Arxiu Compartit?
ConfirmDeleteSharedFile2=El sistema indica que el següent arxiu compartit ja no el fa servir cap altre programa. Voleu eliminar aquest arxiu compartit?%n%nSi algun programa encara fa servir aquest arxiu, podria no funcionar correctament si s'elimina. Si no esteu segur, trieu 'No'. Deixar l'arxiu al sistema no farà cap mal.
SharedFileNameLabel=Nom de l'arxiu:
SharedFileLocationLabel=Localització:
WizardUninstalling=Estat de la desinstal·lació
StatusUninstalling=Desinstal·lant %1...


