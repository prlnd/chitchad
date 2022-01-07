#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include "chatclient.h"
#include "receiverthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChatWindow; }
QT_END_NAMESPACE

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

private:
    Ui::ChatWindow *ui;
    ChatClient *chatClient;
    ReceiverThread *thread;
    QString name;
};
#endif // CHATWINDOW_H
