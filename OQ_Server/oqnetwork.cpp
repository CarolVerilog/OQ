#include "oqnetwork.h"
#include "oqserver.h"
#include "oqsocket.h"
#include "oqmysqltest.h"

OQNetwork* OQNetwork::sNetwork=nullptr;

OQNetwork *OQNetwork::getNetwork()
{
    if(!sNetwork)
    {
        sNetwork=new OQNetwork();
    }
    
    return sNetwork;
}

void OQNetwork::handleMessage(QMap<QString, QString> msg, OQSocket* socket)
{
    QMap<QString, QString> retMsg;
    
    if(msg.count("register"))
    {
        registerUser(msg["userName"],msg["password"], retMsg);
    }
    else if(msg.count("login"))
    {
        login(msg["id"], msg["password"], retMsg);
    }
    else if(msg.count("sendMessage"))
    {
        sendMessage(msg["senderId"],msg["receiverId"],msg["message"], retMsg);
    }
    else if(msg.count("receiveMessage"))
    {
        receiveMessage(msg["senderId"],msg["receiverId"],QDateTime::fromString(msg["queryTime"], "yyyy-MM-dd hh:mm:ss"), retMsg);
    }
    else if(msg.count("getUserName"))
    {
        getUserName(msg["id"],retMsg);
    }
    else if(msg.count("setSettings"))
    {
        setSettings(msg["id"],msg["name"],msg["gender"],msg["age"],msg["address"],retMsg);
    }
    else if(msg.count("getSettings"))
    {
        getSettings(msg["id"],retMsg);
    }
    
    socket->sendMessage(retMsg);
}

void OQNetwork::registerUser(QStringView userName, QStringView password, QMap<QString, QString>& msg)
{
    OQ_REGISTER_STATE state;
    QString name;
    QString id;

    if(mMySqlTest->regist(userName, password, id))
    {
       msg["id"]=id;
       state=OQ_REGISTER_STATE_SUCCESS;
    }
    else
    {
        state=OQ_REGISTER_STATE_UNKNOWN_ERROR;
    }

    msg["register"]="1";
    msg["state"]=QString::number(state);
}

void OQNetwork::login(QStringView id, QStringView password, QMap<QString, QString>& msg)
{
    OQ_LOGIN_STATE state;
    QString name;

    if(!mMySqlTest->getname(id, name))
    {
        state=OQ_LOGIN_STATE_USER_ID_INVALID;
    }
    if(mMySqlTest->find(id, name, password))
    {
       state=OQ_LOGIN_STATE_SUCCESS;
    }
    else
    {
        state=OQ_LOGIN_STATE_PASSWORD_INVALID;
    }
    
    msg["login"]="1";
    msg["state"]=QString::number(state);
}

void OQNetwork::sendMessage(QStringView senderId, QStringView receiverId, QStringView message, QMap<QString, QString>& msg)
{
    OQ_SEND_MESSAGE_STATE state;
    QString name;


    if(message.length()==0)
    {
        state=OQ_SEND_MESSAGE_STATE_EMPTY_MESSAGE;
    }
    else if(!mMySqlTest->getname(senderId, name) || !mMySqlTest->getname(receiverId, name))
    {
        state=OQ_SEND_MESSAGE_STATE_USER_ID_INVALID;
    }
    else if(mMySqlTest->save(senderId, receiverId,message,QDateTime::currentDateTime()))
    {
        state=OQ_SEND_MESSAGE_STATE_SUCCESS;
    }
    else
    {
        state=OQ_SEND_MESSAGE_STATE_UNKNOWN_ERROR;
    }

    msg["sendMessage"]="1";
    msg["state"]=QString::number(state);
}

void OQNetwork::receiveMessage(QStringView senderId, QStringView receiverId, QDateTime queryTime, QMap<QString, QString>& msg)
{
    OQ_RECEIVE_MESSAGE_STATE state;
    QString name;
    QVector<QDateTime> times;
    QVector<QString> messages;
    QVector<QString> senders;
    QVector<QString> receivers;

    if(!mMySqlTest->getname(senderId, name) || !mMySqlTest->getname(receiverId, name))
    {
        state=OQ_RECEIVE_MESSAGE_STATE_USER_ID_INVALID;
    }
    else if(mMySqlTest->history(senderId, receiverId, queryTime, messages, times, senders, receivers ))
    {
        state=OQ_RECEIVE_MESSAGE_STATE_SUCCESS;

        for(int i=0;i<times.length();++i)
        {
            QString num=QString::number(i);

            msg["message"+num]=messages[i];
            msg["time"+num]=times[i].toString("yyyy-MM-dd hh:mm:ss");
            msg["direction"+num]=QString::number(int(senders[i]==senderId));
        }
    }
    else
    {
        state=OQ_RECEIVE_MESSAGE_STATE_UNKNOWN_ERROR;
    }

    msg["receiveMessage"]="1";
    msg["state"]=QString::number(state);
}


void OQNetwork::getUserName(QStringView id, QMap<QString, QString> &msg)
{
    OQ_GET_USERNAME_STATE state;
    QString name;

    if(mMySqlTest->getname(id, name))
    {
        msg["userName"]=name;
        state=OQ_GET_USERNAME_STATE_SUCCESS;
    }
    else
    {
        state=OQ_GET_USERNAME_STATE_USER_ID_INVALID;
    }

    msg["getUserName"]="1";
    msg["state"]=QString::number(state);
}

void OQNetwork::setSettings(QStringView id, QStringView name, QStringView gender, QStringView age, QStringView address, QMap<QString, QString>& msg)
{
    OQ_SET_SETTINGS_STATE state;

    if(mMySqlTest->update(id, name, gender, age, address))
    {
        state=OQ_SET_SETTINGS_STATE_SUCCESS;
    }
    else
    {
        state=OQ_SET_SETTINGS_STATE_UNKNOWN_ERROR;
    }

    msg["setSettings"]="1";
    msg["state"]=QString::number(state);
}

void OQNetwork::getSettings(QStringView id, QMap<QString, QString>& msg)
{
    OQ_GET_SETTINGS_STATE state;
    QString name;
    QString gender;
    QString age;
    QString address;

    if(mMySqlTest->show(id, name, gender, age, address))
    {
        state=OQ_GET_SETTINGS_STATE_SUCCESS;
        msg["name"]=name;
        msg["gender"]=gender;
        msg["age"]=age;
        msg["address"]=address;
    }
    else
    {
        state=OQ_GET_SETTINGS_STATE_UNKNOWN_ERROR;
    }

    msg["getSettings"]="1";
    msg["state"]=QString::number(state);
}

OQNetwork::OQNetwork(QObject* parent)
    :QObject(parent), mServer(new OQServer(this)), mMySqlTest(new OQMySqlTest(this))
{   
}


