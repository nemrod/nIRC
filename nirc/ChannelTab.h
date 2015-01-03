#ifndef CHANNELTAB_H
#define CHANNELTAB_H

#include <QTcpSocket>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QMessageBox>
#include <QTime>

#include "Tab.h"

class ChannelTab : public Tab {
    Q_OBJECT

    public:
        ChannelTab(QString title);
        virtual ~ChannelTab();

        virtual void parseInput(QString input);
        void serverHasDisconnected(); // called when server disconnects, giving us a chance to close channel tab gracefully

        // methods for adding, removing and changing nicks
        void addNicks(QStringList nicks);
        void addNick(QString nick);
        void removeNick(QString nick);
        void changeNick(QString oldNick, QString newNick);

        // methods to add specific types of messages to tab
        void addTimestampedMessage(QString message);
        void addPrivMsg(QString nick, QString message);
        void addJoinMsg(QString nick);
        void addPartMsg(QString nick);
        void addNickChangeMsg(QString oldNick, QString newNick);

    private:
        QListWidget *nickList;

        virtual void createLayout();
        void updateNickListWidth();

    signals:
        void serverCommand(QString input); // forwards unknown command input to server
        void sendMessage(QString message);
        void partChannel();
};

#endif
