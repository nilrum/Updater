//
// Created by user on 14.01.2022.
//

#ifndef ATLAS_UPDATER_H
#define ATLAS_UPDATER_H

#include "Base/Result.h"

/* Программа обновления приложения
Commands:
    -check VERSION ROOT URL
        Проверить и при необходимости обновится до новой версии приложения:
            VERSION - текущая версия приложения
            ROOT - каталог размещения приложения
            URL - путь до файла Version.txt (без названия файла)
    -rollback ROOT
        Откатится на версию до обновления:
            ROOT - каталог размещения приложения
    -test
        Проверка ожидания
*/

int UpdaterCommands(int argc, char* argv[]);

int ShowError(const TResult& value);
TResult ShowMessage(const TString& value, bool isQuestion = false, bool isInfo = false);
TString Translate(const TString& value);
inline auto Trans(const TString& value){ return STR(Translate(value)); }

TResult CheckVersion(int argc, char* argv[]);
TResult RollbackVersion(int argc, char* argv[]);

TResult CheckUpdate(const TString curVersion, const TString& rootDir, const TString& url);
TResult InstallUpdate(const TString serVersion, const TString& rootDir, const TString& url);

TResult RollbackVersion(const TString& rootDir);

ENUM_CLASS(TResultUpdate, Ok, ErrorArguments, ErrorCheckUpdate, ErrorDownloadList, ErrorUpdateFile, ErrorBackups)

#endif //ATLAS_UPDATER_H
