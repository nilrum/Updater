#include <iostream>
#include "Base/Algorithms.h"
#include "Base/Result.h"
#include "cpr/cpr.h"
#include "Base/FileSystem/FileSystem.h"
#include "Base/FileFunctions.h"
/* Программа обновления приложения
Commands:
    -check VERSION URL
        Проверить есть ли новая версия:
            VERSION - текущая версия приложения
            URL - путь до файла Version.txt (без названия файла)
    -update ROOT URL
        Обновится до новой версии приложения:
            ROOT - каталог размещения приложения
            URL - путь до файла Version.txt (без названия файла)
    -rollback ROOT
        Откатится на версию до обновления:
            ROOT - каталог размещения приложения
*/

int ShowError(const TResult& value);
TResult CheckVersion(int argc, char* argv[]);
TResult UpdateVersion(int argc, char* argv[]);
TResult RollbackVersion(int argc, char* argv[]);

ENUM_CLASS(TResultUpdate, Ok, ErrorArguments, ErrorCheckUpdate, ErrorDownloadList, ErrorUpdateFile, ErrorBackups)

int main(int argc, char* argv[])
{
    if(argc < 2) return ShowError(TResultUpdate::ErrorArguments);

    TString command = argv[1];

    TResult res;
    if(command == "-check")
        res = CheckVersion(argc, argv);
    else if(command == "-update")
        res = UpdateVersion(argc, argv);
    else if(command == "-rollback")
        res = RollbackVersion(argc, argv);

    return ShowError(res);
}

int ShowError(const TResult& value)
{
    if(value.IsError())
        std::cout << TResult::TextError(value) << std::endl;

    return value.Code();
}

template<typename T>
TResult GetVersion(const TString& host, T& version)
{
    auto res = cpr::Get(cpr::Url(host + "Version.txt"));
    double value = 0.;
    if(res.error || TryStrToDouble(Trim(res.text), value) == false)
        return TResultUpdate::ErrorCheckUpdate;
    if constexpr(std::is_same_v<T, double>)
        version = value;
    else
        version = Trim(res.text);
    return TResultUpdate::Ok;
}

TResult CheckVersion(int argc, char* argv[])
{
    if(argc < 4) return TResultUpdate::ErrorArguments;

    double currentVersion = 0.;
    double serverVersion = 0.;

    if(TryStrToDouble(argv[2], currentVersion) == false)
        return TResultUpdate::ErrorArguments;

    TString host = argv[3];

    auto res = GetVersion(argv[3], serverVersion);
    if(res.IsError()) return res;

    if (serverVersion > currentVersion)
        std::cout << "New version is available." << std::endl;
    else
        std::cout << "The latest version is installed." << std::endl;
    return TResultUpdate::Ok;
}


TString CheckedPath(const fs::path &p)
{
    if(fs::exists(p.parent_path()) == false)
        fs::create_directories(p.parent_path());
    return p.string();
}

TResult UpdateVersion(int argc, char* argv[])
{
    if(argc < 4) return TResultUpdate::ErrorArguments;

    TString host = argv[3];
    fs::path version;

    auto res = GetVersion(host, version);
    if(res.IsError()) return res;

    fs::path root = argv[2];                    //каталог расположения главной программы
    fs::path updateDir = root / "Updates";      //каталог для сохранения обновлений

    //скачиваем файл со списком файлов обновления
    auto updateList = version / "UpdateList.txt";
    {
        std::ofstream stream(CheckedPath(updateDir / updateList), std::ios::binary);
        auto res = cpr::Download(stream, cpr::Url(host + updateList.string()));
        if (res.error)
            return TResultUpdate::ErrorDownloadList;
    }

    TVecString files;
    res = FileLines(files, CheckedPath(updateDir / updateList));//читаем список файлов обновлений
    if(res.IsError()) return res;
    for(const auto& f : files)
    {
        std::ofstream file(CheckedPath(updateDir / version / f), std::ios::binary);
        if(cpr::Download(file, cpr::Url(host + (version / f).string())).error)
            return TResultUpdate::ErrorUpdateFile;
    }
    std::cout << "Update files downloaded" << std::endl;

    //Делаем резервную копию обновляемых файлов
    fs::copy(updateDir / updateList, CheckedPath(updateDir / "Backup/UpdateList.txt"));
    for(const auto& f : files)
        if(fs::exists(root / f))
            fs::copy(root / f, CheckedPath(updateDir / "Backup" / f));
    std::cout << "Backup is created" << std::endl;

    //Удаляем заменяемые файлы
    for(const auto& f : files)
        if(fs::exists(root / f))
            fs::remove(root / f);

    //Копируем новую версию заменяемые файлы
    for(const auto& f : files)
        fs::copy(updateDir / version / f, root / f);

    std::cout << "Application is updated" << std::endl;

    return TResult();
}

TResult RollbackVersion(int argc, char* argv[])
{
    if(argc < 3) return TResultUpdate::ErrorArguments;

    fs::path root = argv[2];//каталог расположения главной программы
    fs::path backupDir = root / "Updates/Backup";
    auto updateList = backupDir / "UpdateList.txt";
    if(fs::exists(updateList) == false) return TResultUpdate::ErrorBackups;

    TVecString files;
    auto res = FileLines(files, updateList.string());//читаем список файлов обновлений
    if(res.IsError()) return res;
    for(const auto& f : files)
    {
        if(fs::exists(backupDir / f))
        {
            fs::remove(root / f);//удалим если есть этот файл
            fs::copy(backupDir / f, CheckedPath(root / f));
        }
    }
    std::cout << "Application restored" << std::endl;
    return TResult();
}