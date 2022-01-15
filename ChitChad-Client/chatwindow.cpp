#include "chatwindow.h"
#include "ui_chatwindow.h"
#include "chadutils.h"
#include <QDateTime>
#include <QAbstractButton>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <fstream>

ChatWindow::ChatWindow(std::string username, const char *hostAdress, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWindow)
    , chatClient(username, hostAdress)
    , thread(chatClient.clientSocket)
    , username(std::move(username))
{
    ui->setupUi(this);
    setWindowTitle("ChitChad (" + QString::fromStdString(this->username) + ')');
    thread.start();
    connect(&thread, &ReceiverThread::getMessage, this, &ChatWindow::insertMessage);
    connect(&thread, &ReceiverThread::getFile, this, &ChatWindow::createFile);
    connect(&thread, &ReceiverThread::getUserList, this, &ChatWindow::updateUserList);
    connect(&thread, &ReceiverThread::getChannelMessage, this, &ChatWindow::insertChannelMessage);
    connect(&thread, &ReceiverThread::getChannelFile, this, &ChatWindow::createChannelFile);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::insertMessage(std::string_view username, std::string_view message)
{
    auto s = getHeader(username);
    s.append(message.data(), message.size());
    auto qstr = QString::fromStdString(s);
    auto qname = chad::stringViewToQString(username);
    messages[qname] += qstr + '\n';
    if (ui->listWidget->currentItem()->text() == qname) {
        qDebug() << "Appended:" << qstr;
        ui->chatTextEdit->append(qstr);
    }
}

void ChatWindow::createFile(std::string_view username, std::string_view filename, std::string_view content)
{
    std::ofstream ofs(std::move(std::string(filename)), std::ifstream::binary);
    if (!ofs)
        qDebug() << "Error occured while opening file " << filename.data();

    ofs.write(content.data(), content.size());
    std::string s("[File ");
    s.append(filename.data(), filename.size());
    s += " sent]";
    insertMessage(username, s);
}

void ChatWindow::updateUserList(std::string users, std::string channels)
{
    qDebug() << "Updating user list" << chad::stringViewToQString(users);
    ui->listWidget->blockSignals(true);
    ui->listWidget->clear();
    for (decltype(users.size()) i = 0; i < users.size(); i += users[i] + 1) {
        auto substr = users.substr(i + 1, users[i]);
        ui->listWidget->addItem(QString::fromLatin1(substr.data(), substr.size()));
    }
    ui->listWidget->blockSignals(false);
    ui->listWidget->setCurrentRow(0);

    qDebug() << "Updating channel list" << chad::stringViewToQString(channels);
    ui->channelListWidget->blockSignals(true);
    ui->channelListWidget->clear();
    for (decltype(channels.size()) i = 0; i < channels.size(); i += channels[i] + 1) {
        auto substr = channels.substr(i + 1, channels[i]);
        ui->channelListWidget->addItem(QString::fromLatin1(substr.data(), substr.size()));
    }
    ui->channelListWidget->blockSignals(false);
    ui->channelListWidget->setCurrentRow(0);
}

void ChatWindow::insertChannelMessage(std::string_view username, std::string_view channel, std::string_view message)
{
    auto s = getHeader(username);
    s.append(message.data(), message.size());
    auto qstr = QString::fromStdString(s);
    auto qname = chad::stringViewToQString(channel);
    channels[qname] += qstr + '\n';
    if (ui->channelListWidget->currentItem()->text() == qname) {
        qDebug() << "Appended:" << qstr;
        ui->chatTextEdit->append(qstr);
    }
}

void ChatWindow::createChannelFile(std::string_view username, std::string_view channel, std::string_view filename, std::string_view content)
{
    std::ofstream ofs(std::move(std::string(filename)), std::ifstream::binary);
    if (!ofs)
        qDebug() << "Error occured while opening file " << filename.data();

    ofs.write(content.data(), content.size());
    std::string s("[File ");
    s.append(filename.data(), filename.size());
    s += " sent]";
    insertChannelMessage(username, channel, s);
}

void ChatWindow::on_sendButton_clicked()
{
    std::string text = ui->messageTextEdit->toPlainText().toStdString();
    ui->messageTextEdit->clear();
    if (isOnUsers) {
        auto qname = ui->listWidget->currentItem()->text();
        chatClient.sendMessage(qname.toStdString(), text);
        auto s = getHeader(username);
        s += text;
        auto qstr = QString::fromStdString(s);
        messages[qname] += qstr + '\n';
        if (ui->listWidget->currentItem()->text() == qname) {
            qDebug() << "Appended:" << qstr;
            ui->chatTextEdit->append(qstr);
        }
    } else {
        auto qname = ui->channelListWidget->currentItem()->text();
        chatClient.sendChannelMessage(qname.toStdString(), text);
        auto s = getHeader(username);
        s += text;
        auto qstr = QString::fromStdString(s);
        channels[qname] += qstr + '\n';
        if (ui->channelListWidget->currentItem()->text() == qname) {
            qDebug() << "Appended:" << qstr;
            ui->chatTextEdit->append(qstr);
        }
    }
}

std::string ChatWindow::getHeader(std::string_view username)
{
    std::string s(1, '<');
    s.append(username.data(), username.size());
    s += "@";
    s += QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "> ";
    return s;
}

void ChatWindow::on_fileButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Select a file");
    if (isOnUsers)
        chatClient.sendFile(ui->listWidget->currentItem()->text().toStdString(), path.toStdString());
    else
        chatClient.sendChannelFile(ui->channelListWidget->currentItem()->text().toStdString(), path.toStdString());
}

void ChatWindow::on_refreshButton_clicked()
{
    chatClient.getUsers();
}


void ChatWindow::on_newChannelButton_clicked()
{
    bool ok;
    QString name = QInputDialog::getText(
                nullptr
                , "Add channel name"
                , "Channel"
                , QLineEdit::Normal
                , QStringLiteral("New Channel")
                , &ok
                );
    if (ok)
        chatClient.addChannel(name.toStdString());
}


void ChatWindow::on_deleteChannelButton_clicked()
{
    chatClient.removeChannel(ui->channelListWidget->currentItem()->text().toStdString());
}


void ChatWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    isOnUsers = true;
    ui->chatTextEdit->setText(messages[item->text()]);
}


void ChatWindow::on_channelListWidget_itemClicked(QListWidgetItem *item)
{
    isOnUsers = false;
    ui->chatTextEdit->setText(channels[item->text()]);
}


void ChatWindow::on_addButton_clicked()
{
    auto channel = ui->channelListWidget->currentItem()->text().toStdString();
    auto user = ui->listWidget->currentItem()->text().toStdString();
    chatClient.addUser(channel, user);
}


void ChatWindow::on_removeButton_clicked()
{
    auto channel = ui->channelListWidget->currentItem()->text().toStdString();
    auto user = ui->listWidget->currentItem()->text().toStdString();
    chatClient.removeUser(channel, user);
}

