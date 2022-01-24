//
// Created by user on 14.01.2022.
//

#include "Updater.h"
#include "Base/Algorithms.h"
#include "cpr/cpr.h"
#include "Base/FileSystem/FileSystem.h"
#include "Base/FileFunctions.h"

int UpdaterCommands(int argc, char* argv[])
{
    if(argc < 2) return ShowError(TResultUpdate::ErrorArguments);
    /*for(auto i = 0; i < argc; i++)
        ShowMessage(TString(argv[i]));*/
    TString command = argv[1];

    TResult res;
    if(command == "-check")
        res = CheckVersion(argc, argv);
    else if(command == "-rollback")
        res = RollbackVersion(argc, argv);
    else if(command == "-test")
    {
        Sleep(5000);
        ShowMessage("Test finished");
    }
    return ShowError(res);
}

int ShowError(const TResult& value)
{
    if(value.IsError())
        ShowMessage(TResult::TextError(value));
    return value.Code();
}

TResult CheckVersion(int argc, char* argv[])
{
    if(argc < 5) return TResultUpdate::ErrorArguments;
    return CheckUpdate(argv[2], argv[3], argv[4]);
}

TResult RollbackVersion(int argc, char* argv[])
{
    if (argc < 3) return TResultUpdate::ErrorArguments;
    return RollbackVersion(argv[2]);
}

TString CheckedPath(const fs::path &p)
{
    if(fs::exists(p.parent_path()) == false)
        fs::create_directories(p.parent_path());
    return p.string();
}

TResult CheckUpdate(const TString curVersion, const TString &rootDir, const TString &url)
{
    double cur = 0.;
    double ser = 0.;

    if(TryStrToDouble(curVersion, cur) == false)//текущая версия не может быть приведена к числу
        return TResultUpdate::ErrorArguments;

    //запрашиваем версию на сервере
    auto res = cpr::Get(cpr::Url(url + "Version.txt"));
    TString serVersion = Trim(res.text);
    //если не ответили или пришло не число говорим об ошибке проверки
    if(res.error || TryStrToDouble(serVersion, ser) == false)
    {
        ShowMessage(res.error.message);
        return TResultUpdate::ErrorCheckUpdate;
    }

    if (ser <= cur)
        return ShowMessage("The latest version is installed.");

    auto update = ShowMessage("New version is available. Download and install it?", true);
    if(update.IsNoError())
    {
        if(ShowMessage("Please save changes and close main application", false, true).IsNoError())
            return InstallUpdate(serVersion, rootDir, url);
    }

    return TResult();
}

TResult InstallUpdate(const TString serVersion, const TString& rootDir, const TString& url)
{
    ShowMessage("Begin update application");
    fs::path root = rootDir;                    //каталог расположения главной программы
    fs::path updateDir = root / "Updates";      //каталог для сохранения обновлений
    fs::path version = serVersion;
    //скачиваем файл со списком файлов обновления
    auto updateList = version / "UpdateList.txt";
    {
        std::ofstream stream(CheckedPath(updateDir / updateList), std::ios::binary);
        auto res = cpr::Download(stream, cpr::Url(url + updateList.string()));
        if (res.error)
            return TResultUpdate::ErrorDownloadList;
    }

    TVecString files;
    auto res = FileLines(files, CheckedPath(updateDir / updateList));//читаем список файлов обновлений
    if(res.IsError()) return res;
    for(const auto& f : files)
    {
        std::ofstream file(CheckedPath(updateDir / version / f), std::ios::binary);
        if(cpr::Download(file, cpr::Url(url + (version / f).string())).error)
            return TResultUpdate::ErrorUpdateFile;
    }
    ShowMessage("Update files downloaded");

    //Делаем резервную копию обновляемых файлов
    fs::copy(updateDir / updateList, CheckedPath(updateDir / "Backup/UpdateList.txt"));
    for(const auto& f : files)
        if(fs::exists(root / f))
            fs::copy(root / f, CheckedPath(updateDir / "Backup" / f));
    ShowMessage("Backup is created");

    //Удаляем заменяемые файлы
    for(const auto& f : files)
        if(fs::exists(root / f))
            fs::remove(root / f);

    //Копируем новую версию заменяемые файлы
    for(const auto& f : files)
        fs::copy(updateDir / version / f, root / f);

    ShowMessage("Application is updated");
    return TResult();
}

TResult RollbackVersion(const TString& rootDir)
{
    if(ShowMessage("Please save changes and close main application", false, true).IsError())
        return TResult();
    fs::path root = rootDir;//каталог расположения главной программы
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
    ShowMessage("Application restored");
    return TResult();
}

TString Translate(const TString &value)
{
    if(value == "Cancel")
        return "Отмена";
    else if(value == "Yes")
        return "Да";
    else if(value == "No")
        return "Нет";
    else if(value == "Update files downloaded")
        return "Файлы обновлений загружены";
    else if(value == "Backup is created")
        return "Файлы восстановления созданы";
    else if(value == "Application is updated")
        return "Приложение обновлено";
    else if(value == "The latest version is installed")
        return "Последняя версия установлена";
    else if(value == "New version is available. Download and install it?")
        return "Доступна новая версия приложения. Скачать и установить ее?";
    else if(value == "Please save changes and close main application")
        return "Пожалуйста сохраните изменения и закройте обновляемое приложение";
    else if(value == "Begin update application")
        return "Началось обновление приложения";
    else if(value == "Application restored")
        return "Обновление приложения отменено";
    return value;
}
