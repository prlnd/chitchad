#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QListWidgetItem>
#include <string>
#include <map>
#include "chatclient.h"
#include "receiverthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChatWindow; }
QT_END_NAMESPACE

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    ChatWindow(std::string username, const char *hostAdress, QWidget *parent = nullptr);
    ~ChatWindow();

private slots:
    void on_fileButton_clicked();
    void on_sendButton_clicked();
    void insertMessage(std::string_view username, std::string_view message);
    void createFile(std::string_view username, std::string_view filename, std::string_view content);
    void updateUserList(std::string users, std::string channels);
    void insertChannelMessage(std::string_view username, std::string_view channel, std::string_view message);
    void createChannelFile(std::string_view username, std::string_view channel, std::string_view filename, std::string_view content);

    void on_refreshButton_clicked();

    void on_newChannelButton_clicked();

    void on_deleteChannelButton_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_channelListWidget_itemClicked(QListWidgetItem *item);

    void on_addButton_clicked();

    void on_removeButton_clicked();

private:
    std::string getHeader(std::string_view username);
    void sendMessage();

    Ui::ChatWindow *ui;
    ChatClient chatClient;
    ReceiverThread thread;
    std::string username;
    std::map<QString, QString> messages;
    std::map<QString, QString> channels;
    bool isOnUsers = true;
};
#endif // CHATWINDOW_H
