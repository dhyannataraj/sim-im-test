; Translation made with Translator 1.32 (http://www2.arnes.si/~sopjsimo/translator.html)
; $Translator:NL=%n:TB=%t
;
; *** Inno Setup version 4.1.8 Bulgarian messages ***
; Mikhail Balabanov <mishob-at-abv.bg>
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;

[LangOptions]
LanguageName=Български
LanguageID=$0402
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
DialogFontName=
[Messages]

; *** Заглавия на приложенията
SetupAppTitle=Инсталиране
SetupWindowTitle=Инсталиране - %1
UninstallAppTitle=Деинсталиране
UninstallAppFullTitle=Деинсталиране - %1


; *** Разни
InformationTitle=Информация
ConfirmTitle=Потвърждение
ErrorTitle=Грешка

; *** Съобщения на зареждащия модул
SetupLdrStartupMessage=Тази програма ще инсталира %1. Желаете ли да продължите?
LdrCannotCreateTemp=Невъзможно е създаването на временен файл. Инсталацията е прекратена
LdrCannotExecTemp=Невъзможно е стартирането на файл от временната директория. Инсталацията е прекратена

; *** Грешки при стартиране
LastErrorMessage=%1.%n%nГрешка %2: %3
SetupFileMissing=Файлът %1 липсва от инсталационната директория. Моля, отстранете проблема или се снабдете с ново копие на програмата.
SetupFileCorrupt=Инсталационните файлове са повредени. Моля, снабдете се с ново копие на програмата.
SetupFileCorruptOrWrongVer=Инсталационните файлове са повредени или несъвместими с тази версия на инсталатора. Моля, отстранете проблема или се снабдете с ново копие на програмата.
NotOnThisPlatform=Тази програма не може да работи под %1.
OnlyOnThisPlatform=Тази програма може да работи само под %1.
WinVersionTooLowError=Тази програма изисква %1 версия %2 или по-нова.
WinVersionTooHighError=Тази програма не може да бъде инсталирана на %1 версия %2 или по-нова.
AdminPrivilegesRequired=За инсталирането на тази програма са необходими администраторски права.
PowerUserPrivilegesRequired=За инсталирането на тази програма трябва да се легитимирате като администратор или потребител с разширени права (power user).
SetupAppRunningError=Инсталаторът е открил, че %1 в момента работи.%n%nМоля, затворете всички стартирани нейни копия и натиснете OK за продължение или Cancel за изход.
UninstallAppRunningError=Деинсталаторът е открил, че %1 в момента работи.%n%nМоля, затворете всички нейни копия и натиснете OK за продължение или Cancel за изход.

; *** Разни грешки
ErrorCreatingDir=Невъзможно е създаването на директория "%1"
ErrorTooManyFilesInDir=Невъзможно е създаването на файл в директория "%1", тъй като тя съдържа прекалено много файлове

; *** Разни съобщения
ExitSetupTitle=Прекъсване на инсталацията
ExitSetupMessage=Инсталацията не е завършена. Ако я прекратите сега, програмата няма да бъде инсталирана.%n%nМожете да стартирате инсталатора по-късно, за да завършите инсталацията.%n%nЖелаете ли прекъсване на инсталацията?
AboutSetupMenuItem=&За инсталатора...
AboutSetupTitle=За инсталатора
AboutSetupMessage=%1 версия %2%n%3%n%nСтраница на %1 в WWW:%n%4
AboutSetupNote=

; *** Бутони
ButtonBack=< &Назад
ButtonNext=На&пред >
ButtonInstall=&Инсталиране
ButtonOK=OK
ButtonCancel=Отказ
ButtonYes=Д&а
ButtonYesToAll=Да (&всички)
ButtonNo=Н&е
ButtonNoToAll=Не (в&сички)
ButtonFinish=&Край
ButtonBrowse=&Избор...

; *** Стандартен текст на помощника
ButtonWizardBrowse=&Избор...
ButtonNewFolder=&Нова папка
SelectLanguageTitle=Избор на език за инсталатора
SelectLanguageLabel=Изберете език за използване при инсталирането:
ClickNext="Напред" - продължение, "Отказ" - изход.
BeveledLabel=

; *** Поздравителна страница на помощника
BrowseDialogTitle=Избор на папка
BrowseDialogLabel=Изберете