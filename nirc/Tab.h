#ifndef TAB_H
#define TAB_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTimer>
#include <QScrollBar>

class Tab : public QWidget {
    Q_OBJECT

    public:
        Tab(QString title);
        virtual ~Tab();

        QString getTitle();
        void addMessage(QString message);

    protected:
        QString title;
        QPlainTextEdit *messageBox;
        QLineEdit *textInput;

        virtual void createLayout();

    private:
        virtual void parseInput(QString input) = 0; // make sure inheriting tab classes implement this

    private slots:
        void submitInput();

    signals:
        void closeTab();
};

#endif
