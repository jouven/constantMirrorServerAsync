#ifndef CMS_FILELISTREQUESTSERVERTHREAD_HPP
#define CMS_FILELISTREQUESTSERVERTHREAD_HPP

#include "threadedFunctionQtso/threadedFunctionQt.hpp"

class fileListRequestServerThread_c : public threadedFunction_c
{
    Q_OBJECT

    qintptr socketDescriptor_pri = 0;

    void run() override;
public:
    explicit fileListRequestServerThread_c(
            const qintptr socketDescriptor_par_con
            , QObject* parent_par = nullptr
    );
};

#endif // CMS_FILELISTREQUESTSERVERTHREAD_HPP
