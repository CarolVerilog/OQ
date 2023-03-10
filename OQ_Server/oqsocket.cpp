#include "oqsocket.h"
#include "oqnetwork.h"
#include <QDataStream>
#include <QByteArray>
#include <QThread>

OQSocket::OQSocket(QObject* parent)
    :QTcpSocket(parent)
{
    connect(this, &OQSocket::readyRead, this, &OQSocket::receiveMessage, Qt::DirectConnection);
}

void OQSocket::sendMessage(const QMap<QString, QString>& msg)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);

    out<<(quint16)0;

    for(const QString& key : msg.keys())
    {
        QString tempMsg=key+":"+msg[key];
        out<<tempMsg;
    }

    quint16 size=block.length()-2;
    block[0]=0xff&size>>8;
    block[1]=0xff&size;
    write(block);
}

void OQSocket::receiveMessage()
{
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_12);

    // test whether the data size is accessible
    if(mBufSize==0)
    {
        if(bytesAvailable()<(int)sizeof(quint16))
        {
            return;
        }

        in>>mBufSize;
    }

    // test whether all the data has been received
    quint16 initBytes=bytesAvailable();
    if(initBytes<mBufSize)
    {
        return;
    }

    // read json message
    QMap<QString, QString> msg;
    quint16 msgSize=0;

    while(msgSize<mBufSize)
    {
        QString tempMsg;
        in>>tempMsg;

        int splitorIdx=tempMsg.indexOf(":");
        QString key=tempMsg.mid(0, splitorIdx);
        QString value=tempMsg.mid(splitorIdx+1,tempMsg.length()-splitorIdx-1);
        msg[key]=value;

        msgSize=initBytes-bytesAvailable();
    }

    emit(handleMessage(msg, this));

    mBufSize=0;

    if(bytesAvailable()>0)
    {
        receiveMessage();
    }
}

OQSocketThread::OQSocketThread(qintptr socketDesc, QObject *parent)
    :QThread(parent)
{
    mSocket=new OQSocket(this);
    mSocket->setSocketDescriptor(socketDesc);
    connect(mSocket, &OQSocket::handleMessage, OQNetwork::getNetwork(), &OQNetwork::handleMessage);
}

OQSocket *OQSocketThread::getSocket()
{
    return mSocket;
}

void OQSocketThread::run()
{
}
