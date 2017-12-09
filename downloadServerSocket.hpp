#ifndef CMS_DOWNLOADSERVERSOCKET_HPP
#define CMS_DOWNLOADSERVERSOCKET_HPP

#include <QTcpSocket>
#include <QSslSocket>

class downloadServerSocket_c : public QSslSocket
{
    Q_OBJECT

    int_fast64_t unwrittenBytesCount_pri = 0;
public:
    explicit downloadServerSocket_c(
            const qintptr socketDescriptor_par_con
            , QObject* parent_par = nullptr
    );
Q_SIGNALS:
private Q_SLOTS:
    void readyRead_f();
    void bytesWritten_f(qint64 bytes_par);
};

#endif // CMS_DOWNLOADSERVERSOCKET_HPP
