; *** Inno Setup version 4.0.5+ Hungarian messages with "a(z)" definite articles ***
; Copyright (C) 1999-2003 by Korn�l P�l
; E-mail: kornelpal@hotmail.com
; Hungarian Inno Setup home page: http://kornelpal.aom.hu/innosetup/
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.php
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; $jrsoftware: issrc/Files/Default.isl,v 1.32 2003/06/18 19:24:07 jr Exp $

[LangOptions]
LanguageName=Magyar
LanguageID=$040E
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=MS Shell Dlg
;DialogFontSize=8
;DialogFontStandardHeight=13
TitleFontName=Arial CE
;TitleFontSize=29
;WelcomeFontName=Verdana
;WelcomeFontSize=12
CopyrightFontName=Arial CE
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=Telep�t�
SetupWindowTitle=%1 Telep�t�
UninstallAppTitle=Elt�vol�t�
UninstallAppFullTitle=%1 Elt�vol�t�

; *** Misc. common
InformationTitle=Inform�ci�k
ConfirmTitle=Meger�s�t�s
ErrorTitle=Hiba

; *** SetupLdr messages
SetupLdrStartupMessage=A(z) %1 telep�t�sre fog ker�lni. Folytatja?
LdrCannotCreateTemp=Nem lehet �tmeneti f�jlt l�trehozni. A telep�t�s megszakadt
LdrCannotExecTemp=Az �tmeneti k�nyvt�rban nem lehet f�jlt v�grehajtani. A telep�t�s megszakadt

; *** Startup error messages
LastErrorMessage=%1.%n%nHiba %2: %3
SetupFileMissing=A(z) %1 f�jl hi�nyzik a telep�t� k�nyvt�r�b�l. Jav�tsa ki a hib�t, vagy szerezzen be egy �j m�solatot a programr�l.
SetupFileCorrupt=A telep�t�f�jlok megs�r�ltek. Szerezzen be egy �j m�solatot a programr�l.
SetupFileCorruptOrWrongVer=A telep�t�f�jlok megs�r�ltek, vagy nem kompatibilisek a Telep�t� e verzi�j�val. Jav�tsa ki a hib�t, vagy szerezzen be egy �j m�solatot a programr�l.
NotOnThisPlatform=Ez a program nem futtathat� %1 alatt.
OnlyOnThisPlatform=Ezt a programot %1 alatt kell futtatni.
WinVersionTooLowError=Ennek a programnak a %1 legal�bb %2 verzi�j�ra van sz�ks�ge.
WinVersionTooHighError=A programot nem lehet %1 %2 vagy k�s�bbi verzi�j�ra telep�teni.
AdminPrivilegesRequired=A program telep�t�s�hez rendszergazdak�nt kell bejelentkezni.
PowerUserPrivilegesRequired=A program telep�t�s�hez rendszergazdak�nt vagy a kiemelt felhaszn�l�k csoport tagjak�nt kell bejelentkezni.
SetupAppRunningError=A Telep�t� meg�llap�totta, hogy a(z) %1 jelenleg fut.%n%nK�rem z�rja be az �sszes p�ld�ny�t, majd a folytat�shoz kattintson az OK gombra, vagy a M�gse gombra a kil�p�shez.
UninstallAppRunningError=Az Elt�vol�t� meg�llap�totta, hogy a(z) %1 jelenleg fut.%n%nK�rem z�rja be az �sszes p�ld�ny�t, majd a folytat�shoz kattintson az OK gombra, vagy a M�gse gombra a kil�p�shez.

; *** Misc. errors
ErrorCreatingDir=A telep�t� nem tudta l�trehozni a(z) "%1" k�nyvt�rat
ErrorTooManyFilesInDir=Nem hozhat� l�tre f�jl a(z) "%1" k�nyvt�rban, mert az m�r t�l sok f�jlt tartalmaz

; *** Setup common messages
ExitSetupTitle=Kil�p�s a Telep�t�b�l
ExitSetupMessage=A telep�t�s m�g nem fejez�d�tt be. Ha most kil�p, a program nem lesz telep�tve.%n%nA Telep�t�t k�s�bb is futtathatja a telep�t�s befejez�s�hez.%n%nKil�p a Telep�t�b�l?
AboutSetupMenuItem=&N�vjegy...
AboutSetupTitle=A Telep�t� n�vjegye
AboutSetupMessage=%1 %2-s verzi�%n%3%n%nAz %1 honlapja:%n%4
AboutSetupNote=Magyar v�ltozat:%nCopyright (C) 1999-2003 P�l Korn�l%nE-mail: kornelpal@hotmail.com%n%nMagyar Inno Setup oldal:%nhttp://kornelpal.aom.hu/innosetup/

; *** Buttons
ButtonBack=< &Vissza
ButtonNext=&Tov�bb >
ButtonInstall=&Telep�t�s
ButtonOK=OK
ButtonCancel=M�gse
ButtonYes=&Igen
ButtonYesToAll=Igen, &mindet
ButtonNo=&Nem
ButtonNoToAll=Nem, &egyiket sem
ButtonFinish=&Befejez�s
ButtonBrowse=&Tall�z�s...

; *** "Select Language" dialog messages
SelectLanguageTitle=V�lasszon telep�t�si nyelvet
SelectLanguageLabel=V�lassza ki a telep�t�s sor�n haszn�land� nyelvet:

; *** Common wizard text
ClickNext=A folytat�shoz kattintson a Tov�bb gombra, vagy a M�gse gombra a Telep�t�b�l val� kil�p�shez.
BeveledLabel=

; *** "Welcome" wizard page
WelcomeLabel1=�dv�zli a(z) [name] Telep�t� Var�zsl�.
WelcomeLabel2=A(z) [name/ver] a sz�m�t�g�p�re fog ker�lni.%n%nA telep�t�s folytat�sa el�tt aj�nlott minden m�s fut� alkalmaz�st bez�rni.

; *** "Password" wizard page
WizardPassword=Jelsz�
PasswordLabel1=Ez a telep�t�s jelsz�val van v�dve.
PasswordLabel3=Adja meg a jelsz�t, majd kattintson a Tov�bb gombra a folytat�shoz. A jelszavakban a kis- �s a nagybet�k k�l�nb�z�nek sz�m�tanak.
PasswordEditLabel=&Jelsz�:
IncorrectPassword=A megadott jelsz� helytelen. Pr�b�lja �jra.

; *** "License Agreement" wizard page
WizardLicense=Licencszerz�d�s
LicenseLabel=Olvassa el a k�vetkez� fontos inform�ci�kat a folytat�s el�tt.
LicenseLabel3=K�rem olvassa el az al�bbi licencszerz�d�st. El kell fogadnia a szerz�d�s felt�teleit a telep�t�s folytat�sa el�tt.
LicenseAccepted=&Elfogadom a szerz�d�st
LicenseNotAccepted=&Nem fogadom el a szerz�d�st

; *** "Information" wizard pages
WizardInfoBefore=Inform�ci�k
InfoBeforeLabel=Olvassa el a k�vetkez� fontos inform�ci�kat a folytat�s el�tt.
InfoBeforeClickLabel=Ha felk�sz�lt a telep�t�s folytat�s�ra, kattintson a Tov�bb gombra.
WizardInfoAfter=Inform�ci�k
InfoAfterLabel=Olvassa el a k�vetkez� fontos inform�ci�kat a folytat�s el�tt.
InfoAfterClickLabel=Ha felk�sz�lt a telep�t�s folytat�s�ra, kattintson a Tov�bb gombra.

; *** "User Information" wizard page
WizardUserInfo=Felhaszn�l� adatai
UserInfoDesc=K�rem adja meg az adatait.
UserInfoName=&Felhaszn�l�n�v:
UserInfoOrg=&Szervezet:
UserInfoSerial=&Sorozatsz�m:
UserInfoNameRequired=Meg kell adnia egy nevet.

; *** "Select Destination Directory" wizard page
WizardSelectDir=V�lasszon c�lk�nyvt�rat
SelectDirDesc=Hova ker�lj�n telep�t�sre a(z) [name]?
SelectDirLabel=V�lassza ki a mapp�t, ahov� telep�teni k�v�nja a(z) [name]-t, majd kattintson a Tov�bb gombra.
DiskSpaceMBLabel=Legal�bb [mb] MB szabad ter�letre van sz�ks�ge.
ToUNCPathname=A Telep�t� nem tud h�l�zati �tvonalra telep�teni. Ha h�l�zatra k�v�n telep�teni, hozz� kell rendelnie egy meghajt�hoz az �tvonalat.
InvalidPath=Teljes �tvonalat �rjon be a meghajt� bet�jel�vel; p�ld�ul:%n%nC:\Alkalmaz�s%n%nvagy egy UNC �tvonalat a k�vetkez� alakban:%n%n\\kiszolg�l�\megoszt�s
InvalidDrive=A kiv�lasztott meghajt� vagy UNC megoszt�s nem l�tezik vagy nem �rhet� el. V�lasszon m�sikat.
DiskSpaceWarningTitle=Nincs el�g szabad lemezter�let a meghajt�n
DiskSpaceWarning=A Telep�t�nek legal�bb %1 KB szabad lemezter�letre van sz�ks�ge, de a kiv�lasztott meghajt�n csak %2 KB �ll rendelkez�sre.%n%nMindenk�ppen folytatni k�v�nja?
BadDirName32=K�nyvt�rnevekben nem szerepelhetnek a k�vetkez� karakterek:%n%n%1
DirExistsTitle=A k�nyvt�r m�r l�tezik
DirExists=A k�nyvt�r:%n%n%1%n%nm�r l�tezik.%n%nEbbe a k�nyvt�rba k�v�nja telep�teni a programot?
DirDoesntExistTitle=A k�nyvt�r nem l�tezik
DirDoesntExist=A k�nyvt�r:%n%n%1%n%nnem l�tezik. L�tre k�v�nja hozni a k�nyvt�rat?

; *** "Select Components" wizard page
WizardSelectComponents=�sszetev�k kiv�laszt�sa
SelectComponentsDesc=Mely �sszetev�k ker�ljenek telep�t�sre?
SelectComponentsLabel2=V�lassza ki a telep�tend� �sszetev�ket; t�r�lje a telep�teni nem k�v�nt �sszetev�ket. Kattintson a Tov�bb gombra, ha k�szen �ll a folytat�sra.
FullInstallation=Teljes telep�t�s
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=�ltal�nos telep�t�s
CustomInstallation=Egyedi telep�t�s
NoUninstallWarningTitle=Meglev� �sszetev�
NoUninstallWarning=A Telep�t� meg�llap�totta, hogy a k�vetkez� �sszetev�k m�r telep�tve vannak a sz�m�t�g�p�re:%n%n%1%n%nEzen �sszetev�k kijel�l�s�nek t�rl�se nem t�vol�tja el �ket a sz�m�t�g�p�r�l.%n%nMindenk�ppen folytatja?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel=A jelenlegi kijel�l�snek legal�bb [mb] MB szabad ter�letre van sz�ks�ge.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Jel�lj�n ki kieg�sz�t� feladatokat
SelectTasksDesc=Mely kieg�sz�t� feladatok ker�ljenek v�grehajt�sra?
SelectTasksLabel2=Jel�lje ki, mely kieg�sz�t� feladatokat hajtsa v�gre a Telep�t� a(z) [name] telep�t�se sor�n, majd kattintson a Tov�bb gombra.

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=V�lasszon mapp�t a Start men�ben
SelectStartMenuFolderDesc=Hova helyezze a Telep�t� a program parancsikonjait?
SelectStartMenuFolderLabel=V�lassza ki azt a mapp�t a Start men�ben, ahol szeretn�, hogy a Telep�t� l�trehozza a program parancsikonjait, majd kattintson a Tov�bb gombra.
NoIconsCheck=Ne hozzon l�tre ikonokat
MustEnterGroupName=Meg kell adnia egy mappanevet.
BadGroupName=A mappa nev�ben nem szerepelhetnek a k�vetkez� karakterek:%n%n%1
NoProgramGroupCheck2=&Ne hozzon l�tre mapp�t a Start men�ben

; *** "Ready to Install" wizard page
WizardReady=A Telep�t� felk�sz�lt
ReadyLabel1=A Telep�t� felk�sz�lt a(z) [name] sz�m�t�g�p�re val� telep�t�s�re.
ReadyLabel2a=Kattintson a Telep�t�s gombra a folytat�shoz, vagy a Vissza gombra a be�ll�t�sok �ttekint�s�hez, megv�ltoztat�s�hoz.
ReadyLabel2b=Kattintson a Telep�t�s gombra a folytat�shoz.
ReadyMemoUserInfo=Felhaszn�l� adatai:
ReadyMemoDir=Telep�t�si k�nyvt�r:
ReadyMemoType=Telep�t�s t�pusa:
ReadyMemoComponents=V�lasztott �sszetev�k:
ReadyMemoGroup=Start men� mapp�ja:
ReadyMemoTasks=Kieg�sz�t� feladatok:

; *** "Preparing to Install" wizard page
WizardPreparing=Felk�sz�l�s a telep�t�sre
PreparingDesc=A Telep�t� felk�sz�l a(z) [name] sz�m�t�g�p�re val� telep�t�s�re.
PreviousInstallNotCompleted=Egy kor�bbi program telep�t�se/elt�vol�t�sa nem fejez�d�tt be. �jra kell ind�tania a sz�m�t�g�p�t annak a telep�t�snek a befejez�s�hez.%n%nA sz�m�t�g�pe �jraind�t�sa ut�n ism�t futtassa a Telep�t�t a(z) [name] telep�t�s�nek befejez�s�hez.
CannotContinue=A telep�t�s nem folytathat�. A kil�p�shez kattintson a M�gse gombra.

; *** "Installing" wizard page
WizardInstalling=Telep�t�s �llapota
InstallingLabel=Legyen t�relemmel, am�g a(z) [name] sz�m�t�g�p�re val� telep�t�se folyik.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=A(z) [name] Telep�t� Var�zsl� befejez�se
FinishedLabelNoIcons=A(z) [name] telep�t�se befejez�d�tt.
FinishedLabel=A(z) [name] telep�t�se befejez�d�tt. Az alkalmaz�st a l�trehozott ikonok kiv�laszt�s�val ind�thatja.
ClickFinish=Kattintson a Befejez�s gombra a Telep�t�b�l val� kil�p�shez.
FinishedRestartLabel=A(z) [name] telep�t�s�nek befejez�s�hez �jra kell ind�tani a sz�m�t�g�pet. �jraind�tja most?
FinishedRestartMessage=A(z) [name] telep�t�s�nek befejez�s�hez �jra kell ind�tani a sz�m�t�g�pet.%n%n�jraind�tja most?
ShowReadmeCheck=Igen, szeretn�m elolvasni a FONTOS f�jlt
YesRadio=&Igen, �jraind�tom
NoRadio=&Nem, k�s�bb ind�tom �jra
; used for example as 'Run MyProg.exe'
RunEntryExec=%1 futtat�sa
; used for example as 'View Readme.txt'
RunEntryShellExec=%1 megtekint�se

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=A Telep�t�nek sz�ks�ge van a k�vetkez� lemezre
SelectDirectory=V�lasszon k�nyvt�rat
SelectDiskLabel2=Helyezze be a(z) %1. lemezt �s kattintson az OK gombra.%n%nHa a f�jlok ezen a lemezen m�shol tal�lhat�k, adja meg a mapp�t, ahol vannak. �rja be a helyes �tvonalat vagy kattintson a Tall�z�s gombra.
PathLabel=�&tvonal:
FileNotInDir2=A(z) "%1" f�jl nem tal�lhat� itt: "%2". Tegye be a megfelel� lemezt vagy v�lasszon egy m�sik mapp�t.
SelectDirectoryLabel=Adja meg a k�vetkez� lemez hely�t.

; *** Installation phase messages
SetupAborted=A telep�t�s nem fejez�d�tt be.%n%nJav�tsa ki a hib�t, �s futtassa �jra a Telep�t�t.
EntryAbortRetryIgnore=Kil�p�s: megszak�t�s, Ism�t: �j pr�ba, Tov�bb: folytat�s

; *** Installation status messages
StatusCreateDirs=K�nyvt�rak l�trehoz�sa...
StatusExtractFiles=F�jlok kibont�sa...
StatusCreateIcons=Parancsikonok l�trehoz�sa...
StatusCreateIniEntries=INI bejegyz�sek l�trehoz�sa...
StatusCreateRegistryEntries=Rendszerle�r� bejegyz�sek l�trehoz�sa...
StatusRegisterFiles=F�jlok regisztr�l�sa...
StatusSavingUninstall=Elt�vol�t� inform�ci�k ment�se...
StatusRunProgram=Telep�t�s befejez�se...
StatusRollback=V�ltoztat�sok visszavon�sa...

; *** Misc. errors
ErrorInternal2=Bels� hiba: %1
ErrorFunctionFailedNoCode=%1 megszakadt
ErrorFunctionFailed=%1 megszakadt; k�d: %2
ErrorFunctionFailedWithMessage=%1 megszakadt; k�d: %2.%n%3
ErrorExecutingProgram=Nem hajthat� v�gre a f�jl:%n%1

; *** Registry errors
ErrorRegOpenKey=Nem nyithat� meg a rendszerle�r� kulcs:%n%1\%2
ErrorRegCreateKey=Nem hozhat� l�tre a rendszerle�r� kulcs:%n%1\%2
ErrorRegWriteKey=Nem m�dos�that� a rendszerle�r� kulcs:%n%1\%2

; *** INI errors
ErrorIniEntry=Hiba az INI bejegyz�s l�trehoz�sa k�zben a(z) "%1" f�jlban.

; *** File copying errors
FileAbortRetryIgnore=Kil�p�s: megszak�t�s, Ism�t: �j pr�ba, Tov�bb: a f�jl �tl�p�se (nem aj�nlott)
FileAbortRetryIgnore2=Kil�p�s: megszak�t�s, Ism�t: �j pr�ba, Tov�bb: folytat�s (nem aj�nlott)
SourceIsCorrupted=A forr�sf�jl megs�r�lt
SourceDoesntExist=A(z) "%1" forr�sf�jl nem l�tezik
ExistingFileReadOnly=A f�jl csak olvashat�k�nt van jel�lve.%n%nKil�p�s: megszak�t�s, Ism�t: csak olvashat� jel�l�s megsz�ntet�se, �s �jra pr�ba, Tov�bb: a f�jl �tl�p�se (nem aj�nlott)
ErrorReadingExistingDest=Hiba l�pett fel a f�jl olvas�sa k�zben:
FileExists=A f�jl m�r l�tezik.%n%nFel�l k�v�nja �rni?
ExistingFileNewer=A megl�v� f�jl �jabb a telep�t�sre ker�l�n�l. Aj�nlott a megl�v� f�jl megtart�sa.%n%nMeg k�v�nja tartani a megl�v� f�jlt?
ErrorChangingAttr=Hiba l�pett fel a f�jl attrib�tum�nak m�dos�t�sa k�zben:
ErrorCreatingTemp=Hiba l�pett fel a f�jl c�lk�nyvt�rban val� l�trehoz�sa k�zben:
ErrorReadingSource=Hiba l�pett fel a forr�sf�jl olvas�sa k�zben:
ErrorCopying=Hiba l�pett fel a f�jl m�sol�sa k�zben:
ErrorReplacingExistingFile=Hiba l�pett fel a megl�v� f�jl cser�je k�zben:
ErrorRestartReplace=A f�jl cser�je az �jraind�t�s ut�n sikertelen volt:
ErrorRenamingTemp=Hiba l�pett fel f�jl c�lk�nyvt�rban val� �tnevez�se k�zben:
ErrorRegisterServer=Nem lehet regisztr�lni a DLL-t/OCX-et: %1
ErrorRegisterServerMissingExport=A DllRegisterServer export nem tal�lhat�
ErrorRegisterTypeLib=Nem lehet regisztr�lni a t�pust�rat: %1

; *** Post-installation errors
ErrorOpeningReadme=Hiba l�pett fel a FONTOS f�jl megnyit�sa k�zben.
ErrorRestartingComputer=A Telep�t� nem tudta �jraind�tani a sz�m�t�g�pet. Ind�tsa �jra k�zileg.

; *** Uninstaller messages
UninstallNotFound=A(z) "%1" f�jl nem l�tezik. Nem t�vol�that� el.
UninstallOpenError=A(z) "%1" f�jl nem nyithat� meg. Nem t�vol�that� el
UninstallUnsupportedVer=A(z) "%1" elt�vol�t�si napl�f�jl form�tum�t nem tudja felismerni az elt�vol�t� e verzi�ja. Az elt�vol�t�s nem folytathat�
UninstallUnknownEntry=Egy ismeretlen bejegyz�s (%1) tal�lhat� az elt�vol�t�si napl�f�jlban
ConfirmUninstall=Biztos benne, hogy elt�vol�tja a(z) %1 programot �s minden �sszetev�j�t?
OnlyAdminCanUninstall=Ezt a telep�t�st csak adminisztr�ci�s jogokkal rendelkez� felhaszn�l� t�vol�thatja el.
UninstallStatusLabel=K�rem v�rjon, am�g a(z) %1 sz�m�t�g�p�r�l val� elt�vol�t�sa befejez�dik.
UninstalledAll=A(z) %1 sikeresen el lett t�vol�tva a sz�m�t�g�pr�l.
UninstalledMost=A(z) %1 elt�vol�t�sa befejez�d�tt.%n%nN�h�ny elemet nem lehetetett elt�vol�tani. T�r�lje k�zileg.
UninstalledAndNeedsRestart=A(z) %1 elt�vol�t�s�nak befejez�s�hez �jra kell ind�tania a sz�m�t�g�p�t.%n%n�jraind�tja most?
UninstallDataCorrupted=A(z) "%1" f�jl s�r�lt. Nem t�vol�that� el.

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=T�rli a megosztott f�jlt?
ConfirmDeleteSharedFile2=A rendszer azt jelzi, hogy a k�vetkez� megosztott f�jlra nincs m�r sz�ks�ge egyetlen programnak sem. Elt�vol�tja a megosztott f�jlt?%n%nHa m�s programok m�g mindig haszn�lj�k a megosztott f�jlt, akkor az elt�vol�t�sa ut�n lehet, hogy nem fognak megfelel�en m�k�dni. Ha bizonytalan, v�lassza a Nemet. A f�jl megtart�sa nem okoz probl�m�t a rendszerben.
SharedFileNameLabel=F�jln�v:
SharedFileLocationLabel=Helye:
WizardUninstalling=Elt�vol�t�s �llapota
StatusUninstalling=%1 elt�vol�t�sa...

