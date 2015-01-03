#ifndef SERVERTAB_H
#define SERVERTAB_H

#include <QTcpSocket>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QMessageBox>

#include "Tab.h"
#include "ChannelTab.h"

class ServerTab : public Tab {
    Q_OBJECT

    public:
        ServerTab(QString title);
        virtual ~ServerTab();

        void establishConnection(QString address, int port, QString nick);
        void sendMessage(QString target, QString message); // target is a channel
        void joinedChannel(ChannelTab *channel); // called when a channel tab has been created

    private:
        QString address;
        int port;
        QString nick;

        QTcpSocket *tcpSocket;
        QNetworkSession *networkSession;

        QList<ChannelTab*> channels;

        virtual void parseInput(QString input);
        void sendRaw(QString data); // send raw data to the server
        ChannelTab* getChannelTabFromTitle(QString channelTitle); // get the channel object from the channel name we receive from the server

    private slots:
        void sessionOpened(); // called when network connection is established
        void readData(); // called when we have incoming data
        void displayError(QAbstractSocket::SocketError socketError);
        void commandFromChannel(QString input); // receives commands from channel tabs
        void partChannel(); // sends part message to server for calling channel tab
        void sendMessage(QString message); // sends a text message to the channel associated with calling channel tab

    signals:
        void joinChannel(QString channel); // tells main UI to create a channel tab
};

#endif
