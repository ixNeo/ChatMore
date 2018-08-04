#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork>
#include <QtGui>
#include "tcpclientfile.h"
#include "tcpserverfile.h"
#include "privatechat.h"
#include <QTextCharFormat>
#include<QMediaPlayer>

const QString msc_bg ="D:\\chatSmallTerm\\chatMoreDage\\chatMoreFinal\\srcmusic\\cocoon.mp3";
const QString msc_send = "D:\\chatSmallTerm\\chatMoreDage\\chatMoreFinal\\srcmusic\\cuan.mp3";
const QString msc_rcv = "D:\\chatSmallTerm\\chatMoreDage\\chatMoreFinal\\srcmusic\\shou.mp3";

class QUdpSocket;

class TcpServerFile;



namespace Ui {
class ChatRoom;
}

enum MessageType1
{

    Message1,
    NewParticipant1,
    ParticipantLeft1,
    FileName1,
    Refuse1,
    Xchat1
};



class ChatRoom : public QWidget
{
    Q_OBJECT

public:

    /**ï¿½ï¿½ï¿½ï¿½Ë½ï¿½Ä¹ï¿½ï¿½ï¿½*/
    explicit ChatRoom(QWidget *parent = 0);

    ~ChatRoom();
    QString getIP();
    PrivateChat* privatechat;
    PrivateChat* privatechat1;

    QString getUserName();
    QString getMessage();



protected:
    void paintEvent(QPaintEvent *e);
    void changeEvent(QEvent *e);
    void newParticipant(QString userName,
                        QString localHostName, QString ipAddress, int Boo);
    void participantLeft(QString userName,
                         QString localHostName, QString time);
    void sendMessage(MessageType type, QString serverAddress="");



    void hasPendingFile(QString userName, QString serverAddress,
                        QString clientAddress, QString fileName);


    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *target, QEvent *event);//ÊÂ¼þ¹ýÂËÆ÷

    void musicOn();
    void musicOff();

private:
    Ui::ChatRoom *ui;
    QUdpSocket *udpSocket;
    qint16 port;
    qint32 bb;

    QString fileName;
    TcpServerFile *server;

    QColor color;
    bool saveFile(const QString& fileName);
    void showxchat(QString name, QString ip);

    QMediaPlayer *music;
    QMediaPlayer *player;

private slots:

    void on_fontComboBox_currentFontChanged(QFont f);
    void on_sizeComboBox_2_currentIndexChanged(QString );
    void processPendingDatagrams();
    void on_sendButton_2_clicked();
    void on_clearToolBtn_2_clicked();
    void on_saveToolBtn_2_clicked();
    void on_colorToolBtn_2_clicked();
    void on_exitButton_2_clicked();
    void on_italicToolBtn_2_clicked(bool checked);
    void on_boldToolBtn_2_clicked(bool checked);
    void on_sendToolBtn_2_clicked();
    void getFileName(QString);


    void on_underlineToolBtn_2_clicked(bool checked);


    void currentFormatChanged(const QTextCharFormat &format);




    void on_userTableWidget_2_doubleClicked(const QModelIndex &index);
    void on_pushButton_3_clicked();
    void on_sendButton_3_clicked(bool checked);
};

#endif // WIDGET_H
