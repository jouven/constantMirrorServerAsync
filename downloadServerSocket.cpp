#include "downloadServerSocket.hpp"

#include "mirrorConfig.hpp"
#include "essentialQtso/essentialQt.hpp"
#include "signalso/signal.hpp"

#include <QFile>
#include <QThread>

#define BUFFERSIZE 1024 * 32 //32K

downloadServerSocket_c::downloadServerSocket_c(
        const qintptr socketDescriptor_par_con
        , QObject* parent_par) :
    QSslSocket(parent_par)
{
#ifdef DEBUGJOUVEN
    //QOUT_TS("downloadServerSocket_c::ctor()" << endl);
#endif
    connect(this, &QTcpSocket::readyRead, this, &downloadServerSocket_c::readyRead_f);
    connect(this, &QTcpSocket::disconnected, this, &downloadServerSocket_c::deleteLater);
    connect(this, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
    [=](QAbstractSocket::SocketError socketError)
    {
#ifdef DEBUGJOUVEN
        QOUT_TS("(downloadServerSocket_c::ctor() error) address " << this->peerAddress().toString()
                 << " port " << this->peerPort()
                 << " error: " << this->errorString() << endl);
#endif
        if (this->error() != QAbstractSocket::RemoteHostClosedError)
        {
#ifdef DEBUGJOUVEN
            QOUT_TS("(downloadServerSocket_c::ctor() error) his->deleteLater()" << endl);
#endif
            this->deleteLater();
        }
    });
    connect(this, static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors), [=](const QList<QSslError> &errors)
    {
#ifdef DEBUGJOUVEN
        for (const QSslError& errorItem_ite_con : errors)
        {
            QOUT_TS("(downloadServerSocket_c::ctor() sslerror) " << errorItem_ite_con.errorString() << endl);
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


void downloadServerSocket_c::readyRead_f()
{
    //there is no toQString
    std::string filePathStr(this->readAll().toStdString());
    //convert w/e there is in the readAll as it was a filename and try to check it
    QString filePathQStr(QString::fromStdString(filePathStr));
#ifdef DEBUGJOUVEN
    //QOUT_TS("(downloadServerSocket_c::readyRead_f) filePathQStr " << filePathQStr << endl);
#endif
    //if the filePath is not empty and the filePath is on the localUmap
    if (not filePathQStr.isEmpty())
    {
        const std::pair<std::unordered_map<std::string, fileStatus_s>::const_iterator, bool> fileFindResult_con(mirrorConfig_ext.getHashedFileIterator_f(filePathStr));
        if (fileFindResult_con.second)
        {
            QFile fileTmp(filePathQStr);
            if (fileTmp.exists() and fileTmp.open(QIODevice::ReadOnly))
            {
                quint64 actualBufferSize(0);
#ifdef DEBUGJOUVEN
                //QOUT_TS("(downloadServerSocket_c::readyRead_f) fileTmp.bytesAvailable() " << fileTmp.bytesAvailable() << endl);
                //QOUT_TS("(downloadServerSocket_c::readyRead_f) fileTmp.size() " << fileTmp.size() << endl);
#endif
                if (fileTmp.bytesAvailable() > BUFFERSIZE)
                {
                    actualBufferSize = BUFFERSIZE;
                }
                else
                {
                    actualBufferSize = fileTmp.bytesAvailable();
                }
                //empty file "special" case
                if (fileTmp.bytesAvailable() == 0)
                {
                    this->write("0");
                }
                else
                {
                    std::string bufferTmp(actualBufferSize, '\0');
                    qint64 sizeReadTmp(fileTmp.read(&bufferTmp[0], actualBufferSize));
                    //theoretically this is not possible, but... I don't even...
                    if (sizeReadTmp < 0)
                    {
                        //4 = reading past the end of the stream is considered an error, so this function returns -1 in those cases
                        //(that is, reading on a closed socket or after a process has died).
                        this->write("4");
                        QOUT_TS("Error reading file to serve a download " << filePathQStr << endl);
                    }
                    else
                    {
                        //first byte is to tell status, 256 values should be fine? for now
                        //0 = success
                        this->write("0");
                        bool rehashingTmp(false);
                        do
                        {
#ifdef DEBUGJOUVEN
                            //QOUT_TS("(downloadServerSocket_c::readyRead_f) do write bufferTmp " << QString::fromStdString(bufferTmp) << endl);
                            //QOUT_TS("(downloadServerSocket_c::readyRead_f) actualBufferSize " << actualBufferSize << endl);
#endif
                            if (not eines::signal::isRunning_f())
                            {
                                this->disconnectFromHost();
                                return;
                            }
                            this->write(&bufferTmp[0], sizeReadTmp);
                            if (this->bytesToWrite() > actualBufferSize * 32)
                            {
                                while (not this->flush() and eines::signal::isRunning_f())
                                {
                                    QThread::msleep(1);
                                }
                            }
                            sizeReadTmp = fileTmp.read(&bufferTmp[0], actualBufferSize);
                            if (fileFindResult_con.first->second.hashing_pub)
                            {
                                rehashingTmp = true;
                            }
                        } while ((sizeReadTmp > 0) and (fileTmp.bytesAvailable() >= actualBufferSize) and not rehashingTmp);
                        //the loop goes out when:
                        //1 there is no more to read OR
                        //2 buffer is bigger than what's left to read
#ifdef DEBUGJOUVEN
                        //QOUT_TS("(downloadServerSocket_c::readyRead_f) sizeReadTmp " << sizeReadTmp << endl);
                        //QOUT_TS("(downloadServerSocket_c::readyRead_f) fileTmp.bytesAvailable() " << fileTmp.bytesAvailable() << endl);
#endif
                        if (not rehashingTmp)
                        {
                            if (sizeReadTmp < 0)
                            {
                                QOUT_TS("Error reading file to serve a download " << filePathQStr << endl);
                            }
                            else
                            {
                                if (sizeReadTmp > 0)
                                {
                                    this->write(&bufferTmp[0], sizeReadTmp);
                                }
                                if (fileTmp.bytesAvailable() > 0)
                                {
                                    sizeReadTmp = fileTmp.read(&bufferTmp[0], fileTmp.bytesAvailable());
                                    if (sizeReadTmp < 0)
                                    {
                                        QOUT_TS("Error reading file to serve a download " << filePathQStr << endl);
                                    }
                                    else
                                    {
                                        this->write(&bufferTmp[0], sizeReadTmp);
                                    }
                                }
                                else
                                {
                                    //no problem here
                                    //buffer size is multiple of the file size
                                    //so it did the right amount of reads with that buffer size
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                //3 = file doesn't exist physically but it was in the list
                this->write("3");
                QOUT_TS("File doesn't exist or it couldn't be open " << filePathQStr << endl);
            }
        }
        else
        {
            //2 = file doesn't exist in the list
            this->write("2");
        }
    }
    else
    {
        //1 = file empty
        this->write("1");
        //This is a normal error
        QOUT_TS("Request file not found " << filePathQStr << endl);
    }
#ifdef DEBUGJOUVEN
    //QOUT_TS("(downloadServerSocket_c::readyRead_f) this->disconnectFromHost()" << endl);
#endif
    this->disconnectFromHost();
}

//void downloadServerSocket_c::disconnected_f()
//{
//#ifdef DEBUGJOUVEN
//    QOUT_TS("(downloadServerSocket_c::disconnected_f) this->socketDescriptor() "  << this->socketDescriptor() << " Disconnected" << endl);
//#endif
//    this->deleteLater();
//}
