#include "fileListRequestServerThread.hpp"

#include "fileListRequestServerSocket.hpp"

#include "essentialQtso/essentialQt.hpp"

fileListRequestServerThread_c::fileListRequestServerThread_c(const qintptr socketDescriptor_par_con
        , QObject *parent_par) :
    threadedFunction_c(parent_par)
    , socketDescriptor_pri(socketDescriptor_par_con)
{
}

void fileListRequestServerThread_c::run()
{
#ifdef DEBUGJOUVEN
    //QOUT_TS("fileListRequestThread_c::run()" << endl);
#endif
    fileListRequestSocket_c* fileListRequestSocketTmp = new fileListRequestSocket_c(socketDescriptor_pri);

    connect(fileListRequestSocketTmp, &QTcpSocket::destroyed, this, &QThread::quit);

    exec();
}

