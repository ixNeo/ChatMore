#include "login.h"
#include "ui_login.h"
#include <QDateTime>
#include "chatroom.h"
#include <QWidget>
#include <Qtgui>
#include <QMovie>
#include <QHostInfo>
#include <QNetworkInterface>
#include <chatroom.h>
#include<QMessageBox>
#include<QProcess>
Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    ui->label_2->setText("当前时间: "+gettime());
    ui->pushButton->setStyleSheet("QPushButton{border-image: url(:/images/denglu2.png);}"
                                  "QPushButton:hover{border-image: url(:/images/denglu1.png);}"
                                  "QPushButton:pressed{border-image: url(:/images/denglu1.png);}");
    ui->pushButton_2->setStyleSheet("QPushButton{border-image: url(:/images/tuichu2.png);}"
                                  "QPushButton:hover{border-image: url(:/images/tuichu1.png);}"
                                  "QPushButton:pressed{border-image: url(:/images/tuichu1.png);}");
    ui->label_3->setText("欢迎来到异次元");
    ui->userlineEdit->setText(getUserName());
}

Login::~Login()
{
    delete ui;
}

void Login::resizeEvent(QResizeEvent* e)
{
    QBitmap bmp(size());
    bmp.fill();
    QPainter p(&bmp);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(bmp.rect(), 60, 60); //四个角都是圆弧
    //只要上边角圆弧
    int arcR = 30; //圆弧大小
    QRect rect = this->rect().adjusted(0, 0, 0, 0);
    QPainterPath path;
    //逆时针
    path.moveTo(arcR, 0);
    //左上角
    path.arcTo(0, 0, arcR * 2, arcR * 2, 90.0f, 90.0f);
    path.lineTo(0, rect.height()-arcR);
    //左下角
    path.arcTo(0, rect.height() - arcR * 2 , arcR * 2, arcR * 2, 180.0f, 90.0f);
    path.lineTo(rect.width(), rect.height());
    //右下角
    path.arcTo(rect.width() - arcR * 2, rect.height() - arcR * 2 , arcR * 2, arcR * 2, 270.0f, 90.0f);
    path.lineTo(rect.width(), arcR);
    //右上角
    path.arcTo(rect.width() - arcR * 2, 0, arcR * 2, arcR * 2, 0.0f, 90.0f);
    path.lineTo(arcR, 0);
    p.drawPath(path);
    //此行代码必须添加，不然无法达到正常的显示
    p.fillPath(path, QBrush(Qt::red));
    setMask(bmp);

    return QDialog::resizeEvent(e);
}

QString Login::gettime(){

    QDateTime dt;
    QTime time;
    QDate date;
    dt.setTime(time.currentTime());
    dt.setDate(date.currentDate());
    QString currentDate = dt.toString("hh : mm");
   return currentDate;
}


void Login::on_pushButton_clicked()
{
    if(ui->pwdlineEdit->text() == tr("123456"))
        { accept();}
    else{
        QMessageBox::warning(NULL,tr("warning!"),tr("please input correct password"),QMessageBox::Yes);
            ui->pwdlineEdit->clear();
            ui->pwdlineEdit->setFocus();
        }
}




void Login::on_pushButton_2_clicked()
{
    close();
}

QString Login::getUserName()
{
    QStringList envVariables;
    envVariables << "USERNAME.*" << "USER.*" << "USERDOMAIN.*"
                 << "HOSTNAME.*" << "DOMAINNAME.*";
    QStringList environment = QProcess::systemEnvironment();
    foreach (QString string, envVariables) {
        int index = environment.indexOf(QRegExp(string));
        if (index != -1) {
            QStringList stringList = environment.at(index).split('=');
            if (stringList.size() == 2) {
                return stringList.at(1);
                break;
            }
        }
    }
    return "unknown";
}
