#include "fileListRequestServerSocket.hpp"

#include "mirrorConfig.hpp"
#include "essentialQtso/essentialQt.hpp"

fileListRequestSocket_c::fileListRequestSocket_c(const qintptr socketDescriptor_par_con
        , QObject *parent_par) :
    QSslSocket(parent_par)
{
#ifdef DEBUGJOUVEN
    //QOUT_TS("fileListRequestSocket_c::ctor()" << endl);
#endif
    connect(this, &QTcpSocket::readyRead, this, &fileListRequestSocket_c::readyRead_f);
    connect(this, &QTcpSocket::disconnected, this, &fileListRequestSocket_c::deleteLater);
    //connect(this, &QSslSocket::encrypted, this, &fileListRequestSocket_c::sendFileList_f);
    connect(this, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
    [=](QAbstractSocket::SocketError socketError)
    {
#ifdef DEBUGJOUVEN
        QOUT_TS("(fileListRequestThread_c::run()) address " << this->peerAddress().toString()
                 << " port " << this->peerPort()
                 << " error: " << this->errorString() << endl);
#endif
        if (this->error() != QAbstractSocket::RemoteHostClosedError)
        {
#ifdef DEBUGJOUVEN
            //QOUT_TS("(fileListRequestThread_c::ctor() error) this->deleteLater()" << endl);
#endif
            this->deleteLater();
        }
    });
    connect(this, static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors), [=](const QList<QSslError> &errors)
    {
#ifdef DEBUGJOUVEN
        for (const QSslError& errorItem_ite_con : errors)
        {
            QOUT_TS("(fileListRequestSocket_c::ctor() sslerror) " << errorItem_ite_con.errorString() << endl);
        }
#endif
        //this->deleteLater();
    });

    if(not this->setSocketDescriptor(socketDescriptor_par_con))
    {
        Q_EMIT error(this->error());
    }

    this->startServerEncryption();
}

void fileListRequestSocket_c::readyRead_f()
{
    quint32 updatePort(this->readAll().toULong());
    //QTERROR localhost/loopback ip to toIPv4Address = 0, when it should be 2130706433, toString does a ::1 which is the equivalent of 127.0.0.1 for IPv6
//    quint32 ipV4Tmp(this->peerAddress().toIPv4Address());
//    if (ipV4Tmp == 0)
//    {
//        ipV4Tmp = 2130706433;
//    }
    mirrorConfig_ext.addRecentClient(this->peerAddress(), updatePort);
    //readClientPort_pri = true;
    //
#ifdef DEBUGJOUVEN
    //QOUT_TS("fileListRequestSocket_c::readyRead_f() this->peerAddress() " << this->peerAddress().toString() << endl);
    //QOUT_TS("fileListRequestSocket_c::readyRead_f() this->peerAddress().toIPv4Address() " << this->peerAddress().toIPv4Address() << endl);
    //QOUT_TS("fileListRequestSocket_c::readyRead_f() updatePort " << updatePort << endl);
#endif
    this->write(mirrorConfig_ext.JSONFileListData_f());
    //sendFileList_f();
    //disconnectAfter_f();
    this->disconnectFromHost();
}

//void fileListRequestSocket_c::sendFileList_f()
//{
//#ifdef DEBUGJOUVEN
//    QOUT_TS("fileListRequestSocket_c::sendFileList_f()" << endl);
//#endif
//    this->write(mirrorConfig_ext.JSONFileListData_f());
//#ifdef DEBUGJOUVEN
//    //QOUT_TS("fileListRequestSocket_c::sendFileList_f() mirrorConfig_ext.JSONFileListData_f() " << mirrorConfig_ext.JSONFileListData_f().left(32) << endl);
//#endif
//    //no need because it will be flushed when the server, this server, disconnects
//    //this->flush();
//    fileListSent_pri = true;
//    disconnectAfter_f();
//}

//void fileListRequestSocket_c::disconnectAfter_f()
//{
//#ifdef DEBUGJOUVEN
//    QOUT_TS("fileListRequestSocket_c::disconnectAfter_f()" << endl);
//#endif
//    if (readClientPort_pri and fileListSent_pri)
//    {
//this->disconnectFromHost();
//    }
//}

//void fileListRequestSocket_c::disconnected_f()
//{
//#ifdef DEBUGJOUVEN
//    QOUT_TS("fileListRequestSocket_c::disconnected_f() " << this->socketDescriptor() << " Disconnected" << endl);
//#endif
//    this->deleteLater();
//}
