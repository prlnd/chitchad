#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QInputDialog>
#include <QDateTime>
#include <QAbstractButton>
#include <QFileDialog>
#include <fstream>

ChatWindow::ChatWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    name = QInputDialog::getText(
            this
            , tr("Choose a username")
            , tr("Username")
            , QLineEdit::Normal
            , QStringLiteral("User")
        ).toStdString();
    auto hostAddress = QInputDialog::getText(
            this
            , tr("Choose Server")
            , tr("Server Address")
            , QLineEdit::Normal
            , QStringLiteral("127.0.0.1")
        ).toStdString();
    chatClient = new ChatClient(name, hostAddress.c_str());
    thread = new ReceiverThread(chatClient->ClientSocket);
    thread->start();

    connect(thread, &ReceiverThread::getMessage, this, &ChatWindow::insertMessage);
    connect(thread, &ReceiverThread::getFile, this, &ChatWindow::createFile);
}

ChatWindow::~ChatWindow()
{
    delete ui;
    delete chatClient;
    delete thread;
}

void ChatWindow::insertMessage(std::string_view username, std::string_view message)
{
    auto s = getHeader(username);
    s.append(message.data(), message.size());
    s += '\n';
    ui->chatTextEdit->insertPlainText(QString::fromStdString(s));
}

void ChatWindow::createFile(std::string_view username, std::string_view filename, std::string_view content)
{
    std::ofstream ofs(filename.data(), std::ifstream::binary);
    if (!ofs)
        qDebug() << "Error occured while opening file " << filename.data();

    ofs.write(content.data(), content.size());
    auto s = getHeader(username);
    s += "[File ";
    s.append(filename.data(), filename.size());
    s += " sent]";
    insertMessage(username, s);
}

void ChatWindow::on_sendButton_clicked()
{
    std::string text = ui->messageTextEdit->toPlainText().toStdString();
    ui->messageTextEdit->clear();
    chatClient->sendMessage(text);
}

std::string ChatWindow::getHeader(std::string_view username)
{
    std::string s(1, '<');
    s.append(username.data(), username.size());
    s += "@";
    s += QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "> ";
    return s;
}

void ChatWindow::on_pushButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Select a file");
    chatClient->sendFile(path.toStdString());
}
