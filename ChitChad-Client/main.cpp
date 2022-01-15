#include "chatwindow.h"

#include <QApplication>
#include <QInputDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    bool ok;
    QString name = QInputDialog::getText(
                nullptr
                , "Choose a username"
                , "Username"
                , QLineEdit::Normal
                , QStringLiteral("User")
                , &ok
                );
    if (!ok) return 1;
    QString hostAddress = QInputDialog::getText(
                nullptr
                , "Choose Server"
                , "Server Address"
                , QLineEdit::Normal
                , QStringLiteral("127.0.0.1")
                , &ok
                );
    if (!ok) return 1;
    ChatWindow w(name.toStdString(), hostAddress.toLatin1().data());
    w.show();
    return a.exec();
}
