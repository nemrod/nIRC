#ifndef NIRC_H
#define NIRC_H

#include <QDialog>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QTextStream>
#include <QInputDialog>
#include <QMessageBox>

#include "User.h"
#include "ServerTab.h"
#include "ChannelTab.h"

class nIRC : public QDialog {
    Q_OBJECT

    public:
        nIRC();
        ~nIRC();

    private:
        User *user; // holds basic user config, mainly nick
        QTabWidget *tabWidget; // holds the tabs

        // menu stuff
        QMenuBar *menuBar;
        QMenu *fileMenu;
        QAction *setNickAction;
        QAction *connectAction;
        QAction *quitAction;

        void createMenu();
        void createLayout();

    private slots:
        void closeTab();
        void closeTab(int index);
        void setNick();
        void connectToServer(); // creates server tab
        void joinChannel(QString channel); // creates channel tab
};

#endif
