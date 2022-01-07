#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QInputDialog>
#include <QDateTime>
#include <QAbstractButton>

ChatWindow::ChatWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    name = QInputDialog::getText(
            this
            , tr("Choose username")
            , tr("Username")
            , QLineEdit::Normal
            , QStringLiteral("User")
        );

    const QString hostAddress = QInputDialog::getText(
            this
            , tr("Choose Server")
            , tr("Server Address")
            , QLineEdit::Normal
            , QStringLiteral("127.0.0.1")
        );
    chatClient = new ChatClient(name.toStdString(), hostAddress.toStdString().c_str());
    thread = new ReceiverThread(chatClient->ClientSocket);
    thread->start();
    connect(thread, &ReceiverThread::getMessage, [this](std::string_view username, std::string_view message) {
        std::string s(1, '<');
        s.append(username.data(), username.size());
        s += "@";
        s += QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "> ";
        s.append(message.data(), message.size());
        s += '\n';
        this->ui->chatTextEdit->insertPlainText(QString::fromStdString(s));
    });
    connect(ui->sendButton, &QAbstractButton::clicked, [this]{
        std::string text = this->ui->messageTextEdit->toPlainText().toStdString();
        //std::string s(1, '<');
        //s += this->name.toStdString() + '@' + QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "> " + text + '\n';
        //this->ui->chatTextEdit->insertPlainText(QString::fromStdString(s));
        this->ui->messageTextEdit->clear();
        chatClient->sendMessage(text);
    });
}

ChatWindow::~ChatWindow()
{
    delete ui;
    delete chatClient;
    delete thread;
}

