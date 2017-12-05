#include "updateClient.hpp"

#include "mirrorConfig.hpp"
#include "essentialQtso/essentialQt.hpp"

bool updateClient_c::success_f() const
{
    return success_pri;
}

updateClient_c::updateClient_c(
        const QHostAddress& address_par_con
        , const quint16 port_par_con
        , QObject *parent)
    : QSslSocket(parent)
    , address_pri(address_par_con)
    , port_pri(port_par_con)
{
    connect(this, &QSslSocket::encrypted, this, &updateClient_c::successfulConnection_f);
    //connect(this, &QTcpSocket::readyRead, this, &downloadClient_c::newRead_f);
    //connect(this, &QTcpSocket::disconnected, this, &downloadClient_c::finishFile_f);
    connect(this,  static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), [=](QAbstractSocket::SocketError socketError)
    {
#ifdef DEBUGJOUVEN
        QOUT_TS("(updateClient_c::ctor() error) address " << this->peerAddress().toString()
                 << " port " << this->peerPort()
                 << " error: " << this->errorString() << endl);
#endif
        if (this->error() != QAbstractSocket::RemoteHostClosedError)
        {
#ifdef DEBUGJOUVEN
            //QOUT_TS("(updateClient_c::ctor() error) this->deleteLater()" << endl);
#endif
            this->deleteLater();
        }
    });
    connect(this, static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors), [=](const QList<QSslError> &errors)
    {
#ifdef DEBUGJOUVEN
        for (const QSslError& errorItem_ite_con : errors)
        {
            QOUT_TS("(updateClient_c::ctor() sslerror) " << errorItem_ite_con.errorString() << endl);
        }
#endif
        //this->deleteLater();
    });
#ifdef DEBUGJOUVEN
    //QOUT_TS("(updateClient_c::ctor() error) address_par_con.toString() " << address_par_con.toString() << endl);
    //QOUT_TS("(updateClient_c::ctor() error) port_par_con " << port_par_con << endl);
#endif
    this->connectToHostEncrypted(address_par_con.toString(), port_par_con);
}

void updateClient_c::successfulConnection_f()
{
#ifdef DEBUGJOUVEN
    //QOUT_TS("(updateClient_c::successfulConnection_f) client connected" << endl);
#endif
//    if (this->state() == QAbstractSocket::ConnectedState)
//    {
//#ifdef DEBUGJOUVEN
//        QOUT_TS("(updateClient_c::successfulConnection_f) client connected state" << endl);
//        //qDebug() << "this->peerAddress()" << this->peerAddress() << endl;
//        //qDebug() << "this->peerPort()" << this->peerPort() << endl;
//#endif
        QByteArray byteArrayTmp;
        byteArrayTmp.append(QString::number(mirrorConfig_ext.fileListServerPort_f()));
        success_pri = this->write(byteArrayTmp.data(), byteArrayTmp.size()) > 0;
        this->disconnectFromHost();
//    }
//    else
//    {
//#ifdef DEBUGJOUVEN
//        QOUT_TS("(updateClient_c::successfulConnection_f) but state is not connected" << endl);
//#endif
//    }
}
