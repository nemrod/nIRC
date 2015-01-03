#include "Tab.h"

Tab::Tab(QString title) {
    this->title = title;
}

Tab::~Tab() {
}

QString Tab::getTitle() {
    return title;
}

void Tab::addMessage(QString message) {
    message.remove("\r\n"); // remove potentially trailing newlines
    messageBox->appendPlainText(message);
    messageBox->ensureCursorVisible(); // automatically scroll to bottom after message is added
}

void Tab::createLayout() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    messageBox = new QPlainTextEdit(this);
    messageBox->setReadOnly(true);
    messageBox->setMaximumBlockCount(500); // 500 message scrollback

    textInput = new QLineEdit(this);
    textInput->setFocus();
    QTimer::singleShot(0, textInput, SLOT(setFocus()));

    connect(textInput, SIGNAL(returnPressed()), this, SLOT(submitInput())); // submit text by pressing enter

    mainLayout->addWidget(messageBox);
    mainLayout->addWidget(textInput);

    setLayout(mainLayout);
}

void Tab::submitInput() {
    QString input = textInput->text();
    textInput->setText("");

    if (!input.isEmpty()) { // don't bother parsing if nothing was entered
        parseInput(input);
    }
}
