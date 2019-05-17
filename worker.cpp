#include "worker.h"
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QHostAddress>
#include <QThread>
#include <QStringList>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <windows.h>

Worker::Worker(QObject *parent, QString fname, QString text, QString addr, quint16 port, quint32 delay)
{
    this->name = fname;
    this->ddtext = text;
    this->addr = addr;
    this->port = port;
    this->delay = delay;
    connect(this, SIGNAL(showError(QString)), parent, SLOT(onWarn(QString)));

}

void Worker::run(){
    char buf[1024];
    QMessageBox msgBox;
    // open file
    qDebug() << "thread running!";

    QFile file(name);
    file.open(QIODevice::ReadOnly);
    QByteArray content = file.readAll();
    qDebug() << "Read:" << content.size();

    QTcpSocket s;
    while(!trySend(s, addr, port)){
        QThread::msleep(delay);   //wait delay ms before new try
        if(s.error() == QAbstractSocket::ConnectionRefusedError){
            emit(showError("Connect error, please check server!"));
            //MessageBox(nullptr, L"Connect error, please check server!", L"Error", MB_OK|MB_ICONHAND);
            return;
        }
    }

    qDebug() << "Connected and enabled!";
    QString data = "POST:";
    if(ddtext.contains(':')) ddtext.replace(':', ';');
    data += ddtext + ":";
    data += name.mid(name.lastIndexOf('/')+1) + ":";    // имя без пути
    data += QString::number(content.size()) + "\r\n";   // размер
    s.write(data.toLocal8Bit().data());
    s.flush();
    s.write(content);
    s.flush();
    s.waitForBytesWritten();    // большой объем передается

    s.waitForReadyRead();
    s.readLine(buf, 1024);
    QString request = QString::fromLocal8Bit(buf);
    qDebug() << "rcvd:" << request;

    QStringList part = request.split(':');

    qint32 fsz = part[2].toInt();

    if(part[0] == "POST"){
        uchar *boo = nullptr;
        if(getFile(s, fsz, &boo)){
            qDebug() << "Response received";
            // осталось сохранить в файл, имя прежнее, а вот путь...
            const QString downloadsFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
            QFile wr(downloadsFolder + "/" + part[1]);
            qDebug() << wr.fileName();
            wr.open(QIODevice::WriteOnly);
            wr.write(reinterpret_cast<char *>(boo), fsz);
            wr.close();
            free(boo);
        }
    }
    s.disconnectFromHost();
    file.close();
}

// Выкачвает заданный в sz объем из сокета s в буфер, на который указывает *bb
// при успешной загрузке *bb следует освобождать
bool Worker::getFile(QTcpSocket &s, qint32 sz, uchar **bb){
    uchar *buf = reinterpret_cast<uchar *>(malloc(static_cast<size_t>(sz)));
    qint64 rcvd = 0;
    qint64 por = 0;
    while(rcvd < sz){
        s.waitForReadyRead();
        por = s.read(reinterpret_cast<char *>(buf+rcvd), sz-rcvd);
        rcvd += por;
        qDebug() << "read" << por << "bytes";
        if(!s.isReadable()) break;
    }
    if(rcvd == sz){
        *bb = buf;
        return true;
    }
    free(buf);
    return false;
}

// на вход подается неподключенный сокет, на выходе он подключен к серверу и готов отправлять
// либо отключен если ответ BUSY
bool Worker::trySend(QTcpSocket &so, QString addr, quint16 port){

    char buf[1024];

    so.connectToHost(addr, port, QIODevice::ReadWrite);
    if(so.waitForConnected()){
        qDebug() << "Connected!";
        so.waitForReadyRead();
        if(so.canReadLine()) so.readLine(buf, 1024);
        qDebug() << buf;
        if(strncmp(buf,"READY", 5)==0) return true;
        else {
            so.disconnectFromHost();
            return false;
        }
    }
    return false;
}
