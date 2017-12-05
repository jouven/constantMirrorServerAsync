#ifndef CMS_DOWNLOADSERVERTHREAD_HPP
#define CMS_DOWNLOADSERVERTHREAD_HPP

#include "threadedFunctionQtso/threadedFunctionQt.hpp"

class downloadServerThread_c : public threadedFunction_c
{
    Q_OBJECT

    qintptr socketDescriptor_pri = 0;

    void run() override;
public:
    explicit downloadServerThread_c(
            const qintptr socketDescriptor_par_con
            , QObject* parent_par = nullptr
    );
};

#endif // CMS_DOWNLOADSERVERTHREAD_HPP
