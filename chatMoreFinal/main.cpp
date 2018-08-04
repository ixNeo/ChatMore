#include <QtGui>
#include <QApplication>
#include "chatroom.h"
#include <QDialog>
//#include <QDebug>
#include "login.h"
#include <QElapsedTimer>
int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    Login app;
    app.setWindowTitle("ChatMore");
    app.setWindowIcon(QIcon(":/images/lip"));
    app.setWindowOpacity(1);
    app.setWindowFlags(Qt::FramelessWindowHint);
    app.show();
    if(app.exec() == QDialog::Accepted)
        {
        ChatRoom *w = new ChatRoom;
        app.close();
        w->setWindowOpacity(1);
        w->setWindowFlags(Qt::FramelessWindowHint);
        w->setWindowIcon(QIcon(":/images/lip"));
        w->setAttribute(Qt::WA_TranslucentBackground);
        w->setObjectName("Widget");
        w->setStyleSheet("QWidget#Widget{border:2px solid green;border-radius:15px}");
        w->show();
        return a.exec();
        }

}
