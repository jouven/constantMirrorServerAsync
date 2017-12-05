#ifndef CMS_FILELISTREQUESTSERVER_HPP
#define CMS_FILELISTREQUESTSERVER_HPP

#include <QTcpServer>

class fileListRequestServer_c : public QTcpServer
{
    Q_OBJECT

    void incomingConnection(qintptr socketDescriptor) override;
public:
    explicit fileListRequestServer_c(
            const QHostAddress &address_par_con = QHostAddress::Any
            , const quint16 port_par_con = 0
            , QObject *parent_par = nullptr);
};

#endif // CMS_FILELISTREQUESTSERVER_HPP
