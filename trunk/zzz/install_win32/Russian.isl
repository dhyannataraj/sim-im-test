; *** Inno Setup version 4.1.4+ English messages ***
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.php
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; $jrsoftware: issrc/Files/Default.isl,v 1.51

[LangOptions]
LanguageName=Russian
LanguageID=$0419
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=
;DialogFontSize=8
;WelcomeFontName=Verdana
;WelcomeFontSize=12
;TitleFontName=Arial
;TitleFontSize=29
;CopyrightFontName=Arial
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=Установка
SetupWindowTitle=Установка - %1
UninstallAppTitle=Деинсталляция
UninstallAppFullTitle=%1 Деинсталляция

; *** Misc. common
InformationTitle=Информация
ConfirmTitle=Подтвердить
ErrorTitle=Ошибка

; *** SetupLdr messages
SetupLdrStartupMessage=Произойдет процесс установки %1. Продолжить?
LdrCannotCreateTemp=Временный файл не создан. Прекращение установки
LdrCannotExecTemp=Ошибка открытия файла из временного каталога. Прекращение установки

; *** Startup error messages
LastErrorMessage=%1.%n%nError %2: %3
SetupFileMissing=В папке установки нет файла %1 . Исправьте ошибку или приобретите новую копию программы.
SetupFileCorrupt=Установочные файлы повреждены. Приобретите новую копию программы.
SetupFileCorruptOrWrongVer=Установочные файлы повреждены либо несовместимы с этой версией установки. Исправьте проблему либо приобретите новую копию программы.
NotOnThisPlatform=Эта программа не будет работать под %1.
OnlyOnThisPlatform=Эту программу надо запускать под %1.
WinVersionTooLowError=Программа требует %1 версии %2 или более поздней.
WinVersionTooHighError=Эту программу нельзя проинсталлировать на %1 версии %2 или более поздней.
AdminPrivilegesRequired=Вы должны войти как администратор при инсталляции этой программы.
PowerUserPrivilegesRequired=Вы должны войти как администратор или дозволенный пользователь при инсталляции этой программы.
SetupAppRunningError=Программа установки обнаружила, что запущена программа %1 .%n%nЗакройте все приложения, затем нажмите ОК для продолжения либо Отмена, чтобы выйти из программы установки.
UninstallAppRunningError=Программа деинсталляции обнаружила, что запущена программа %1 .%n%nЗакройте все приложения, затем нажмите ОК для продолжения либо Отмена, чтобы выйти из программы установки.

; *** Misc. errors
ErrorCreatingDir=Ошибка при создании папки "%1"
ErrorTooManyFilesInDir=Файл в папке "%1" не создан, так как эта папка содержит слишком много файлов

; *** Setup common messages
ExitSetupTitle=Выйти
ExitSetupMessage=Установка не завершена. Если вы сейчас выйдете, программа не будет полностью проинсталлирована.%n%nМожете запустить программу установки в другой раз.%n%nВыйти?
AboutSetupMenuItem=&Об Установке...
AboutSetupTitle=Об Установке
AboutSetupMessage=%1 версия %2%n%3%n%n%1 страница:%n%4
AboutSetupNote=

; *** Buttons
ButtonBack=< &Назад
ButtonNext=&Дальше >
ButtonInstall=&Установить
ButtonOK=ОК
ButtonCancel=Отмена
ButtonYes=&Да
ButtonYesToAll=Да &всем
ButtonNo=&Нет
ButtonNoToAll=Н&ет всем
ButtonFinish=&Конец
ButtonBrowse=&Обзор...
ButtonWizardBrowse=&Обзор...
ButtonNewFolder=&Новая папка

; *** "Select Language" dialog messages
SelectLanguageTitle=Выберите язык установки
SelectLanguageLabel=Выберите язык установки:

; *** Common wizard text
ClickNext=Дальше - продолжить, Отмена - выйти.
BeveledLabel=


BrowseDialogTitle=Выбор папки
BrowseDialogLabel=Выберите папку и нажмите ОК.
NewFolderName=Новая папка

; *** "Welcome" wizard page
WelcomeLabel1=Добро пожаловать в [name] Мастер Установки
WelcomeLabel2=произойдет процесс установки [name/ver] на ваш компьютер.%n%nРекомендуется закрыть все запущенные приложения и ОТКЛЮЧИТЬ АНТИВИРУСЫ прежде, чем продолжить установку.

; *** "Password" wizard page
WizardPassword=Пароль
PasswordLabel1=Инсталляция под паролем.
PasswordLabel3=Введите пароль и щелкните Дальше для продолжения. При введении пароля соблюдайте регистр.
PasswordEditLabel=&Пароль:
IncorrectPassword=Неверный пароль. Повторите попытку.

; *** "License Agreement" wizard page
WizardLicense=Лицензионное соглашение
LicenseLabel=Прочтите следующую информацию прежде, чем продолжить установку.
LicenseLabel3=Прочтите лицензионное соглашение. Вы должны принять условия этого соглашения, чтобы продолжить установку.
LicenseAccepted=Я &принимаю условия
LicenseNotAccepted=Я &не принимаю условия

; *** "Information" wizard pages
WizardInfoBefore=Информация
InfoBeforeLabel=Прочтите следующую важную информацию.
InfoBeforeClickLabel=Когда будете готовы приступить к установке, щелкните Дальше.
WizardInfoAfter=Информация
InfoAfterLabel=Прочтите следующую важную информацию.
InfoAfterClickLabel=Когда будете готовы приступить к установке, щелкните Дальше.

; *** "User Information" wizard page
WizardUserInfo=Информация о пользователе
UserInfoDesc=Введите свои данные.
UserInfoName=&Имя:
UserInfoOrg=&Организация:
UserInfoSerial=&Серийный номер:
UserInfoNameRequired=Вы должны ввести имя.

; *** "Select Destination Directory" wizard page
WizardSelectDir=Выберитие папку установки
SelectDirDesc=Куда установить [name] ?
SelectDirBrowseLabel=Программа [name] будет установлена в следующую папку.%n%nДля продолжения, жмите Дальше. Для выбора другой папки жмите Обзор.
DiskSpaceMBLabel=Для установки программы требуется не менее [mb] Мб свободного места.
ToUNCPathname=Программа не может выполнить установку по пути UNC. Если вы пользуетесь сетью, тогда вы должны указать путь к сетевому дисковому накопителю.
InvalidPath=Вы должны ввести полный путь; например:%n%nC:\APP%n%nили путь UNC в форме:%n%n\\server\share
InvalidDrive=Указанный вами диск или каталог не существует или недоступен. Выберите другой путь.
DiskSpaceWarningTitle=Мало свободного места на диске
DiskSpaceWarning=Программа установки требует не менее %1 КБ свободного места, а на указанном вами диске свободно всего %2 КБ.%n%nВсе равно продолжить?
DirNameTooLong=Имя или путь папки слишком длинное.
InvalidDirName=Неправильное имя папки.
BadDirName32=Имена папок не должны содержать следующих символов:%n%n%1
DirExistsTitle=Такая папка уже существует
DirExists=Папка:%n%n%1%n%nуже существует. Все равно провести установку в эту папку?
DirDoesntExistTitle=Такой папки не существует
DirDoesntExist=Папка:%n%n%1%n%nне существует. Хотите создать эту папку?

; *** "Select Components" wizard page
WizardSelectComponents=Веберите компоненты
SelectComponentsDesc=Какие компоненты установить?
SelectComponentsLabel2=Выберите компоненты установки. Щелкните Дальше для продолжения.
FullInstallation=Полная установка
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Минимальная установка
CustomInstallation=Пользовательская
NoUninstallWarningTitle=Такие компоненты уже есть
NoUninstallWarning=Программа установки обнаружила, что следующие компоненты уже проинсталлированы на вашем компьютере:%n%n%1%n%n. %n%nПродолжить?
ComponentSize1=%1 Кб
ComponentSize2=%1 Мб
ComponentsDiskSpaceMBLabel=Нужно не менее [mb] Мб своюодного места на диске.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Выберите дополнительные задачи
SelectTasksDesc=Какие дополнительные задачи выполнить?
SelectTasksLabel2=Выберите дополнительные задачи, которые должна выполнить программа при установке [name], затем щелкните Дальше.

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Выберите каталог Стартового меню
SelectStartMenuFolderDesc=Куда поместить ярлыки программы?
NoIconsCheck=&Не создавать ярлыков
MustEnterGroupName=Введите имя каталога.
GroupNameTooLong=Имя или путь папки слишклм длинное.
InvalidGroupName=Неправильное имя каталога.
BadGroupName=Имя этого каталога не должно содержать следущих символов:%n%n%1
NoProgramGroupCheck2=&Не создавать каталог в Стартовом меню

; *** "Ready to Install" wizard page
WizardReady=Все готово к установке
ReadyLabel1=Теперь программа начнет установку [name] на ваш компьютер.
ReadyLabel2a=Щелкните Установить, чтобы продолжить установку, либо щелкните Назад, если хотите сменить настройки.
ReadyLabel2b=Щелкните Установить, чтобы продолжить.
ReadyMemoUserInfo=Информация о пользователе:
ReadyMemoDir=Папка установки:
ReadyMemoType=Тип установки:
ReadyMemoComponents=Выбранные компоненты:
ReadyMemoGroup=Каталог Стартового меню:
ReadyMemoTasks=Дополнительные задачи:

; *** "Preparing to Install" wizard page
WizardPreparing=Подготовка к установке
PreparingDesc=Подготовка к установке [name] на ваш компьютер.
PreviousInstallNotCompleted=Установка/удаление предыдущей программы не было завершено. Вы должны перезапустить ваш компьютер.%n%nПосле этого снова запустите программу установки, чтобы завершить инсталляцию [name].
CannotContinue=Программа не может продолжить установку. Щелкните Отмена, чтобы выйти.

; *** "Installing" wizard page
WizardInstalling=Установка
InstallingLabel=Подождите, пока программа установки проинсталлирует [name] на ваш компьютер.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Завершение установки [name] 
FinishedLabelNoIcons=Установка [name] завершена.
FinishedLabel=Установка [name] завершена. 
ClickFinish=Щелкните Конец, чтобы выйти.
FinishedRestartLabel=Чтобы завершить установку [name], нужно перезапустить ваш компьютер. Сделать это сейчас?
FinishedRestartMessage=Чтобы завершить установку [name], нужно перезапустить ваш компьютер.%n%nСделать это сейчас?
ShowReadmeCheck=Да, я хочу просмотреть файл README 
YesRadio=&Да, перезапустить компьютер
NoRadio=&Нет, я перезапущу компьютер позже
; used for example as 'Run MyProg.exe'
RunEntryExec=Запустите %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Обзор %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=Вставьте следующий диск
SelectDiskLabel2=Вставьте диск %1 и щелкните ОК.%n%nЕсли файлы на диске можно найти в другом каталоге, а не в том, что указан ниже, тогда укажите правильный путь или щелкните Обзор.
PathLabel=&Путь:
FileNotInDir2=Файл "%1" не может находиться в "%2". Вставье диск или выберите другой каталог.
SelectDirectoryLabel=Укажите местонахождение следующего диска.

; *** Installation phase messages
SetupAborted=Установка не завершена.%n%nИсправьте проблему и запустите программу Установки еще раз.
EntryAbortRetryIgnore=Щелкните Повторить, чтобы повторить попытку, Игнорировать - чтобы все равно продолжить, или Отмена - чтобы отменить установку.

; *** Installation status messages
StatusCreateDirs=Создание папок...
StatusExtractFiles=Извлечение файлов...
StatusCreateIcons=Создание ярлыков...
StatusCreateIniEntries=Создание файлов INI...
StatusCreateRegistryEntries=Создание реестровых ссылок...
StatusRegisterFiles=Регистрация файлов...
StatusSavingUninstall=Сохранение информации...
StatusRunProgram=Завершение установки...
StatusRollback=Принятие изменений...

; *** Misc. errors
ErrorInternal2=Ошибка: %1
ErrorFunctionFailedNoCode=%1 неудача
ErrorFunctionFailed=%1 неудача; код %2
ErrorFunctionFailedWithMessage=%1 неудача; код %2.%n%3
ErrorExecutingProgram=Не удается открыть файл:%n%1

; *** Registry errors
ErrorRegOpenKey=Ошибка при открытии ключа:%n%1\%2
ErrorRegCreateKey=Ошибка при создании ключа:%n%1\%2
ErrorRegWriteKey=Ошибка при записи ключа:%n%1\%2

; *** INI errors
ErrorIniEntry=Ошибка при создании INI записи в файле "%1".

; *** File copying errors
FileAbortRetryIgnore=Щелкните Повторить, чтобы повторить попытку, Игнорировать - чтобы пропустить этот файл (не рекомендуется), или Отмена - чтобы отменить установку.
FileAbortRetryIgnore2=Щелкните Повторить, чтобы повторить попытку, Игнорировать - чтобы все равно продолжить (не рекомендуется), или Отмена - чтобы отменить установку.
SourceIsCorrupted=Исходный файл поврежден
SourceDoesntExist=Файл "%1" не существует
ExistingFileReadOnly=Исходный файл только для чтения.%n%nНажмите повтор для удаления этого атрибута и попробуйте еще раз, Игнорировать для пропуска файла, Отмена для выхода из инсталляции.
ErrorReadingExistingDest=Ошибка чтения существующего файла:
FileExists=Файл уже существует.%n%nХотите перезаписать?
ExistingFileNewer=Существующий файл новее инсталлируемого. Рекоммендуется сохранить его.%n%nСохранить существующий файл?
ErrorChangingAttr=Произошла ошибка при попытке изменения свойств существующего файла:
ErrorCreatingTemp=Ошибка при создании файла в папке установки:
ErrorReadingSource=Ошибка считывания исходного файла:
ErrorCopying=Ошибка при копировании файла:
ErrorReplacingExistingFile=Ошибка при замене существующего файла:
ErrorRestartReplace=RestartReplace неудача:
ErrorRenamingTemp=Ошибка при переименовании файла в папке установки:
ErrorRegisterServer= Регистрация DLL/OCX не удалась: %1
ErrorRegisterServerMissingExport=DllRegisterServer не найден
ErrorRegisterTypeLib=Библиотека не зарегистрирована: %1

; *** Post-installation errors
ErrorOpeningReadme=Ошибка при открытии файла README.
ErrorRestartingComputer=Программа установки не смогла перезапустить компьютер. Сделайте это сами.

; *** Uninstaller messages
UninstallNotFound=Файл "%1" не существует. Деинсталляция не может быть произведена.
UninstallOpenError=Не удается открыть файл "%1" . Деинсталляция не может быть произведена
UninstallUnsupportedVer=Формат файла "%1" не поддерживается этой версией деинсталлятора. Деинсталляция не может быть произведена
UninstallUnknownEntry=В журнале деинсталляции обнаружена неизвестная запись (%1) 
ConfirmUninstall=Вы точно хотите удалить %1 и все компоненты?
OnlyAdminCanUninstall=Эту программу может деинсталлировать только администратор.
UninstallStatusLabel=Подождите, пока будет проведено удаление %1 .
UninstalledAll=%1 успешно удален.
UninstalledMost=деинсталляция %1 проведена.%n%nНекоторые элементы не удалены. Их вы можете удалить вручную.
UninstalledAndNeedsRestart=Для завершения удаления %1, компьютер нужно перезагрузить.%n%nСделать это сейчас?
UninstallDataCorrupted=файл "%1" поврежден. Деинсталляция не может быть произведена

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Удалить файл?
ConfirmDeleteSharedFile2=Система определила, что следующий файл не используется ни одной из программ.  Удалить этот файл?%n%nНо если какие-то программы используют этот файл, тогда после его удаления они могут работать с ошибками. Если вы не уверены, выберите Нет. Никакого вреда от этого файла не будет.
SharedFileNameLabel=Имя файла:
SharedFileLocationLabel=Место:
WizardUninstalling=Статус удаления
StatusUninstalling=Удаление %1...
