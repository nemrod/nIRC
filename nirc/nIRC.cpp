#include "nIRC.h"

nIRC::nIRC() {
    createMenu();
    createLayout();

    user = new User();
}

nIRC::~nIRC() {
    delete user;
}

void nIRC::createMenu() {
    menuBar = new QMenuBar(this);

    fileMenu = new QMenu(tr("&File"), this);

    setNickAction = fileMenu->addAction(tr("Set &Nick"));
    connectAction = fileMenu->addAction(tr("&Connect To Server"));
    fileMenu->addSeparator();
    quitAction = fileMenu->addAction(tr("&Quit"));

    quitAction->setShortcut(tr("CTRL+Q"));

    menuBar->addMenu(fileMenu);

    connect(setNickAction, SIGNAL(triggered()), this, SLOT(setNick()));
    connect(connectAction, SIGNAL(triggered()), this, SLOT(connectToServer()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(accept()));
}

void nIRC::createLayout() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMenuBar(menuBar);

    tabWidget = new QTabWidget(this);
    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    mainLayout->addWidget(tabWidget);

    setLayout(mainLayout);
}

void nIRC::closeTab() { // closes current tab
    emit closeTab(tabWidget->currentIndex());
}

void nIRC::closeTab(int index) {
    tabWidget->removeTab(index);
}

void nIRC::setNick() {
    QString oldNick = QString::fromStdString(user->getNick().c_str());

    bool ok;
    QString newNick = QInputDialog::getText(this, tr("Set Nick"), tr("Enter new nick:"), QLineEdit::Normal, oldNick, &ok);

    QMessageBox msgBox;
    if (ok && user->setNick(newNick.toStdString())) { // if we got input and we could set the nick (setNick checks nick validity)
        msgBox.setText("Nick has been changed.");
    } else {
        msgBox.setText("Nick couldn't be changed.");
    }
    msgBox.exec();
}

void nIRC::connectToServer() {
    QMessageBox msgBox;
    bool ok;
    QString address;
    int port = 6667;

    QString addressInput = QInputDialog::getText(this, tr("Connect To Server"), tr("Enter server to connect to (address:port):"), QLineEdit::Normal, "irc.freenode.net:6667", &ok);

    if (ok) {
        QStringList splitAddress = addressInput.split(":");
        if (splitAddress.size() == 2) { // ensure we have an address:port pair
            address = splitAddress.at(0);
            port = splitAddress.at(1).toInt(&ok);
        } else {
            ok = false;
        }
    }

    if (ok) { // if we got input, it was an address:port pair, and we could cast port as int
        int index = tabWidget->addTab(new ServerTab(addressInput), addressInput); // create server tab

        tabWidget->setCurrentIndex(index); // switch to the new tab

        ServerTab *tab = static_cast<ServerTab*>(tabWidget->widget(index)); // get server tab object
        tab->establishConnection(address, port, QString::fromStdString(user->getNick())); // tell the tab to establish a connection to the server

        connect(tab, SIGNAL(closeTab()), this, SLOT(closeTab()));
        connect(tab, SIGNAL(joinChannel(QString)), this, SLOT(joinChannel(QString)));
    } else {
        msgBox.setText("Didn't get proper address input.");
        msgBox.exec();
    }
}

void nIRC::joinChannel(QString channel) {
    int index = tabWidget->addTab(new ChannelTab(channel), channel); // create channel tab

    tabWidget->setCurrentIndex(index); // switch to new tab

    ChannelTab *tab = static_cast<ChannelTab*>(tabWidget->widget(index)); // get channel tab object
    ServerTab *server = static_cast<ServerTab*>(sender()); // get the channel's associated server tab
    connect(tab, SIGNAL(closeTab()), this, SLOT(closeTab()));

    server->joinedChannel(tab); // tell the server that we've created the channel tab
}
