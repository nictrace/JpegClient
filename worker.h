#ifndef WORKER_H
#define WORKER_H

#include <QRunnable>
#include <QString>
#include <QTcpSocket>

class Worker : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent, QString fname, QString text, QString addr = "127.0.0.1", quint16 port = 8888, quint32 delay = 100);


protected:
    void run();
    QString ddtext;
private:
    bool getFile(QTcpSocket &s, qint32 sz, uchar **bb);
    bool trySend(QTcpSocket &so, QString addr, quint16 port);
    QString name;
    quint16 port;
    QString addr;
    quint32 delay;

signals:
    void showError(QString data);

};

#endif // WORKER_H
