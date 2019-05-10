//program initial config reading structs/classes, the config is read from a json configuration file
#ifndef CMS_MIRRORCONFIG_HPP
#define CMS_MIRRORCONFIG_HPP

#include "baseClassQtso/baseClassQt.hpp"
#include "fileHashQtso/fileHashQt.hpp"
#include "downloadServer.hpp"
#include "fileListRequestServer.hpp"

#include <QStringList>
#include <QHostAddress>
#include <QSet>
#include <QTimer>

#include <chrono>
#include <vector>
#include <unordered_map>
//#include <unordered_set>

class requestWithPass_c
{
    QString data_pri;
    QString password_pri;
public:
    requestWithPass_c() = default;
    requestWithPass_c(
            const QString& data_par_con
            , const QString& password_par_con = QString()
    );

    void read_f(const QJsonObject &json);
    void write_f(QJsonObject &json) const;

    QString password_f() const;
    QString data_f() const;
};

struct clientInfo_s
{
    QHostAddress address_pub;
    quint16 updatePort_pub = 0;
    uint_fast64_t failedTries_pub = 0;
    uint_fast64_t lastFileRequestTime_pub = 0;
    clientInfo_s(
            const QHostAddress& address_par_con
            , const quint16& updatePort_par_con
    ) : address_pub(address_par_con)
      , updatePort_pub(updatePort_par_con)
    {
    }
};

class mirrorConfigSourceDestinationMapping_c : public baseClassQt_c
{
    int_fast64_t id_pri = 0;

    //serialized/deserialized fields BEGIN

    //local path to "share"
    QString originalSourcePath_pri;
    QString sourcePath_pri;

    //get filtered files, ignore those which do not match
    QStringList filenameFilters_pri;
    bool includeSubdirectories_pri = true;
    //for directory paths, source wise only grab files from those with a named file
    QString includeDirectoriesWithFileX_pri;

    //"source"/"destination" how often to check local files for changes
    qint64 localCheckIntervalMilliseconds_pri = 10000;

    int_fast64_t localLastCheckedIntervalMilliseconds_pri = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    //serialized/deserialized fields END

    //contains
    //std::unordered_map<std::string, fileStatus_s> localFileStatusUMAP_pri;
    //but it's localFileStatusUMAP_pub
    //key = localPath
    fileHashControl_c fileHashControl_pri;

    //control variables
    bool generateJSONRequired_pri = false;

    //result of isValid_f
    bool isValid_pri = false;

    //variable to track if the path is just a single file or a folder with files
    bool isSingleFile_pri = false;
    bool isSingleFileSet_pri = false;
public:
    mirrorConfigSourceDestinationMapping_c();

    void read_f(const QJsonObject &json);
    void write_f(QJsonObject &json) const;
    //QString sourcePath_f() const;
    //QString sourceAddress_f() const;
    //QString getIncludeDirectoriesWithFileX_f() const;
    qint64 localCheckIntervalMilliseconds_f() const;
    bool isValid_f() const;

    void checkValid_f();

    void localScan_f();
    //this makes a copy
    std::unordered_map<std::string, fileStatus_s> localFileStatusUMAP_f() const;
    bool generateJSONRequired_f();
    std::string fileHashControlMutexName_f() const;
    std::pair<std::unordered_map<std::string, fileStatus_s>::iterator, bool> getFileHashControlPair_f(const std::string& key_par_con);
    bool fileHashControlChanged_f();
};

class mirrorConfig_c : public baseClassQt_c
{
    //TODO, AFTER IT WORKS, make them optional and just use the defaults, printing, after, what the defaults are after the server/s is up
    //ip-dns interface to use (always mandatory)

    //serialized/deserialized fields BEGIN

    QString selfServerAddressStr_pri;
    QHostAddress selfServerAddress_pri;

    //request server port (mandatory for source)
    quint16 fileListServerPort_pri = 0;
    //file server port (mandatory for source)
    quint16 downloadServerPort_pri = 0;

    std::vector<mirrorConfigSourceDestinationMapping_c> sourceDestinationMappings_pri;

    //if the sender reads faster than the client can write...
    uint_fast64_t senderFasterThanReceiver_pri = 0;

    QString password_pri;

    //serialized/deserialized fields END

    std::unordered_map<std::string, clientInfo_s> recentClients_pri;
    bool notifyClientsOfUpdate_pri = false;

    //the json to serve on the file list request
    QByteArray sourceJSONFileListData_pri;

    //threadedFunction_c* localScanThread_pri = nullptr;
    bool localScanThreadExists_pri = false;

    qint64 gcdWaitMillisecondsAll_pri = 0;

//    QSslKey privateKey_pri;
//    QSslCertificate privateKeyCertificate_pri;
//    bool privateKeyAndCertificateInitialized_pri = false;

    //holds the result of checkValid_f
    bool valid_pri = false;

    void generateJSONFileList();
    //this controls the first generation, it also fixes the case of an initial empty file list
    bool JSONFileListGeneratedOnce_pri = false;
    void localScan_f();
    void checkValid_f();

    void read_f(const QJsonObject &json);
    void write_f(QJsonObject &json) const;

    QTimer* mainLoopTimer_pri;
    downloadServer_c* downloadServer_pri;
    fileListRequestServer_c* fileRequestServer_pri;
public:

    //QString selfServerAddress_f() const;
    //std::vector<mirrorConfigSourceDestinationMapping_c> sourceDestinationMappings_f() const;

    //std::unordered_map<std::string, fileStatus_s> localFileStatusUMAP_f() const;
    QByteArray JSONFileListData_f();
    quint16 fileListServerPort_f() const;

    void initialSetup_f();
    void mainLoop_f();

    std::pair<std::unordered_map<std::string, fileStatus_s>::const_iterator, bool> getHashedFileIterator_f(const std::string& filename_par_con);
    void addRecentClient(const QHostAddress ip_par_con, const quint16 port_par_con);
    QString password_f() const;
};

extern mirrorConfig_c mirrorConfig_ext;

#endif // MIRRORCONFIG_HPP
