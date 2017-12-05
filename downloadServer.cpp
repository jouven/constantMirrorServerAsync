#include "downloadServer.hpp"

#include "downloadServerThread.hpp"
#include "mirrorConfig.hpp"

#include "essentialQtso/essentialQt.hpp"

downloadServer_c::downloadServer_c(
        const QHostAddress &address_par_con
        , const quint16 port_par_con
        , QObject *parent_par)
    : QTcpServer(parent_par)
{
    if (this->listen(address_par_con, port_par_con))
    {
#ifdef DEBUGJOUVEN
        QOUT_TS("(downloadServer_c::downloadServer_c) listen successfull " << endl);
        QOUT_TS("(downloadServer_c::downloadServer_c) address_par_con.toString() " << address_par_con.toString() << endl);
        QOUT_TS("(downloadServer_c::downloadServer_c) port_par_con " << port_par_con << endl);
#endif
    }
    else
    {
        QOUT_TS("Failed to setup download server, address " << address_par_con.toString() << " port " << port_par_con << endl);
    }
}

void downloadServer_c::incomingConnection(qintptr socketDescriptor)
{
    downloadServerThread_c* downloadThreadTmp = new downloadServerThread_c(
                socketDescriptor
                , this
    );

//    connect(downloadThreadTmp, &QThread::finished, []
//    {
//#ifdef DEBUGJOUVEN
//        QOUT_TS("downloadServer_c::incomingConnection() downloadThreadTmp finished" << endl);
//#endif
//    });
    connect(downloadThreadTmp, &QThread::finished, downloadThreadTmp, &QThread::deleteLater);

    downloadThreadTmp->start();
}

