#ifndef CMS_FILELISTREQUESTSERVERSOCKET_HPP
#define CMS_FILELISTREQUESTSERVERSOCKET_HPP

#include <QTcpSocket>
#include <QSslSocket>

class fileListRequestSocket_c : public QSslSocket
{
    Q_OBJECT

    //bool readClientPort_pri = false;
    //bool fileListSent_pri = false;

    //void disconnectAfter_f();
public:
    explicit fileListRequestSocket_c(
            const qintptr socketDescriptor_par_con
            , QObject* parent_par = nullptr
    );
private Q_SLOTS:
    //void disconnected_f();
    void readyRead_f();
    //void sendFileList_f();
};

#endif // CMS_FILELISTREQUESTSERVERSOCKET_HPP
