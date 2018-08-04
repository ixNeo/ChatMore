#ifndef CHAT_H
#define CHAT_H

#include <QDialog>
#include <QtNetwork>
#include <QtGui>
#include "tcpclientfile.h"
#include "tcpserverfile.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptValue>
#include<QMediaPlayer>

const QString msc_bg2 ="D:\\chatSmallTerm\\chatMoreDage\\chatMoreFinal\\srcmusic\\cocoon.mp3";
const QString msc_send2 = "D:\\chatSmallTerm\\chatMoreDage\\chatMoreFinal\\srcmusic\\cuan.mp3";
const QString msc_rcv2 = "D:\\chatSmallTerm\\chatMoreDage\\chatMoreFinal\\srcmusic\\shou.mp3";


namespace Ui {
    class PrivateChat;
}

static const QString TuLing_URL = "http://www.tuling123.com/openapi/api";
static const QString TuLing_APIKEY = "86a516ab67a64ed29f28bb683e77e1f8";
//私聊聊天 用于枚举变量 对收到的数据进行处理
enum MessageType
{

    Message,
    NewParticipant,
    ParticipantLeft,
    FileName,
    Refuse,
    Xchat
};

class PrivateChat : public QDialog
{
    Q_OBJECT


public:
    ~PrivateChat();
//	chat();

    //构造函数传入对方的主机名和ip地址
    PrivateChat(QString pasvusername, QString pasvuserip);
    QString xpasvuserip;
    QString xpasvusername;
    QUdpSocket *xchat;
    qint32 xport;
    void sendMessage(MessageType type,QString serverAddress="");
    quint16 a;
//	static  qint32 is_opened = 0;
    bool is_opened;

public slots:


public:
    QString getIP();
protected:

    /**发送文件名信息时用haspendingfile来判断是否要接收该文件 拒收时服务端执行refused函数*/
    void hasPendingFile(QString userName,QString serverAddress,  //接收文件
                                QString clientAddress,QString fileName);
    //处理用户离开
    void participantLeft(QString userName,QString localHostName,QString time);
    bool eventFilter(QObject *target, QEvent *event); //事件过滤器
    void paintEvent(QPaintEvent*);

private:
    Ui::PrivateChat *ui;
    TcpServerFile *server;
    QColor color;//颜色
    bool saveFile(const QString& fileName);//保存聊天记录
    QString getMessage();

    QString getUserName();
    QString message;
    QString fileName;

    QMediaPlayer *player;
private slots:
    void replyFinish(QNetworkReply * reply);
    void sentFileName(QString);
    void on_sendfile_clicked();
    void processPendingDatagrams();
    void on_send_clicked();
    void on_close_clicked();
    void on_clear_clicked();
    void on_save_clicked();
    void on_textcolor_clicked();
    void on_textUnderline_clicked(bool checked);
    void on_textitalic_clicked(bool checked);
    void on_textbold_clicked(bool checked);
    void on_fontComboBox_currentFontChanged(QFont f);
    void on_fontsizecomboBox_currentIndexChanged(QString );
    void currentFormatChanged(const QTextCharFormat &format);

};

#endif // CHAT_H
