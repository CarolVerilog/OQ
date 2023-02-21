#ifndef OQNETWORK_H
#define OQNETWORK_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringView>
#include <memory>

class OQSocket;
class OQServer;
class mysqltest;

enum OQ_REGISTER_STATE
{
    OQ_REGISTER_STATE_SUCCESS,
    OQ_REGISTER_STATE_USER_EXIST,
    OQ_REGISTER_STATE_NETWORK_ERROR,
    OQ_REGISTER_STATE_UNKNOWN_ERROR
};

enum OQ_LOGIN_STATE
{
    OQ_LOGIN_STATE_SUCCESS,
    OQ_LOGIN_STATE_USER_ID_INVALID,
    OQ_LOGIN_STATE_PASSWORD_INVALID,
    OQ_LOGIN_STATE_NETWORK_ERROR,
    OQ_LOGIN_STATE_UNKNOWN_ERROR
};

enum OQ_SEND_MESSAGE_STATE
{
    OQ_SEND_MESSAGE_STATE_SUCCESS,
    OQ_SEND_MESSAGE_STATE_USER_ID_INVALID,
    OQ_SEND_MESSAGE_STATE_EMPTY_MESSAGE,
    OQ_SEND_MESSAGE_STATE_NETWORK_ERROR,
    OQ_SEND_MESSAGE_STATE_UNKNOWN_ERROR
};

enum OQ_RECEIVE_MESSAGE_STATE
{
    OQ_RECEIVE_MESSAGE_STATE_SUCCESS,
    OQ_RECEIVE_MESSAGE_STATE_USER_ID_INVALID,
    OQ_RECEIVE_MESSAGE_STATE_NO_NEW_MESSAGE,
    OQ_RECEIVE_MESSAGE_STATE_NETWORK_ERROR,
    OQ_RECEIVE_MESSAGE_STATE_UNKNOWN_ERROR
};

class OQNetwork : public QObject
{
    Q_OBJECT
public:
    static OQNetwork* getNetwork();
public slots:
    void handleMessage(QMap<QString, QString> msg, OQSocket* socket);
public:
    void registerUser(QStringView id, QStringView userName, QStringView password, QMap<QString, QString>& msg);
    void login(QStringView id, QStringView password, QMap<QString, QString>& msg);
    void sendMessage(QStringView senderId, QStringView receiverId, QStringView message, QMap<QString, QString>& msg);
    void receiveMessage(QString senderId, QStringView receiverId, QMap<QString, QString>& msg);
private:
    OQNetwork();
	static OQNetwork* sNetwork;
    OQServer* mServer;
    std::unique_ptr<mysqltest> mMySqlTest;
};

#endif // OQNETWORK_H