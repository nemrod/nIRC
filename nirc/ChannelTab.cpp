#include "ChannelTab.h"

ChannelTab::ChannelTab(QString title) : Tab(title) {
    createLayout();
}

ChannelTab::~ChannelTab() {
}

void ChannelTab::createLayout() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    messageBox = new QPlainTextEdit(this);
    messageBox->setReadOnly(true);
    messageBox->setMaximumBlockCount(500);

    nickList = new QListWidget(this);
    nickList->setSortingEnabled(true);
    nickList->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
    nickList->setMaximumWidth(150);

    textInput = new QLineEdit(this);
    textInput->setFocus();
    QTimer::singleShot(0, textInput, SLOT(setFocus()));

    connect(textInput, SIGNAL(returnPressed()), this, SLOT(submitInput()));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(messageBox);
    layout->addWidget(nickList);
    mainLayout->addLayout(layout);

    mainLayout->addWidget(textInput);

    setLayout(mainLayout);
}

void ChannelTab::parseInput(QString input) {
    if (input.startsWith("/")) { // check if it's a command input
        QStringList parts = input.split(" ");
        QString command = parts[0].remove(0, 1);

        if (command == "part") { // /part is used to leave the channel
            emit partChannel(); // so we tell the server to send part message
            emit closeTab(); // and the main UI to close this tab
        } else {
            emit serverCommand(input); // if unknown command, elevate to server
        }
    } else {
        emit sendMessage(input); // if not a command, send as regular message to channel
    }
}

void ChannelTab::serverHasDisconnected() { // if server has been disconnected
    emit closeTab(); // we tell main UI to close this tab as well
}

void ChannelTab::addNicks(QStringList nicks) {
    for (int i = 0; i < nicks.size(); i++) {
        new QListWidgetItem(nicks.at(i), nickList);
    }
}

void ChannelTab::addNick(QString nick) {
    new QListWidgetItem(nick, nickList);
    addJoinMsg(nick);
}

void ChannelTab::removeNick(QString nick) {
    for (int i = 0; i < nickList->count(); i++) {
        if (nickList->item(i)->text() == nick) {
            delete nickList->item(i);
            addPartMsg(nick);
            break;
        }
    }
}

void ChannelTab::changeNick(QString oldNick, QString newNick) {
    for (int i = 0; i < nickList->count(); i++) {
        if (nickList->item(i)->text() == oldNick) {
            delete nickList->item(i);
            new QListWidgetItem(newNick, nickList);
            addNickChangeMsg(oldNick, newNick);
            break;
        }
    }
}

void ChannelTab::addTimestampedMessage(QString message) {
    QTime time = QTime::currentTime();
    QString timestamp = time.toString("HH:mm:ss");

    addMessage(timestamp + " " + message);
}

void ChannelTab::addPrivMsg(QString nick, QString message) {
    addTimestampedMessage("<" + nick + "> " + message);
}

void ChannelTab::addJoinMsg(QString nick) {
    addTimestampedMessage("* " + nick + " joined the channel");
}

void ChannelTab::addPartMsg(QString nick) {
    addTimestampedMessage("* " + nick + " left the channel");
}

void ChannelTab::addNickChangeMsg(QString oldNick, QString newNick) {
    addTimestampedMessage("* " + oldNick + " is now known as " + newNick);
}
