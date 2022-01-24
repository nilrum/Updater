//
// Created by user on 14.01.2022.
//

#include "InteractiveQt.h"
#include <QMessageBox>
#include <QListWidget>
#include <QTimer>

TResult ShowMessage(const TString& value, bool isQuestion, bool isInfo)
{
    TString v = Translate(value);
    if(isQuestion)
    {
        QMessageBox box(QMessageBox::Question, "Updater", STR(v), QMessageBox::Yes | QMessageBox::No, nullptr);
        auto y = Translate("Yes");
        auto n = Translate("No");
        box.setButtonText(QMessageBox::Yes, STR(y));
        box.setButtonText(QMessageBox::No, STR(n));
        if(box.exec() == QMessageBox::No)
            return TResult::Cancel;
    }
    else
    {
        if(isInfo == false)
        {
            auto w = Single<QListWidget*>();
            if(w->isVisible() == false)
                w->showNormal();
            w->addItem(STR(v));
        }
        else
        {
            QMessageBox box(QMessageBox::Information, "Updater", STR(v), QMessageBox::Ok | QMessageBox::Cancel, nullptr);
            auto c = Translate("Cancel");
            box.setButtonText(QMessageBox::Cancel, STR(c));

            if(box.exec() == QMessageBox::Cancel)
                return TResult::Cancel;
        }
    }
    return TResult();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv, true);

    auto wid = new QListWidget();
    wid->setWindowFlags(Qt::Dialog);
    wid->setWindowTitle("Updater");
    Single<QListWidget*>() = wid;
    //запускаем по таймеру, после запуска окна
    QTimer::singleShot(50, [argc, argv, wid](){ UpdaterCommands(argc, argv); });
    QApplication::exec();
    return 0;
}