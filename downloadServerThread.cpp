#include "downloadServerThread.hpp"

#include "downloadServerSocket.hpp"

#include "essentialQtso/essentialQt.hpp"

downloadServerThread_c::downloadServerThread_c(const qintptr socketDescriptor_par_con
        , QObject *parent_par) :
    threadedFunction_c(parent_par)
    , socketDescriptor_pri(socketDescriptor_par_con)
{
}

void downloadServerThread_c::run()
{
#ifdef DEBUGJOUVEN
    //QOUT_TS("downloadServerThread_c::run()" << endl);
#endif
    downloadServerSocket_c* socketTmp = new downloadServerSocket_c(socketDescriptor_pri);

    connect(socketTmp, &QTcpSocket::destroyed, this, &QThread::quit);

    exec();
}
