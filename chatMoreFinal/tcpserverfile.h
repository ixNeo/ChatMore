#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QDialog>

#include <QTime>
class QFile;
class QTcpServer;
class QTcpSocket;

namespace Ui {
class TcpServerFile;
}

class TcpServerFile : public QDialog
{
    Q_OBJECT

public:
    explicit TcpServerFile(QWidget *parent = 0);
    ~TcpServerFile();

    void initServer();
    void refused();

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::TcpServerFile *ui;

    qint16 tcpPort;
    QTcpServer *tcpServer;
    QString fileName;
    QString theFileName;
    QFile *localFile;

    qint64 TotalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 payloadSize;
    QByteArray outBlock;

    QTcpSocket *clientConnection;

    QTime time;

private slots:
    void sendMessage();
    void updateClientProgress(qint64 numBytes);

    void on_serverOpenBtn_clicked();

    void on_serverSendBtn_clicked();

    void on_serverCloseBtn_clicked();

signals:
    void sendFileName(QString fileName);
};

#endif // TCPSERVER_H
