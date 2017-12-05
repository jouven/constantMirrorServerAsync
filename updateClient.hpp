#ifndef CMS_UPDATECLIENT_HPP
#define CMS_UPDATECLIENT_HPP

#include <QTcpSocket>
#include <QSslSocket>
#include <QHostAddress>

class updateClient_c : public QSslSocket
{
    Q_OBJECT

    QHostAddress address_pri;
    quint16 port_pri;
    bool success_pri = false;
public:
    explicit updateClient_c(
            const QHostAddress& address_par_con
            , const quint16 port_par_con
            , QObject *parent = nullptr);

    bool success_f() const;
private Q_SLOTS:
    void successfulConnection_f();
};

#endif // CMS_UPDATECLIENT_HPP
