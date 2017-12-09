#include "downloadServerSocket.hpp"

#include "mirrorConfig.hpp"
#include "essentialQtso/essentialQt.hpp"
#include "signalso/signal.hpp"

#include <QFile>
#include <QThread>
#include <QJsonDocument>

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
    connect(this, &QSslSocket::encryptedBytesWritten, this, &downloadServerSocket_c::bytesWritten_f);
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
    QString filePathQStr;
    if (not mirrorConfig_ext.password_f().isEmpty())
    {
        QJsonDocument jsonDocumentTmp(QJsonDocument::fromJson(this->readAll()));
        requestWithPass_c requestWithPassTmp;
        requestWithPassTmp.read_f(jsonDocumentTmp.object());

        if (requestWithPassTmp.password_f() == mirrorConfig_ext.password_f())
        {
            filePathQStr = requestWithPassTmp.data_f();
        }
        else
        {
            this->disconnectFromHost();
        }
    }
    else
    {
        filePathQStr.append(this->readAll());
    }
#ifdef DEBUGJOUVEN
    //QOUT_TS("(downloadServerSocket_c::readyRead_f) filePathQStr " << filePathQStr << endl);
#endif
    //if the filePath is not empty and the filePath is on the localUmap
    if (not filePathQStr.isEmpty())
    {
        const std::pair<std::unordered_map<std::string, fileStatus_s>::const_iterator, bool> fileFindResult_con(mirrorConfig_ext.getHashedFileIterator_f(filePathQStr.toStdString()));
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
                        unwrittenBytesCount_pri = sizeReadTmp + 1;
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
                            //Explanation time:
                            //This happens in linux and windows with different degrees, but it's the same effect.
                            //Writing on the socket doesn't block until it's sent,
                            //what happens then is that the process memory keeps growing
                            //until qt-network-framework/kernel/whatever decides to start sending.
                            //And no matter what case:
                            //1 The server writes faster that the network can transmit
                            // or the client can read-write in its storage.
                            //2 The client and network are faster than the server.
                            //This effect can still happen, because still the server storage can be faster than the network stack, latency wise.
                            //Also it's impossible to know what the host might be doing with the network or the storage...
                            //This effect is pretty evident when sending files larger than the system memory size.
                            //It is pretty common since regular HD read speed is faster than the write speed

                            //Even when flushing/QCoreApplication::processEvents() after every write this effect can/will eventually happen,
                            //the best? way to mitigate this is to add a sleep and wait for encryptedBytesWritten,
                            //because without the sleep the cpu usage skyrockets looping flush/QCoreApplication::processEvents() until encryptedBytesWritten happens.
                            //And without any kind of wait the "pending" writes stay in memory and if the read data is big enough it can collapse the memory and the system

                            //It must be taken in account that this might not be apparent all the time because, initially the server disk activy is split
                            //between hashing and reading the files that's sending,
                            //but at some point it will finish hashing and all the disk activiy will be available for the "sending".
                            //Same with the client, at the start if no common files are present all the disk activity can focus on downloading-writing
                            //but once it finishes some downloads it will split time between downloading and checking the hashes of the finished downloads,
                            //and it can complex... if there are multiple clients,
                            //if the OS decided to cache the data and it's sending the same data at memory read speeds...

                            //at 2MB of unwritten data start waiting before writing more,
                            //because otherwise the memory usage will grow out of control,
                            //already with this measure it's already using 2MB of memory
                            //per socket (if the read data is that size or more)
                            //This theoretically limits the speed to 2000MB/s
                            while ((unwrittenBytesCount_pri > (BUFFERSIZE * 32 * 2)) and eines::signal::isRunning_f())
                            {
                                QCoreApplication::processEvents();
                                QThread::msleep(1);
                            }

                            sizeReadTmp = fileTmp.read(&bufferTmp[0], actualBufferSize);
                            unwrittenBytesCount_pri = unwrittenBytesCount_pri + sizeReadTmp;
                            if (fileFindResult_con.first->second.hashing_pub)
                            {
                                rehashingTmp = true;
                            }
                        } while ((sizeReadTmp > 0) and (fileTmp.bytesAvailable() >= actualBufferSize) and not rehashingTmp and eines::signal::isRunning_f());
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

void downloadServerSocket_c::bytesWritten_f(qint64 bytes_par)
{
    //QOUT_TS("(downloadServerSocket_c::bytesWritten_f) bytes_par " << bytes_par << endl);
    unwrittenBytesCount_pri = unwrittenBytesCount_pri - bytes_par;
    //QOUT_TS("(downloadServerSocket_c::bytesWritten_f) unwrittenBytesCount_pri " << unwrittenBytesCount_pri << endl);
}

//void downloadServerSocket_c::disconnected_f()
//{
//#ifdef DEBUGJOUVEN
//    QOUT_TS("(downloadServerSocket_c::disconnected_f) this->socketDescriptor() "  << this->socketDescriptor() << " Disconnected" << endl);
//#endif
//    this->deleteLater();
//}
