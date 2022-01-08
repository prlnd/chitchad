#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <string>
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

private slots:
    void on_pushButton_clicked();

    void on_sendButton_clicked();

private:
    std::string getHeader(std::string_view username);
    void insertMessage(std::string_view username, std::string_view message);
    void createFile(std::string_view username, std::string_view filename, std::string_view content);
    void sendMessage();

    Ui::ChatWindow *ui;
    ChatClient *chatClient;
    ReceiverThread *thread;
    std::string name;
};
#endif // CHATWINDOW_H
