#ifndef CMS_DOWNLOADSERVER_HPP
#define CMS_DOWNLOADSERVER_HPP

#include <QTcpServer>
#include <QHostAddress>

class downloadServer_c : public QTcpServer
{
    Q_OBJECT

    void incomingConnection(qintptr socketDescriptor) override;
public:
    explicit downloadServer_c(
            const QHostAddress &address_par_con = QHostAddress::Any
            , const quint16 port_par_con = 0
            , QObject *parent_par = nullptr);
};


#endif // CMS_DOWNLOADSERVER_HPP
