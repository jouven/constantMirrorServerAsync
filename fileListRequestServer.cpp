#include "fileListRequestServer.hpp"

#include "mirrorConfig.hpp"
#include "fileListRequestServerThread.hpp"

#include "essentialQtso/essentialQt.hpp"

#include <QTcpSocket>

fileListRequestServer_c::fileListRequestServer_c(
        const QHostAddress &address_par_con
        , const quint16 port_par_con
        , QObject *parent_par)
    : QTcpServer(parent_par)
{
    if (this->listen(address_par_con, port_par_con))
    {
#ifdef DEBUGJOUVEN
        QOUT_TS("(fileListRequestServer_c::fileListRequestServer_c) listen successfull" << endl);
        QOUT_TS("(fileListRequestServer_c::fileListRequestServer_c) address_par_con.toString() " << address_par_con.toString() << endl);
        QOUT_TS("(fileListRequestServer_c::fileListRequestServer_c) port_par_con " << port_par_con << endl);
#endif
    }
    else
    {
        QOUT_TS("Failed to setup file list server, address " << address_par_con.toString() << " port " << port_par_con << endl);
    }
}

void fileListRequestServer_c::incomingConnection(qintptr socketDescriptor)
{
    fileListRequestServerThread_c* fileRequestThreadTmp = new fileListRequestServerThread_c(
                socketDescriptor
                , this
    );
    connect(fileRequestThreadTmp, &QThread::finished, fileRequestThreadTmp, &QThread::deleteLater);
    fileRequestThreadTmp->start();
}

