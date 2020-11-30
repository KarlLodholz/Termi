#ifndef QCONSOLE_H
#define QCONSOLE_H
 
#include <QWidget>
#include <QString>
#include <QtGui>
#include <QTextEdit>
#include <QVBoxLayout>
 
class QConsole : public QWidget
{
    Q_OBJECT
public:
    QConsole(QWidget *parent = 0);
    QString getAddress();
    std::string getArg();
    std::string getTxt();
    void setArg(const std::string &arg);

protected:
    void keyPressEvent(QKeyEvent *);
    //void keyReleaseEvent(QKeyEvent *);
 
private:
    void Process(const std::string &cmd);
    void launch_process(std::vector<std::string> tokens);
    void execute_command(std::vector<std::string> tokens);
    void launch_built_in(std::vector<std::string> tokens);

    QTextEdit *console;
    int lineStart;
    QVBoxLayout *mainLayout;
    std::vector<std::string> history;
    std::string temp_hist;
    int hist_idx;
    QString address;
    std::string user;
};
 
#endif