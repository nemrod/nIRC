#include "ServerTab.h"

ServerTab::ServerTab(QString title) : Tab(title) {
    createLayout();
}

ServerTab::~ServerTab() {
}

void ServerTab::establishConnection(QString address, int port, QString nick) {
    this->address = address;
    this->port = port;
    this->nick = nick;

    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

    QNetworkConfigurationManager manager;
    if (true || manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        QNetworkConfiguration config = manager.defaultConfiguration();

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        networkSession->open();
    }
}

void ServerTab::parseInput(QString input) {
    if (input.startsWith("/")) {
        QStringList parts = input.split(" ");
        QString command = parts[0].remove(0, 1);

        if (command == "join") {
            sendRaw("JOIN " + parts[1] + "\r\n");
            emit joinChannel(parts[1]); // tell main UI to create channel tab
        } else if (command == "disconnect") {
            sendRaw("QUIT :\r\n");
        } else {
            addMessage("Unknown command: " + command);
        }
    } else {
        //sendRaw(input + "\r\n"); // XXX: uncomment this and any unknown input in server tab will be sent raw to server
    }
}

void ServerTab::joinedChannel(ChannelTab *channel) {
    channels.append(channel); // add channel tab object to our very own list

    connect(channel, SIGNAL(serverCommand(QString)), this, SLOT(commandFromChannel(QString)));
    connect(channel, SIGNAL(sendMessage(QString)), this, SLOT(sendMessage(QString)));
    connect(channel, SIGNAL(partChannel()), this, SLOT(partChannel()));
}

void ServerTab::commandFromChannel(QString input) {
    parseInput(input); // just forward command input from channel to our own input parsing method
}

void ServerTab::partChannel() { // channel is telling us to part
    ChannelTab *channel = static_cast<ChannelTab*>(sender());
    channels.removeOne(channel); // so remove from our list

    sendRaw("PART " + channel->getTitle() + "\r\n"); // and tell server we're parting
}

void ServerTab::sendRaw(QString data) {
    QByteArray d = data.toUtf8();
    tcpSocket->write(d);

    printf("[S] (%s) %s", title.toStdString().c_str(), data.toStdString().c_str()); // XXX: output sent data to console, debugging purposes
}

void ServerTab::sendMessage(QString message) {
    ChannelTab *channel = static_cast<ChannelTab*>(sender()); // get which channel it is that is sending
    sendMessage(channel->getTitle(), message); // send message to server
    channel->addTimestampedMessage("<" + nick + "> " + message); // and add to channel message list
}

void ServerTab::sendMessage(QString target, QString message) {
    sendRaw("PRIVMSG " + target + " :" + message + "\r\n");
}

void ServerTab::sessionOpened() { // we're connected to the server,
    tcpSocket->connectToHost(address, port);

    sendRaw("USER " + nick + " 0 0 :" + nick + "\r\n"); // to keep connecting we need to send user data and nick
    sendRaw("NICK " + nick + "\r\n");
}


void ServerTab::readData() {
    while (tcpSocket->canReadLine()) { // keep reading until there's no more to read
        QByteArray buffer = tcpSocket->readLine(512); // get a line - max length 512 (see IRC RFC2812#2.3)

        QString data = QString(buffer);
        printf("[R] (%s) %s", title.toStdString().c_str(), data.toStdString().c_str()); // XXX: output received data to console, debugging purposes

        if (data.startsWith("PING")) { // when PINGed, change I to O and send right back at them
            data.replace(1, 1, "O");
            sendRaw(data);
        } else if (data.startsWith("ERROR")) { // ERROR means we've been disconnected (even after gracefully sending QUIT, counterintuitively)
            for (int i = 0; i < channels.size(); i++) { // loop through channels and tell them we're disconnected so they can close
                channels.at(i)->serverHasDisconnected();
            }

            emit closeTab(); // and then tell main UI to close this tab
        } else if (data[0] == ':') { // if the data starts with ':' it's a more complicated IRC message, most things happen here
            data.remove(0, 1); // remove leading colon
            data.remove("\r\n"); // remove trailing newline

            // a lot of ugly parsing by splitting and joining follows; this could be made prettier either by regex or by Qt/C++ getting better methods for this, such as a split method that takes a count argument, like Python amongst others does
            QStringList parts = data.split(" ");

            QString actingNick;
            if (parts[0].contains('!')) { // it's a message instigated by a user, containing a nick (as opposed to a message sent to us by the server)
                actingNick = parts[0].split('!')[0]; // so we extract the nick for later use
            }

            parts.removeFirst(); // remove nick/address part of command, we're done with it

            // the following separates the parts of the command (in a list) from the content sent with the command (in a string)
            QString joinedParts = parts.join(' ');
            QStringList largeParts = joinedParts.split(':');
            QStringList commandParts = largeParts[0].split(' ');
            largeParts.removeFirst();
            QString content = largeParts.join(':');

            QRegularExpression numericReply("^\\d\\d\\d$");
            if (numericReply.match(commandParts[0]).hasMatch()) { // numeric IRC reply from server
                int reply = commandParts[0].toInt();

                switch (reply) {
                    case 366:
                        break;
                    case 353: // NAMES reply, i.e. a list of people in the channel, received when first joining a channel f.e.
                        {
                            QString channel = commandParts[3];
                            content.remove("@"); // strip mode characters
                            content.remove("+"); // ^
                            QStringList nicks = content.split(" ");
                            //nicks.removeOne(nick); // remove ourselves from the list

                            getChannelTabFromTitle(channel)->addNicks(nicks);

                            break;
                        }
                    default:
                        addMessage(data); // if unhandled numeric reply, simply output in server tab
                }
            } else if (commandParts[0] == "PRIVMSG") { // called PRIVMSG in the IRC protocol, but is also for messages in channels since it doesn't differentiate and only has different targets
                QString channel = commandParts[1];
                ChannelTab *channelTab = getChannelTabFromTitle(channel);

                if (channelTab != NULL) {
                    channelTab->addPrivMsg(actingNick, content);
                } else {
                    addMessage(data); // if channel didn't exist, meaning it was truly a private message, display in server tab
                }
            } else if (commandParts[0] == "JOIN") { // someone joined a channel we're in
                if (actingNick != nick) {
                    getChannelTabFromTitle(commandParts[1])->addNick(actingNick);
                }
            } else if (commandParts[0] == "PART") { // someone left a channel we're in
                if (actingNick != nick) {
                    getChannelTabFromTitle(commandParts[1])->removeNick(actingNick);
                }
            } else if (commandParts[0] == "QUIT") { // someone in a channel we're in quit
                if (actingNick != nick) { // after making sure it's not us
                    for (int i = 0; i < channels.size(); i++) { // we go through all channels
                        channels.at(i)->removeNick(actingNick); // to remove the nick from the nicklist, since QUIT doesn't specify what channel(s) we have in common
                    }
                }
            } else if(commandParts[0] == "NICK") { // someone in a channel we're in changed their nick
                for (int i = 0; i < channels.size(); i++) { // again, it doesn't say what channel, so change in all channels that finds the old nick
                    channels.at(i)->changeNick(actingNick, content);
                }
            } else {
                addMessage(data); // if still unhandled, display in server tab
            }
        } else {
            addMessage(data); // if still unhandled, display in server tab
        }
    }
}

ChannelTab* ServerTab::getChannelTabFromTitle(QString channelTitle) {
    for (int i = 0; i < channels.size(); i++) {
        if (channels.at(i)->getTitle() == channelTitle) {
            return channels.at(i);
        }
    }

    return NULL;
}

void ServerTab::displayError(QAbstractSocket::SocketError socketError) {
    printf("[E] (%s) %s", title.toStdString().c_str(), tr("The following error occured: %1.").arg(tcpSocket->errorString()).toStdString().c_str()); // print (network) error message to console
}
