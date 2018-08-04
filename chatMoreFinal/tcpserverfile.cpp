#include "tcpserverfile.h"
#include "ui_tcpserverfile.h"

#include <QFile>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

TcpServerFile::TcpServerFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TcpServerFile)
{
    ui->setupUi(this);

    setFixedSize(350,180);

    //创建一个tcp变量
    tcpPort = 6666;
    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(sendMessage()));

    //初始化服务器
    initServer();
}

TcpServerFile::~TcpServerFile()
{
    delete ui;
}

// 初始化
void TcpServerFile::initServer()
{
    //对缓冲区和发送字节进行初始化
    payloadSize = 64*1024;
    TotalBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;

    ui->serverStatusLabel->setText(tr("Choose a file to start!"));
    ui->progressBar->reset();
    ui->serverOpenBtn->setEnabled(true);
    ui->serverSendBtn->setEnabled(false);

    tcpServer->close();
}

// 开始发送数据
void TcpServerFile::sendMessage()
{
    ui->serverSendBtn->setEnabled(false);
    clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(bytesWritten(qint64)),
            this, SLOT(updateClientProgress(qint64)));

    ui->serverStatusLabel->setText(tr("Transmission %1!").arg(theFileName));

    localFile = new QFile(fileName);
    if(!localFile->open((QFile::ReadOnly))){
        QMessageBox::warning(this, tr("Application"), tr("Reach File Failed %1:\n%2")
                             .arg(fileName).arg(localFile->errorString()));
        return;
    }
    TotalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_7);
    time.start();  // 开始计时
    QString currentFile = fileName.right(fileName.size()
                                         - fileName.lastIndexOf('/')-1);
    sendOut << qint64(0) << qint64(0) << currentFile;
    TotalBytes += outBlock.size();
    sendOut.device()->seek(0);
    sendOut << TotalBytes << qint64((outBlock.size() - sizeof(qint64)*2));
    bytesToWrite = TotalBytes - clientConnection->write(outBlock);
    outBlock.resize(0);
}

// 更新进度条
void TcpServerFile::updateClientProgress(qint64 numBytes)
{
    //用于在传输大文件时窗口不会冻结 timer不采用耗时的等待操作 而是每一次溢出之后对进度条进行一次更新
    qApp->processEvents();
    bytesWritten += (int)numBytes;
    if (bytesToWrite > 0) {
        outBlock = localFile->read(qMin(bytesToWrite, payloadSize));
        bytesToWrite -= (int)clientConnection->write(outBlock);
        outBlock.resize(0);
    } else {
        localFile->close();
    }
    ui->progressBar->setMaximum(TotalBytes);
    ui->progressBar->setValue(bytesWritten);

    float useTime = time.elapsed();
    double speed = bytesWritten / useTime;
    ui->serverStatusLabel->setText(tr("Done %1MB (%2MB/s) "
                                      "\Total%3MB Time used:%4s\nTime left: %5s")
                   .arg(bytesWritten / (1024*1024))
                   .arg(speed*1000 / (1024*1024), 0, 'f', 2)
                   .arg(TotalBytes / (1024 * 1024))
                   .arg(useTime/1000, 0, 'f', 0)
                   .arg(TotalBytes/speed/1000 - useTime/1000, 0, 'f', 0));

    if(bytesWritten == TotalBytes) {
        localFile->close();
        tcpServer->close();
        ui->serverStatusLabel->setText(tr("FileReceived %1").arg(theFileName));
    }
}

// 打开按钮 弹出一个对话框选择文件
void TcpServerFile::on_serverOpenBtn_clicked()
{
    fileName = QFileDialog::getOpenFileName(this);
    if(!fileName.isEmpty())
    {
        theFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);
        ui->serverStatusLabel->setText(tr("Slected:%1 ").arg(theFileName));
        ui->serverSendBtn->setEnabled(true);
        ui->serverOpenBtn->setEnabled(false);
    }
}

// 发送按钮 点击发送按钮之后   使用udp广播将文件信息发送给客户端
//客户端收到广播之后弹出一个对话框 同意之后新建一个客户端 服务端使用tcp进行文件传输
void TcpServerFile::on_serverSendBtn_clicked()
{
    if(!tcpServer->listen(QHostAddress::Any,tcpPort))//开始监听
    {
        qDebug() << tcpServer->errorString();
        close();
        return;
    }

    ui->serverStatusLabel->setText(tr("Waiting for the response......"));
    //传递信号
    emit sendFileName(theFileName);
}

// 关闭按钮
void TcpServerFile::on_serverCloseBtn_clicked()
{
    if(tcpServer->isListening())
    {
        tcpServer->close();
        if (localFile->isOpen())
            localFile->close();
        clientConnection->abort();
    }
    close();
}

// 被对方拒绝 客户端调用tcpClient->abort()后 服务器端关闭当前的tcp连接
void TcpServerFile::refused()
{
    tcpServer->close();
    ui->serverStatusLabel->setText(tr("Be refused"));
}

// 关闭事件
void TcpServerFile::closeEvent(QCloseEvent *)
{
    on_serverCloseBtn_clicked();
}
