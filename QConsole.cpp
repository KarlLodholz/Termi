#include "QConsole.h"
 
#include <QApplication>
#include <QKeyEvent>
#include <bits/stdc++.h> 
//#include <iostream>
// #include <unistd.h>


// Linux file system headers
#include <unistd.h>
#include <dirent.h>	// linux directory structures

// Async headers
#include <sys/types.h>
#include <sys/wait.h>

const std::vector<std::string> built_ins = {
	"ls",
	"cd",
	"rm",
	"mkdir",
	"rmdir",
	"touch",
	"exit"
};


QConsole::QConsole(QWidget *parent) : QWidget(parent)
{
    resize(650,500);

    //set user info
    char temp[32];
    FILE *file;
    file = popen("whoami", "r");
    
    
    user = std::string(fgets(temp, sizeof(temp), file));
    pclose(file);
    user.erase(remove(user.begin(), user.end(), '\n'), user.end());

    std::vector<std::string> home{"cd","/home/"+user+"/"};

    launch_built_in(home);

    //set console
    console = new QTextEdit();
    console->setCursorWidth(2);
    console->setFontFamily("Courier");
    console->setFontPointSize(13);
    console->setTextColor(QColor(255,255,255));
    console->setText( getAddress());
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
            console->append("\n"+getAddress());
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

QString QConsole::getAddress() {
    char loc[48];
    FILE *file;
    file = popen("pwd", "r");
    std::string temp = std::string(fgets(loc, sizeof(loc), file));
    pclose(file);
    temp.erase(remove(temp.begin(), temp.end(), '\n'), temp.end());
    std::strcpy(loc,user.c_str());
    std::strcat(loc,":");
    std::strcat(loc,temp.c_str());
    std::strcat(loc,"$ ");
    return loc;
}

void QConsole::Process(const std::string &cmd) {
    if(cmd.size()) {
        history.push_back(cmd);
        hist_idx = int(history.size());

        //Variables
        std::vector<std::string> token_vec;
        std::string s = cmd;
        std::string delimiter = " ";
        size_t pos = 0;
        std::string token;
        std::string temp;
        int ctr;
        int parse_ctr;

        //Variables that need to be reset at the top of the loop
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

        pos = 0;

        //Assign each index of the array
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            token_vec.push_back(token);

            //Increment actual parse counter and erase the indexed part of the input
            parse_ctr++;
            s.erase(0, pos + delimiter.length());
        }
        token_vec.push_back(s);
        
        execute_command(token_vec);
    }
    return;
}

void QConsole::launch_built_in(std::vector<std::string> tokens)
{
	/*
	BULIT-IN COMMANDS:
	
	"cd",
	"ls",
	"rm",
	"mkdir",
	"rmdir",
	"touch",
	"exit"
	
	*/
	
	// If command is "cd"
	if (tokens[0] == "cd")
	{
		// Expecting at least 1 argument
		if (tokens.size() >= 2)
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
	}

	// Else if command is "ls"
	else if (tokens[0] == "ls")
	{
		// Open directory stream for the current directory
		DIR* dirp = opendir(".");

		// Output every entry in the current directory
		dirent* direntp;
		while ((direntp = readdir(dirp)) != NULL)
		{
			// Left justify
			// std::cout << (std::string(std::left));
            console->append(QString::fromStdString(std::string( direntp->d_name )));
			std::cout << std::setw(20) << direntp->d_name << std::setw(5);
			if (direntp->d_type == DT_REG)
			{
				std::cout << "file";
			}
			else if (direntp->d_type == DT_DIR)
			{
				std::cout << "dir";
			}
			else
			{
				std::cout << "other";
			}

			// Revert to right justify
			std::cout << std::right << std::endl;
		}

		// Close the directory stream
		closedir(dirp);	
	}

	// Else if command is "touch"
	else if (tokens[0] == "touch")
	{
		// Expects exactly 1 argument
		if (tokens.size() == 2)
		{
			// Open new file stream
			std::ofstream newFile;
			newFile.open(tokens[1].c_str());

			// Then immediately close it
			newFile.close();
		}
	}

	// Else if command is "exit"
	else if (tokens[0] == "exit")
	{
		// Expects no arguments
		if (tokens.size() == 1)
		{
			// Exit the shell
			exit(0);
		}
	}
}


void QConsole::launch_process(std::vector<std::string> tokens)
{
	// Fork and execute the command
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0)
	{
		// Copy the contents of the tokens vector into a null-terminated char* array
		char* args[tokens.size() + 1];
		for (int i = 0; i < int(tokens.size()); i++)
		{
			strcpy(args[i], tokens[i].c_str());
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
			waitpid(pid, &status, WUNTRACED);
		}
		while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}	
}

void QConsole::execute_command(std::vector<std::string> tokens)
{
	// An empty command was entered.
	if (tokens.size() == 0)
	{
		std::cout << "Empty command entered." << std::endl; 
		return;
	}

	// Check if command is built-in
	for (int i = 0; i < int(built_ins.size()); i++)
	{
		if (strcmp(tokens[0].c_str(), built_ins[i].c_str()) == 0)
		{
			// First token matches a built-in function, so launch it
			launch_built_in(tokens);

			return;
		}
	}

	// Else try to launch a process
	launch_process(tokens);

	return;
}