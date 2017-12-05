#ifndef CMS_DOWNLOADSERVERSOCKET_HPP
#define CMS_DOWNLOADSERVERSOCKET_HPP

#include <QTcpSocket>
#include <QSslSocket>

class downloadServerSocket_c : public QSslSocket
{
    Q_OBJECT
public:
    explicit downloadServerSocket_c(
            const qintptr socketDescriptor_par_con
            , QObject* parent_par = nullptr
    );

private Q_SLOTS:
    void readyRead_f();
    //void disconnected_f();
};

#endif // CMS_DOWNLOADSERVERSOCKET_HPP
