#include "QConsole.h"
 
#include <QApplication>
#include <QKeyEvent>
#include <bits/stdc++.h> 
//#include <iostream>
// #include <unistd.h>
 
QConsole::QConsole(QWidget *parent) : QWidget(parent)
{
    resize(650,500);

    //set user info
    char temp[32];
    FILE *file;
    file = popen("whoami", "r");
    fgets(temp, sizeof(temp), file);
    pclose(file);
    std::string name = std::string(temp);
    name.erase(remove(name.begin(), name.end(), '\n'), name.end());
    std::strcpy(temp,name.c_str());
    adrs = std::strcat(temp,":~ $");

    //set console
    console = new QTextEdit();
    console->setCursorWidth(2);
    console->setFontFamily("Courier");
    console->setFontPointSize(13);
    console->setTextColor(QColor(255,255,255));
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
    if(cmd.size()) {
        history.push_back(cmd);
        hist_idx = int(history.size());

        //Variables
        std::string* token_arr;
        std::string s = cmd;
        std::string delimiter = " ";
        size_t pos = 0;
        std::string token;
        std::string temp;
        int command;
        int ctr;
        int parse_ctr;

        //Key
        const char *keyTable[6] = {"ls", "cd", "rm", "mkdir", "quit", "history"};

        //Variables that need to be reset at the top of the loop
        command = -1;
        pos = 0;
        ctr = 1;
        parse_ctr = 0;

        temp = s;

        //Loop to find needed length of array 
        while ((pos = temp.find(delimiter)) != std::string::npos) {
            //std::cout<<"seggi"<<std::endl;
            token = temp.substr(0, pos);
            ctr++;
            temp.erase(0, pos + delimiter.length());
        }

        //std::cout<<"so"<<std::endl;

        //Declare array with the length
        token_arr = new std::string [ctr];
        pos = 0;

        //Assign each index of the array
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            token_arr[parse_ctr] = token;

            //Increment actual parse counter and erase the indexed part of the input
            parse_ctr++;
            s.erase(0, pos + delimiter.length());
        }
        token_arr[parse_ctr] = s;

        //Compare the command to the key table
        for(int i = 0; i < 6; i++) {
            if((token_arr[0] == keyTable[i])) {
                //std::cout << keyTable[i] << std::endl;
                command = i;
            }
        }

        delete [] token_arr;

        //Switch case where execution is done for each command
        //console->append("\n");
        switch(command) {
            case 0: 
                std::cout << "ls" << std::endl;
                console->append("ls"); 
            break;

            case 1:
                std::cout << "cd" << std::endl;
                console->append("cd");
            break;

            case 2:
                std::cout << "rm" << std::endl;
                console->append("rm");
            break;

            case 3:
                std::cout << "mkdir" << std::endl;
                console->append("mkdir");
            break;

            case 4:
                std::cout << "gtfo" << std::endl;
                console->append("gtfo");
                exit(1);
            break;

            case 5:
            std::cout << std::endl;
            for (int i = 0; i < int(history.size()); i++) {
                std::cout << history[i] << std::endl;
            }
            break;
            default:
            console->append("Error, No valid command inputted");
        }
    }
    return;
}
