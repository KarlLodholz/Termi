#include "QConsole.h"
 
#include <QApplication>
#include <QKeyEvent>
#include <bits/stdc++.h> 

#include <iostream>
#include <iomanip>  // setw()
#include <fstream>  // file I/O
#include <vector>
#include <sstream>
#include <iterator>
#include <cstring>
#include <cstdio>

#include <unistd.h>
#include <dirent.h> // linux directory structures

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

// Forward declarations for executing commands
void execute_command(std::vector<std::string> tokens);
void launch_built_in(std::vector<std::string> tokens);
void launch_process(std::vector<std::string> tokens);

const std::vector<std::string> built_ins =
{
    "cd",
    "ls",
    "rm",
    "mkdir",
    "touch",
    "history",
    "quit"
};

std::vector<std::string> history;

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
    // console->setTextColor(QColor(255,255,255));
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

        std::vector<std::string> token_vect;
        std::string curr_token = "";
        // Iterate over every character in cmd
        for (int i = 0; i < cmd.length(); i++)
        {
            // If the character is '\'
            if (cmd[i] == '\\')
            {
                // If there is another character and it is a space
                if ((i < cmd.length() - 1) && (cmd[i + 1] == ' '))
                {
                    // Add a space to the token
                    curr_token += " ";
                    // Skip to the next character
                    i++;
                }
            }
            // Else if the character is a space
            else if (cmd[i] == ' ')
            {
                // This is the end of the token
                token_vect.push_back(curr_token);
                curr_token = "";
            }
            // Else if this is the last character of the string
            else if (i == cmd.length() - 1)
            {
                curr_token += cmd[i];
                token_vect.push_back(curr_token);
                curr_token = "";
            }
            // Else add the character to the current token
            else
            {
                curr_token += cmd[i];
            }
        }

        // Print tokens
        // for (int i = 0; i < token_vect.size(); i++)
        // {
        //     std::cout << token_vect[i] << std::endl;
        // }

        // Process and execute the given command
        execute_command(token_vect);
    }

    return;
}



void execute_command(std::vector<std::string> tokens)
{
    // An empty command was entered.
    if (tokens.size() == 0)
    {
        std::cout << "Empty command entered." << std::endl; 
        return;
    }

    // Check if command is built-in
    for (int i = 0; i < built_ins.size(); i++)
    {
        if (strcmp(tokens[0].c_str(), built_ins[i].c_str()) == 0)
        {
            // First token matches a built-in function, so launch it
            launch_built_in(tokens);

            return;
        }
    }

    std::cout << "starting new process" << std::endl;

    // Else try to launch a process
    launch_process(tokens);

    return;
}

void launch_built_in(std::vector<std::string> tokens)
{
    /*
    BUILT-IN COMMANDS:
    
    "cd",
    "ls",
    "rm",
    "mkdir",
    "touch",
    "history",
    "quit"
    */
    
    // If command is "cd"
    if (tokens[0] == "cd")
    {
        // Expecting exactly 1 argument
        if (tokens.size() == 2)
        {
            // Attempt to cd into the given directory
            if (chdir(tokens[1].c_str()) == 0)
            {
                std::cout << "Successfully changed directory." << std::endl << std::endl;
            }
            else
            {
                std::cout << "Failed to change directory." << std::endl << std::endl;
            }
        }
        else
        {
            // Too many or too few args
            std::cout << "Incorrect number of arguments (expects 1)" << std::endl;
        }
    }

    // Else if command is "ls"
    else if (tokens[0] == "ls")
    {
        // Open directory stream for the current directory
        DIR* dirp = opendir(".");

        // Left justify the output
        std::cout << std::left;

        // Output . and .. first
        std::cout << std::setw(20) << "." << std::setw(5) << "Dir" << std::endl;
        std::cout << std::setw(20) << ".." << std::setw(5) << "Dir" << std::endl;

        // Output every entry in the current directory
        dirent* direntp;
        while ((direntp = readdir(dirp)) != NULL)
        {
            // Don't re-print . and ..
            if ((strcmp(direntp->d_name, ".") != 0) && (strcmp(direntp->d_name, "..") != 0))
            {
                // Left justify the output
                std::cout << std::left;
                // Print entry name and type
                std::cout << std::setw(20) << direntp->d_name << std::setw(5);

                if (direntp->d_type == DT_REG)
                {
                    std::cout << "File";
                }
                else if (direntp->d_type == DT_DIR)
                {
                    std::cout << "Dir";
                }
                else
                {
                    std::cout << "Other";
                }

                // Revert to right justify
                std::cout << std::right << std::endl;
            }
        }

        // Close the directory stream
        closedir(dirp); 
    }

    // Else if command is "rm"
    else if (tokens[0] == "rm")
    {
        // Expects at least 1 argument
        if (tokens.size() >= 2)
        {
            // For each token after the command
            for (int i = 1; i < tokens.size(); i++)
            {
                // Remove the entry with that name if it exists
                if (remove(tokens[i].c_str()) != 0)
                {
                    std::string errMsg = "Error deleting entry \"" + tokens[i] + "\"";
                    perror(errMsg.c_str());
                }
                else
                {
                    std::cout << "Entry successfully deleted" << std::endl;
                }
            }
        }
    }

    // Else if command is "mkdir"
    else if (tokens[0] == "mkdir")
    {
        // Expects at least 1 argument
        if (tokens.size() >= 2)
        {
            // For each token after the command
            for (int i = 1; i < tokens.size(); i++)
            {
                // Temp storage for path
                char buf[4096];
                getcwd(buf, sizeof buf);
                std::string path = buf;
                path += "/" + tokens[i];

                // Make a new directory with the given name
                if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
                {
                    std::cout << "Error making directory \"" << tokens[i] << "\"" << std::endl;
                }
                else
                {
                    std::cout << "Directory successfully created" << std::endl;
                }
            }
        }
    }

    // Else if command is "touch"
    else if (tokens[0] == "touch")
    {
        // Expects at least 1 argument
        if (tokens.size() >= 2)
        {
            // Create a new file for each arguement given
            for (int i = 1; i < tokens.size(); i++)
            {
                // Open new file stream
                std::ofstream newFile;
                newFile.open(tokens[i].c_str());

                // Then immediately close it
                newFile.close();
            }
        }
    }

    // Else if command is "history"
    else if (tokens[0] == "history")
    {
        std::cout << std::endl;
        for (int i = 0; i < int(history.size()); i++) {
            std::cout << history[i] << std::endl;
        }
    }

    // Else if command is "quit"
    else if (tokens[0] == "quit")
    {
        // Exit the shell
        exit(0);
    }
}

void launch_process(std::vector<std::string> tokens)
{
    // Fork and execute the command
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // Copy the contents of the tokens vector into a null-terminated char* array
        char* args[tokens.size() + 1];
        for (int i = 0; i < tokens.size(); i++)
        {
            strcpy(args[i], tokens[i].c_str());
            std::cout << args[i] << std::endl;
        }
        args[tokens.size()] = NULL;

        // Child process
        /* int execvp(const char *file, char *const argv[]); */
        if (execvp(args[0], args) == -1)
        {
            perror("Cannot execute child");
        }               
        // Failed
        exit(1);
    }
    else if (pid < 0)
    {
        // Error forking
        perror("Cannot fork");
    }
    else
    {
        // Parent process waits until child has terminated
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }   
}