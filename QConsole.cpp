#include "QConsole.h"
 
#include <QApplication>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <bits/stdc++.h> 
#include<iostream>
 
QConsole::QConsole(QWidget *parent) : QWidget(parent)
{
    resize(650,500);
    console = new QTextEdit();
    console->setCursorWidth(2);
    console->setFontFamily("Courier");
    console->setFontPointSize(13);
    console->setTextColor(QColor(255,255,255));
    //console->setTextBackgroundColor(QColor(0,0,0));
    console->setText(adrs);
    console->moveCursor(QTextCursor::End);
    lineStart = (console->textCursor()).position();
    hist_idx = 0;
    temp_hist = "";
    //stops console from taking input directly, so it can be filetered
    console->setFocusPolicy(Qt::NoFocus);
    //layout stuff
    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(console);
    setLayout(mainLayout);
}
 
void QConsole::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Escape: //exits terminal
            exit(1);
        case Qt::Key_Shift:
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Meta: //windows key
            break;// do nothing because if sent to console it seg_faults
        case Qt::Key_Enter:
        case Qt::Key_Return:
            Process(getArg());
            console->append("\n"+adrs);
            console->moveCursor(QTextCursor::End);
            lineStart = (console->textCursor()).position();
            break;
        case Qt::Key_Up:
            if(hist_idx) { 
                if(hist_idx == int(history.size())) temp_hist = getArg();
                else history[hist_idx] = getArg();
                setArg(history[--hist_idx]);
            }
            break;
        case Qt::Key_Down: 
            if(hist_idx < int(history.size())) {
                history[hist_idx++] = getArg();
                //console->setText( QString::fromStdString((getTxt().substr(0,lineStart-1)).append((hist_idx == history.size() ? temp_hist : history[hist_idx]))));    
                setArg((hist_idx == int(history.size()) ? temp_hist : history[hist_idx]));
            }
            break;
        case Qt::Key_Left:
        case Qt::Key_Backspace:
            if(lineStart < (console->textCursor()).position() || 
                ((lineStart <= (console->textCursor()).position() &&
                lineStart < (console->textCursor()).anchor()) && e->key() != Qt::Key_Left) )
                QApplication::sendEvent(console, e);
            break;
        default:
            if(lineStart <= (console->textCursor()).position())
                QApplication::sendEvent(console, e);
            else console->moveCursor(QTextCursor::MoveOperation(lineStart));
            break;
    }
}

std::string QConsole::getArg() {
    return getTxt().substr(lineStart, getTxt().length());
}

std::string QConsole::getTxt() {
    return std::string((console->toPlainText()).toUtf8().constData());
}

void QConsole::setArg(const std::string &arg) {
    console->setText(QString::fromStdString((getTxt().substr(0,lineStart)).append(arg)));
    console->moveCursor(QTextCursor::End);
}

void QConsole::Process(const std::string &cmd) {
    return;
}