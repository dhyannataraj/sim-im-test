; *** Inno Setup version 4.0.0+ Italian messages ***
; Translated by Ciro Ferrara ciferrara@tin.it
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.php
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; $jrsoftware: issrc/Files/Default.isl,v 1.31 2003/04/14 18:53:12 jr Exp $

[LangOptions]
LanguageName=English
LanguageID=$0409
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
SetupAppTitle=Installazione
SetupWindowTitle=Installazione - %1
UninstallAppTitle=Disinstalla
UninstallAppFullTitle=%1 Disinstalla

; *** Misc. common
InformationTitle=Informazioni
ConfirmTitle=Conferma
ErrorTitle=Errore

; *** SetupLdr messages
SetupLdrStartupMessage=%1 sarà installato, vuoi continuare?
LdrCannotCreateTemp=Non è possibile creare il file temporaneo. Installazione interrotta.
LdrCannotExecTemp=Non è possibile eseguire il file nella cartella temporanea. Installazione interrotta.

; *** Startup error messages
LastErrorMessage=%1.%n%nError %2: %3
SetupFileMissing=Il file %1 non è presente nella cartella di installazione. Correggere il problema o provare con una diversa copia del programma.
SetupFileCorrupt=Il file di installazione è corrotto.. Correggere il problema o provare con una diversa copia del programma.
SetupFileCorruptOrWrongVer=Il file di installazione è corrotto o incompatibile.
NotOnThisPlatform=Questo programa non si avvia su %1.
OnlyOnThisPlatform=Questo programma è per %1.
WinVersionTooLowError=Questo programma richiede %1 versione %2 o posteriore.
WinVersionTooHighError=Questo programma non può essere installato in %1 versione %2 o posteriore.
AdminPrivilegesRequired=Solo l' amministatore può installare il programma.
PowerUserPrivilegesRequired=Solo l' amministatore o un membro dei Power Users può installare il programma.
SetupAppRunningError=L'installazione ha rilevato che %1 è in esecuzione.%n%nChiudere le istanze precedenti e premere OK per continuare oppure Cancella per uscire.
UninstallAppRunningError=La disinstallazione ha rilevato che %1 è in esecuzione.%n%nChiudere le istanze precedenti e premere OK per continuare oppure Cancella per uscire.

; *** Misc. errors
ErrorCreatingDir=La cartella "%1" non è stata creata.
ErrorTooManyFilesInDir=Non è possibile creare la cartella "%1" perché contiene troppi files.

; *** Setup common messages
ExitSetupTitle=Uscita dall'installazione.
ExitSetupMessage=L'installazione non è completa, se viene interrotta il programma non sarà installato.%n%nPuoi riprovare in un secondo momento.
AboutSetupMenuItem=&Informazioni sull'installazione...
AboutSetupTitle=Informazioni sull'installazione
AboutSetupMessage=%1 versione %2%n%3%n%n%1 home page:%n%4
AboutSetupNote=

; *** Buttons
ButtonBack=< &Indietro
ButtonNext=&Avanti >
ButtonInstall=&Installa
ButtonOK=OK
ButtonCancel=Annulla
ButtonYes=&Si
ButtonYesToAll=Si a &Tutti
ButtonNo=&No
ButtonNoToAll=N&o a Tutti
ButtonFinish=&Fine
ButtonBrowse=&Sfoglia...

; *** "Select Language" dialog messages
SelectLanguageTitle=Selezionare il linguaggio dell'installazione
SelectLanguageLabel=Selezionare il linguaggio da utilizzare durante l'installazione:

; *** Common wizard text
ClickNext=premere Avanti per continuare o Annulla per annullare l'installazione.
BeveledLabel=

; *** "Welcome" wizard page
WelcomeLabel1=Benvenuto nell'installazione guidata di [name]
WelcomeLabel2=[name/ver] sarà installato.%n%nSi raccomanda di chiudere tutte le applicazioni prima di continuare.

; *** "Password" wizard page
WizardPassword=Password
PasswordLabel1=Questa installazione è protetta da password.
PasswordLabel3=Inserire la password e premere Avanti per continuare. Rispettare le maiuscole e minuscole.
PasswordEditLabel=&Password:
IncorrectPassword=La password non è corretta. Riprovare.

; *** "License Agreement" wizard page
WizardLicense=Accordo contrattuale.
LicenseLabel=Leggere le seguenti importanti informazioni prima di continuare.
LicenseLabel3=Leggere le norme contrattuali, devono essere accettate prima di proseguire con l'installazione.
LicenseAccepted= &accetto
LicenseNotAccepted= &non accetto

; *** "Information" wizard pages
WizardInfoBefore=Informazioni
InfoBeforeLabel=Leggere le seguenti importanti informazioni prima di continuare.
InfoBeforeClickLabel=Quando sei pronto per continuare premi Avanti.
WizardInfoAfter=Informazioni
InfoAfterLabel=Leggere le seguenti importanti informazioni prima di continuare.
InfoAfterClickLabel=Quando sei pronto per continuare premi Avanti.

; *** "User Information" wizard page
WizardUserInfo=Informazioni dell'utente
UserInfoDesc=Prego inserire le proprie informazioni.
UserInfoName=&User Name:
UserInfoOrg=&Organization:
UserInfoSerial=&Serial Number:
UserInfoNameRequired=Si deve inserire il nome.

; *** "Select Destination Directory" wizard page
WizardSelectDir=Selezionare la cartella di destinazione.
SelectDirDesc=Dove installare [name] ?
SelectDirLabel=Selezionare la cartella dove installare [name] prima di continuare.
DiskSpaceMBLabel=Il programma richiede almeno [mb] MB di spazio sul disco.
ToUNCPathname=L'installazione non può utilizzare un percorso UNC. Per installare in rete è necessario mappare un disco di rete.
InvalidPath=Inserire un percorso completo come:%n%nC:\APP%n%nod un percorso UNC come:%n%n\\server\share
InvalidDrive=Il drive UNC non esiste o non è accessibile.
DiskSpaceWarningTitle=Spazio sul disco non adeguato.
DiskSpaceWarning=L'installazione richiede almeno %1 KB di spazio, il drive selezionato ha solo %2 KB di spazio.%n%nContinuare?
BadDirName32=I nomi di cartella non possono contenere i seguenti caratteri:%n%n%1
DirExistsTitle=La cartella già esiste!
DirExists=La cartella%n%n%1%n%nesiste gia! Usarla per l'installazione?
DirDoesntExistTitle=La cartella non esiste.
DirDoesntExist=La cartella%n%n%1%n%nnon esiste! Creare la cartella?

; *** "Select Components" wizard page
WizardSelectComponents=Componenti selezionati
SelectComponentsDesc=Quali componenti installare?
SelectComponentsLabel2=Selezionare i componenti da installare, eliminare i componenti da non installare. Premi Avanti terminata la selezione.
FullInstallation=Completa
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Compatta
CustomInstallation=Personalizzata
NoUninstallWarningTitle=Il componente esiste già.
NoUninstallWarning=L'installazione ha trovato i seguenti componenti già installati:%n%n%1%n%nDeselezionado questi componenti non saranno disinstallati.%n%nContinuare?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel=La versione corrente richiede almeno [mb] MB di spazio su disco.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Selezionare task aggiuntivi
SelectTasksDesc=Quali task addizionali eseguire?
SelectTasksLabel2=Selezionare i task addizionali da eseguire installando [name].

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Selezionare la cartella di partenza del menu.
SelectStartMenuFolderDesc=Inserire un accesso rapido?
SelectStartMenuFolderLabel=Selezionare la cartella di partenza del menu nel quale inserire un accesso rapido.
NoIconsCheck=&Non creare icone
MustEnterGroupName=Inserire il nome della cartella.
BadGroupName=Il nome della cartella non può contenere i seguenti caratteri:%n%n%1
NoProgramGroupCheck2=&Non creare una Start Menu folder

; *** "Ready to Install" wizard page
WizardReady=Pronto per l'installazione
ReadyLabel1=L'installazione è pronta per scrivere [name] sul tuo computer.
ReadyLabel2a=Premi Installa per continuare l'installazione o Indietro per variare le scelte precedenti.
ReadyLabel2b=Premi installa per proseguire.
ReadyMemoUserInfo=Informazioni utente:
ReadyMemoDir=Cartella di destinazione:
ReadyMemoType=Tipo di installazione:
ReadyMemoComponents=Componenti selezionati:
ReadyMemoGroup=Cartella di partenza:
ReadyMemoTasks=Task aggiuntivi:

; *** "Preparing to Install" wizard page
WizardPreparing=Preparando l'installazione:
PreparingDesc=Si sta preparando l'installazione di [name] sul tuo computer.
PreviousInstallNotCompleted=L'installazione/disinstallazione di un programma precedente non è stata completata. Fare ripartire il computer per terminare l'installazione.%n%nSuccessivamente procedere nuovamente con l'installazione.
CannotContinue=L'installazione non può continuare. Premi annulla per terminare.

; *** "Installing" wizard page
WizardInstalling=Installando
InstallingLabel=Attendere finchè [name] è installato.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Completando l'installazione guidata di [name]
FinishedLabelNoIcons=[name] è stato installato.
FinishedLabel=[name] è stato installato. L'applicazione può essere lanciata selezionando le icone.
ClickFinish=Premi Fine per uscire dall'installazione.
FinishedRestartLabel=Per completare l'installazione di [name] si deve fare ripartire il computer. Procedere ora?
FinishedRestartMessage=Per completare l'installazione di [name] si deve fare ripartire il computer.%n%nProcedere ora?
ShowReadmeCheck=Si, desidero vedere le informazioni.
YesRadio=&Si,fai ripartire il computer.
NoRadio=&No, farò ripartire il computer in seguito.
; used for example as 'Run MyProg.exe'
RunEntryExec=Esegui %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Analizza %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=L'installazione richiede il disco successivo.
SelectDirectory=Seleziona la cartella.
SelectDiskLabel2=Inserire il disco %1 e selezionare OK.%n%nSe i file fossero in una diversa cartella inserire il percorso o selezionare sfoglia.
PathLabel=&Percorso:
FileNotInDir2=Il file "%1" non esiste in "%2". Inserire il percorso corretto.
SelectDirectoryLabel=Specificare la posizione del disco successivo.

; *** Installation phase messages
SetupAborted=L'installazione non è stata completata.%n%nCorreggere il problema e riprovare.
EntryAbortRetryIgnore=Selezionare Riprova per per provare ancora, Ignora per continuare, Abortisci per eliminare l'installazione.

; *** Installation status messages
StatusCreateDirs=Creando le cartelle...
StatusExtractFiles=Estraendo i files...
StatusCreateIcons=Creando le icone...
StatusCreateIniEntries=Creando le voci INI...
StatusCreateRegistryEntries=Creando le voci del registro...
StatusRegisterFiles=Registrando i files...
StatusSavingUninstall=Salvando le informazioni di disinstallazione...
StatusRunProgram=Terminando l'installazione...
StatusRollback=Annullando le modifiche...

; *** Misc. errors
ErrorInternal2=Internal error: %1
ErrorFunctionFailedNoCode=%1 failed
ErrorFunctionFailed=%1 failed; code %2
ErrorFunctionFailedWithMessage=%1 failed; code %2.%n%3
ErrorExecutingProgram=Unable to execute file:%n%1

; *** Registry errors
ErrorRegOpenKey=Errore nell'apertura della chiave del registro%n%1\%2
ErrorRegCreateKey=Errore nella creazione della chiave del registro%n%1\%2
ErrorRegWriteKey=Errore scrivendo la chiave del registro%n%1\%2

; *** INI errors
ErrorIniEntry=Errore in creazione INI entry in file "%1".

; *** File copying errors
FileAbortRetryIgnore=Click Retry to try again, Ignore to skip this file (not recommended), or Abort to cancel installation.
FileAbortRetryIgnore2=Click Retry to try again, Ignore to proceed anyway (not recommended), or Abort to cancel installation.
SourceIsCorrupted=Il file sorgente è corrotto
SourceDoesntExist=Il file sorgente "%1" non esiste
ExistingFileReadOnly=Il file è di sola lettura.%n%nPremi Retry rimuovere l'attributo read-only e riptovare, Ignore salta il file, o Abort to annulla l installazione.
ErrorReadingExistingDest=Si è verificato un errore leggendo il file.:
FileExists=Il file esiste già.%n%nVuoi sovrascriverlo?
ExistingFileNewer=Il file esistente è più recente di quello da installare, è raccomandato di conservarlo.%n%nVuoi conservarlo?
ErrorChangingAttr=Un errore è occorso tentando di cambiare gli attributi del file:
ErrorCreatingTemp=Un errore è occorso cercando di creare un file nella cartella di destinazione:
ErrorReadingSource=Un errore è occorso cercando di leggere il file sorgente:
ErrorCopying=Un errore è occorso copiando un file:
ErrorReplacingExistingFile=Un errore è occorso cercando di sostituire un file esistente:
ErrorRestartReplace=RestartReplace fallito:
ErrorRenamingTemp=Un errore è occorso rinominando un file nella cartella destinazione:
ErrorRegisterServer=Incapace di registrare la DLL/OCX: %1
ErrorRegisterServerMissingExport=DllRegisterServer export introvabile.
ErrorRegisterTypeLib=Incapace di registrare la  type library: %1

; *** Post-installation errors
ErrorOpeningReadme=Un errore è occorso aprendo il file README.
ErrorRestartingComputer=L'installazione non è riuscita a fare ripartire il computer. Procedere manualmente.

; *** Uninstaller messages
UninstallNotFound=Il file "%1" non esiste, impossibile disinstallare.
UninstallOpenError=file "%1" non può essere aperto, La disinstallazione viene annullata.
UninstallUnsupportedVer=Il log file di disinstallazione "%1" è in un formato non riconosciuto, impossibile disinstallare.
UninstallUnknownEntry=Una voce non riconosciuta (%1) è stata incontrata nella lista di disinstallazione.
ConfirmUninstall=Vuoi realmente rimuovere %1 e tutti i suoi componenti?
OnlyAdminCanUninstall=TSolo un utente con privilegi di amministratore può disinstallare.
UninstallStatusLabel=Attendere sino a che %1 sia disinstallato.
UninstalledAll=%1 è stato disinstallato.
UninstalledMost=%1 è stato disinstallato.%n%nQualche elemento non può essere rimosso.
UninstalledAndNeedsRestart=Per terminare la disinstallazione di %1 il computer deve esesere fatto ripartire.%n%nRipartire ora?
UninstallDataCorrupted="%1" è corrotto. Impossibile disinstallare.

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Rimuovere il file condiviso?
ConfirmDeleteSharedFile2=Il sistema indica che il file condiviso non è usato da alcun programma. Vuoi disinstallarlo?%n%nIn caso di dubbi lasciare il file sul computer. Non produrrà danni.
SharedFileNameLabel=Nome del file:
SharedFileLocationLabel=Posizione:
WizardUninstalling=Stato della disinstallazione
StatusUninstalling=Disinstallando %1...

