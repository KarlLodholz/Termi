// #include "QConsoleTextEdit.h"

// #include <QApplication>

// int main(int argc, char *argv[]) {
//     QApplication app(argc, argv);

//     QConsoleTextEdit *txt = new QConsoleTextEdit();
//     txt->setText(" This is Termi\n >");
//     txt->show();
//     return app.exec();
// }

#include <QtGui>
#include <QApplication>
#include "QConsole.h"
 
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
 
    QConsole *c = new QConsole();
    c->show();
 
    return a.exec();
}
