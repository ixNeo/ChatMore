#include "chatroom.h"
#include "ui_chatroom.h"
#include <QUdpSocket>
#include <QHostInfo>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QNetworkInterface>
#include <QProcess>
#include "tcpserverfile.h"
#include "tcpclientfile.h"
#include <QFileDialog>
//#include <QDebug>
#include "privatechat.h"
#include <QColorDialog>
#include<QApplication>
#include <QDesktopWidget>


ChatRoom::ChatRoom(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatRoom){

    ui->setupUi(this);
    ui->userTableWidget_2->resizeColumnsToContents();
    ui->userTableWidget_2->resizeRowsToContents();
//    ui->userTableWidget_2->setColumnWidth(2,400);
//    ui->userTableWidget_2->setColumnWidth(3,200);
    ui->userTableWidget_2->horizontalHeader()->resizeSection(0,60);
    ui->userTableWidget_2->horizontalHeader()->resizeSection(1,150);
    ui->userTableWidget_2->horizontalHeader()->resizeSection(2,150);
    ui->userTableWidget_2->horizontalHeader()->setStyleSheet("QHeaderView::section{background-color: rgb(147, 161, 200);}");
    for(int i=0;i<3;i++){
        QTableWidgetItem *columnHeaderItem = ui->userTableWidget_2->horizontalHeaderItem(i);
        columnHeaderItem->setFont(QFont("Helvetica"));  //设置字体
        columnHeaderItem->setBackgroundColor(QColor(147, 161, 200));  //设置单元格背景颜色
        columnHeaderItem->setTextColor(QColor(255,255,30));     //设置文字颜色
    }
     ui->userTableWidget_2->insertRow(0);
     ui->userTableWidget_2->setItem(0,0,new QTableWidgetItem("Alice"));
     ui->userTableWidget_2->setItem(0,1,new QTableWidgetItem("Alice"));
     ui->userTableWidget_2->setItem(0,2,new QTableWidgetItem("0.0.0.0"));

    privatechat = NULL;
    bb = 0;
    music = new QMediaPlayer;
    player = new QMediaPlayer;
    connect(music, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));

    ui->messageTextEdit_2->installEventFilter(this);//注册一个监听事件，这次是实现回车快捷键发送

    //创建套接字并进行初始化
    udpSocket = new QUdpSocket(this);
    port = 45456;
    udpSocket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);//QUdpSocket::ShareAddress 绑定模式  其他服务也可以绑定在其上
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    sendMessage(NewParticipant);//调用sendMessage(NewParticipant)放udp广播

    server = new TcpServerFile(this);

    //关联信号 获取文件名 并调用sendmessage函数发送文件
    connect(server, SIGNAL(sendFileName(QString)), this, SLOT(getFileName(QString)));

    connect(ui->messageTextEdit_2, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this, SLOT(currentFormatChanged(const QTextCharFormat)));
}

ChatRoom::~ChatRoom()
{
    delete ui;
}

void ChatRoom::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setColor(QColor(151,160,200));
    pen.setWidth(4);
    QPainterPath painterPath;
    painterPath.addRoundedRect(QRect(QPoint(2,2),QPoint(769,446)),20,20);
    painter.setPen(pen);
    painter.fillPath(painterPath,QColor(228,238,255));
    painter.drawRoundedRect(QRect(QPoint(2,2),QPoint(769,446)),20,20);
    QWidget::paintEvent(e);
}

// 使用UDP广播发送信息 type用于区分数据类型
void ChatRoom::sendMessage(MessageType type, QString serverAddress)
{

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QString localHostName = QHostInfo::localHostName();
    QString address = getIP();
    //qDebug()<< tr("%1").arg(address);
    out << type << getUserName() << localHostName;
//    qDebug()<<type << getUserName() << localHostName<<"sendMessage";
    switch(type)
    {
    case Message :
        if (ui->messageTextEdit_2->toPlainText() == "") {
            QMessageBox::warning(0,tr("Warning!"),tr("Cannot send nothing"),QMessageBox::Ok);
            return;
        }
        out << address << getMessage();
        ui->messageBrowser_2->verticalScrollBar()
                ->setValue(ui->messageBrowser_2->verticalScrollBar()->maximum());
        break;

    case NewParticipant :
        out << address;
        break;

    case ParticipantLeft :
        break;

    case FileName : {
        int row = ui->userTableWidget_2->currentRow();
        QString clientAddress = ui->userTableWidget_2->item(row, 2)->text();
        out << address << clientAddress << fileName;
        break;
    }

    case Refuse :
        out << serverAddress;
        break;
    }
    //调用udpsocket进行udp广播
    udpSocket->writeDatagram(data,data.length(),QHostAddress::Broadcast, port);
}


// 接收UDP广播进行处理
void ChatRoom::processPendingDatagrams()
{
   while(udpSocket->hasPendingDatagrams())
    //if(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        QDataStream in(&datagram, QIODevice::ReadOnly);
        int messageType1;
        in >> messageType1;
        QString userName,localHostName,ipAddress,message;
        QString time = QDateTime::currentDateTime()
                .toString("yyyy-MM-dd hh:mm:ss");


        switch(messageType1)
        {

        case Message1:
            in >> userName >> localHostName >> ipAddress >> message;
            ui->messageBrowser_2->setTextColor(Qt::blue);
            ui->messageBrowser_2->setCurrentFont(QFont("Times New Roman",12));
            ui->messageBrowser_2->append("[ " +localHostName+" ] "+ time);
            ui->messageBrowser_2->append(message);
            if(ipAddress!=getIP())
            {
                player->setMedia(QMediaContent(QUrl::fromLocalFile(msc_rcv)));
                player->play();
            }
            break;

        case NewParticipant1:
            in >>userName >>localHostName >>ipAddress;
            newParticipant(userName,localHostName,ipAddress, 1);
            break;

        case ParticipantLeft1:
            in >>userName >>localHostName;
            participantLeft(userName,localHostName,time);
            break;

        case FileName1: {
            in >> userName >> localHostName >> ipAddress;
            QString clientAddress, fileName;
            in >> clientAddress >> fileName;
            hasPendingFile(userName, ipAddress, clientAddress, fileName);
            break;
        }

        case Refuse1: {
            in >> userName >> localHostName;
            QString serverAddress;
            in >> serverAddress;
            QString ipAddress = getIP();

            if(ipAddress == serverAddress)
            {
                server->refused();
            }
            break;

        }
        case Xchat1:
            {
                in >>userName >>localHostName >>ipAddress;
                showxchat(localHostName,ipAddress);//显示与主机名聊天中，不是用户名
                break;
            }
        }
    }
}


// 处理用户离开 仅仅在用户列表中删除掉该用户的信息
void ChatRoom::participantLeft(QString userName, QString localHostName, QString time)
{
    int rowNum = ui->userTableWidget_2->findItems(localHostName, Qt::MatchExactly).first()->row();
    if(rowNum)
    {
    ui->userTableWidget_2->removeRow(rowNum);
    ui->messageBrowser_2->setTextColor(Qt::yellow);
    ui->messageBrowser_2->setCurrentFont(QFont("Times New Roman", 10));
    ui->messageBrowser_2->append(tr("%1 at %2 leave!").arg(userName).arg(time));
    ui->userNumLabel_2->setText(tr("now online: %1 person(s)").arg(ui->userTableWidget_2->rowCount()));
    }
}

void ChatRoom::on_userTableWidget_2_doubleClicked(const QModelIndex &index)
{
    if(ui->userTableWidget_2->item(index.row(),0)->text() == getUserName() &&
        ui->userTableWidget_2->item(index.row(),2)->text() == getIP())
    {
        QMessageBox::warning(0,"Warning","No one talks with himself!",QMessageBox::Ok);
    }
    else
    {
        if(privatechat){

               PrivateChat * temp = privatechat;
               delete temp;
        }
       privatechat = new PrivateChat(ui->userTableWidget_2->item(index.row(),1)->text(), //接收主机名
                       ui->userTableWidget_2->item(index.row(),2)->text()) ;//接收用户IP
       QByteArray data;
       QDataStream out(&data,QIODevice::WriteOnly);
       QString localHostName = QHostInfo::localHostName();
       QString address = getIP();
       out << Xchat << getUserName() << localHostName << address;
       udpSocket->writeDatagram(data,data.length(),QHostAddress(ui->userTableWidget_2->item(index.row(),2)->text()), port);

       privatechat->setWindowOpacity(1);
       privatechat->setWindowFlags(Qt::FramelessWindowHint);
       privatechat->setWindowIcon(QIcon(":/images/lip"));
       privatechat->setAttribute(Qt::WA_TranslucentBackground);
       privatechat->show();


       privatechat->is_opened = true;
    }

}

// 是否接收文件
void ChatRoom::hasPendingFile(QString userName, QString serverAddress,
                            QString clientAddress, QString fileName)
{
    QString ipAddress = getIP();
    if(ipAddress == clientAddress)
    {
        int btn = QMessageBox::information(this,tr("Receive"),
                                           tr("File from%1(%2) Name:%3,Receive?")
                                           .arg(userName).arg(serverAddress).arg(fileName),
                                           QMessageBox::Yes,QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            QString name = QFileDialog::getSaveFileName(0,tr("Save"),fileName);
            if(!name.isEmpty())
            {
                TcpClientFile *client = new TcpClientFile(this);
                client->setFileName(name);
                client->setHostAddress(QHostAddress(serverAddress));
                client->show();
            }
        } else {
            sendMessage(Refuse, serverAddress);
        }
    }
}

// 传输文件按钮
void ChatRoom::on_sendToolBtn_2_clicked()
{
    if(ui->userTableWidget_2->selectedItems().isEmpty())
    {
        QMessageBox::warning(0, tr("Choose one"),
                             tr("Choose one from the list!"), QMessageBox::Ok);
        return;
    }
    server->show();
    server->initServer();
}


// 处理新用户加
void ChatRoom::newParticipant(QString userName, QString localHostName, QString ipAddress, int Boo = 1)
{
        //去重处理
    qDebug() << localHostName<<ipAddress<<userName<<"newParticipant";
    bool isEmpty = ui->userTableWidget_2->findItems(ipAddress, Qt::MatchExactly).isEmpty();
    if (isEmpty) {
        QTableWidgetItem *user = new QTableWidgetItem(userName);
        QTableWidgetItem *host = new QTableWidgetItem(localHostName);
        QTableWidgetItem *ip = new QTableWidgetItem(ipAddress);

        ui->userTableWidget_2->insertRow(1);
        ui->userTableWidget_2->setItem(1,0,user);
        ui->userTableWidget_2->setItem(1,1,host);
        ui->userTableWidget_2->setItem(1,2,ip);
        ui->messageBrowser_2->setTextColor(Qt::yellow);
        ui->messageBrowser_2->setCurrentFont(QFont("Times New Roman",11));
        ui->messageBrowser_2->append(tr("%1 online!").arg(localHostName));
        ui->userNumLabel_2->setText(tr("now online: %1 person(s)").arg(ui->userTableWidget_2->rowCount()));
        sendMessage(NewParticipant);

     //老用户向新用户发送自己的信息
        if(Boo==1)
        {
                QByteArray data;
                QDataStream out(&data, QIODevice::WriteOnly);
                QString localHostName = QHostInfo::localHostName();
                QString address = getIP();
                int o = 2;
                out << address << getUserName() << localHostName<<o;
                qDebug()<<"laofaxin";
                qDebug()<< address << getUserName() << localHostName<<o;
                udpSocket->writeDatagram(data,data.length(),QHostAddress(ipAddress), port);
        }
    }
}

// 获取ipv4地址
QString ChatRoom::getIP()
{
    QString ipattr = 0;
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();

    foreach(QNetworkInterface interfaceItem, interfaceList)
    {
        if(interfaceItem.flags().testFlag(QNetworkInterface::IsUp)
                &&interfaceItem.flags().testFlag(QNetworkInterface::IsRunning)
                &&interfaceItem.flags().testFlag(QNetworkInterface::CanBroadcast)
                &&interfaceItem.flags().testFlag(QNetworkInterface::CanMulticast)
                &&!interfaceItem.flags().testFlag(QNetworkInterface::IsLoopBack)
//                &&interfaceItem.hardwareAddress()!="00:50:56:C0:00:01"
                &&interfaceItem.hardwareAddress()=="00:50:56:C0:00:08")
        {
            QList<QNetworkAddressEntry> addressEntryList=interfaceItem.addressEntries();
            foreach(QNetworkAddressEntry addressEntryItem, addressEntryList)
            {
                if(addressEntryItem.ip().protocol()==QAbstractSocket::IPv4Protocol)
                {
                    ipattr = addressEntryItem.ip().toString();
                }
            }
        }
    }
    return ipattr;
}

// 获取用户名 使用QProcess 类进行获取
QString ChatRoom::getUserName()
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

// 获得要发送的消息 并清空输入框
QString ChatRoom::getMessage()
{
    QString msg = ui->messageTextEdit_2->toHtml();

    ui->messageTextEdit_2->clear();
    ui->messageTextEdit_2->setFocus();
    return msg;
}


// 发送消息
void ChatRoom::on_sendButton_2_clicked()
{
    sendMessage(Message);
    player->setMedia(QMediaContent(QUrl::fromLocalFile(msc_send)));
    player->play();
}



// 获取要发送的文件名
void ChatRoom::getFileName(QString name)
{
    fileName = name;
    sendMessage(FileName);
}


// 更改字体族
void ChatRoom::on_fontComboBox_currentFontChanged(QFont f)
{
    ui->messageTextEdit_2->setCurrentFont(f);
    ui->messageTextEdit_2->setFocus();
}


// 更改字体大小
void ChatRoom::on_sizeComboBox_2_currentIndexChanged(QString size)
{
    ui->messageTextEdit_2->setFontPointSize(size.toDouble());
    ui->messageTextEdit_2->setFocus();
}

// 加粗
void ChatRoom::on_boldToolBtn_2_clicked(bool checked)
{
    if(checked)
        ui->messageTextEdit_2->setFontWeight(QFont::Bold);
    else
        ui->messageTextEdit_2->setFontWeight(QFont::Normal);
    ui->messageTextEdit_2->setFocus();
}

// 倾斜
void ChatRoom::on_italicToolBtn_2_clicked(bool checked)
{
    ui->messageTextEdit_2->setFontItalic(checked);
    ui->messageTextEdit_2->setFocus();
}

// 下划线
void ChatRoom::on_underlineToolBtn_2_clicked(bool checked)
{
    ui->messageTextEdit_2->setFontUnderline(checked);
    ui->messageTextEdit_2->setFocus();
}

// 颜色
void ChatRoom::on_colorToolBtn_2_clicked()
{
    color = QColorDialog::getColor(color, this);
    if (color.isValid()) {
        ui->messageTextEdit_2->setTextColor(color);
        ui->messageTextEdit_2->setFocus();
    }
}

void ChatRoom::currentFormatChanged(const QTextCharFormat &format)
{
    ui->fontComboBox->setCurrentFont(format.font());

    // 如果字体大小出错(因为我们最小的字体为9)，使用12
    if (format.fontPointSize() < 9) {
        ui->sizeComboBox_2->setCurrentIndex(3);
    } else {
        ui->sizeComboBox_2->setCurrentIndex( ui->sizeComboBox_2
                                          ->findText(QString::number(format.fontPointSize())));
    }
    ui->boldToolBtn_2->setChecked(format.font().bold());
    ui->italicToolBtn_2->setChecked(format.font().italic());
    ui->underlineToolBtn_2->setChecked(format.font().underline());
    color = format.foreground().color();
}

// 保存聊天记录
void ChatRoom::on_saveToolBtn_2_clicked()
{
    if (ui->messageBrowser_2->document()->isEmpty()) {
        QMessageBox::warning(0, tr("Warning"), tr("Nothing to save"), QMessageBox::Ok);
    } else {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save record"), tr("Record"), tr("textfile(*.txt);;All File(*.*)"));
        if(!fileName.isEmpty())
            saveFile(fileName);
    }
}

// 保存聊天记录
bool ChatRoom::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Save"),
                             tr("Save failed %1:\n %2").arg(fileName)
                             .arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out << ui->messageBrowser_2->toPlainText();

    return true;
}

// 清空聊天记录
void ChatRoom::on_clearToolBtn_2_clicked()
{
    ui->messageBrowser_2->clear();
}

// 退出按钮
void ChatRoom::on_exitButton_2_clicked()
{
    close();
}

// 关闭事件
void ChatRoom::closeEvent(QCloseEvent *e)
{
    sendMessage(ParticipantLeft);

}

void ChatRoom::on_pushButton_3_clicked(){}

void ChatRoom::showxchat(QString name, QString ip)
{
    if(!privatechat1)
    privatechat1 = new PrivateChat(name,ip);
    bb++;
}


void ChatRoom::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


bool ChatRoom::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->messageTextEdit_2)
    {
        if(event->type() == QEvent::KeyPress)//回车键
        {
             QKeyEvent *k = static_cast<QKeyEvent *>(event);
             if(k->key() == Qt::Key_Return)
             {
                 on_sendButton_2_clicked();
                 return true;
             }
        }
    }
    return QWidget::eventFilter(target,event);
}

void ChatRoom::musicOn()
{
    music->setMedia(QMediaContent(QUrl::fromLocalFile(msc_bg)));
    music->play();
}

void ChatRoom::musicOff()
{
    music->stop();
}

void ChatRoom::on_sendButton_3_clicked(bool checked)
{
    if(checked == true)
    {
        qDebug()<<"music on!";
        musicOn();
    }
    else
    {
        musicOff();
    }
}
