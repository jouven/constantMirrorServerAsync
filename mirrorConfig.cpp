#include "mirrorConfig.hpp"

#include "updateClient.hpp"

#include "signalso/signal.hpp"

#include "essentialQtso/essentialQt.hpp"
#include "fileHashQtso/fileHashQt.hpp"
#include "qmutexUMapQtso/qmutexUMapQt.hpp"
#include "sslUtilsso/sslUtils.hpp"
#include "threadedFunctionQtso/threadedFunctionQt.hpp"

#include "comuso/practicalTemplates.hpp"

#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QHostAddress>
#include <QHostInfo>
#include <QCommandLineParser>
#include <QRegExp>
#include <QSslConfiguration>
#include <QSslKey>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <boost/math/common_factor.hpp>

int_fast64_t generateId_f()
{
    static int_fast64_t rootId(0);
    rootId = rootId + 1;
    return rootId;
}


requestWithPass_c::requestWithPass_c(
        const QString &data_par_con
        , const QString &password_par_con)
    : data_pri(data_par_con)
    , password_pri(password_par_con)
{}

void requestWithPass_c::read_f(const QJsonObject &json)
{
    data_pri = json["data"].toString();
    password_pri = json["password"].toString();
}

void requestWithPass_c::write_f(QJsonObject &json) const
{
    json["data"] = data_pri;
    json["password"] = password_pri;
}

QString requestWithPass_c::data_f() const
{
    return data_pri;
}

QString requestWithPass_c::password_f() const
{
    return password_pri;
}

//QString mirrorConfigSourceDestinationMapping_c::sourcePath_f() const
//{
//    return sourcePath_pri;
//}

qint64 mirrorConfigSourceDestinationMapping_c::localCheckIntervalMilliseconds_f() const
{
    return localCheckIntervalMilliseconds_pri;
}

bool mirrorConfigSourceDestinationMapping_c::isValid_f() const
{
    return isValid_pri;
}

//QString mirrorConfigSourceDestinationMapping_c::getIncludeDirectoriesWithFileX_f() const
//{
//    return includeDirectoriesWithFileX_pri;
//}

std::unordered_map<std::string, fileStatus_s> mirrorConfigSourceDestinationMapping_c::localFileStatusUMAP_f() const
{
    return fileHashControl_pri.fileStatusUMAP_pub;
}

bool mirrorConfigSourceDestinationMapping_c::generateJSONRequired_f()
{
    bool boolTmp(generateJSONRequired_pri);
    generateJSONRequired_pri = false;
    return boolTmp;
}

std::string mirrorConfigSourceDestinationMapping_c::fileHashControlMutexName_f() const
{
    return fileHashControl_pri.mutexName_f();
}

bool mirrorConfigSourceDestinationMapping_c::fileHashControlChanged_f()
{
    return fileHashControl_pri.fileStatusUMAPChanged_f();
}

std::pair<std::unordered_map<std::string, fileStatus_s>::iterator, bool> mirrorConfigSourceDestinationMapping_c::getFileHashControlPair_f(const std::string& key_par_con)
{
    bool found(false);
    std::unordered_map<std::string, fileStatus_s>::iterator iteratorResult(fileHashControl_pri.fileStatusUMAP_pub.find(key_par_con));
    if (iteratorResult != fileHashControl_pri.fileStatusUMAP_pub.end())
    {
        found = true;
    }
    return {iteratorResult, found};
}

mirrorConfigSourceDestinationMapping_c::mirrorConfigSourceDestinationMapping_c()
    : id_pri(generateId_f()), fileHashControl_pri("localFileStatusUMAPMutexId_" + std::to_string(id_pri))
{
}

void mirrorConfigSourceDestinationMapping_c::read_f(const QJsonObject &json)
{
    originalSourcePath_pri = json["sourcePath"].toString();
    sourcePath_pri = QDir::fromNativeSeparators(originalSourcePath_pri);
    QJsonArray jsonArrayFilenameFiltersTmp(json["filenameFilters"].toArray());
    if (not jsonArrayFilenameFiltersTmp.isEmpty())
    {
        filenameFilters_pri.reserve(jsonArrayFilenameFiltersTmp.size());
        for (const auto& jsonArrayItem_ite_con : jsonArrayFilenameFiltersTmp)
        {
            filenameFilters_pri.append(jsonArrayItem_ite_con.toString());
            //qout_glo << "jsonArrayItem_ite_con.toString() " << jsonArrayItem_ite_con.toString() << endl;
        }
    }
    includeSubdirectories_pri = json["includeSubdirectories"].toBool(true);
    includeDirectoriesWithFileX_pri = json["includeDirectoriesWithFileX"].toString();
    if (not json["localCheckIntervalMilliseconds"].isUndefined())
    {
        localCheckIntervalMilliseconds_pri = json["localCheckIntervalMilliseconds"].toInt(1000);
    }
}

void mirrorConfigSourceDestinationMapping_c::write_f(QJsonObject &json) const
{
    json["sourcePath"] = sourcePath_pri;
    if (not filenameFilters_pri.isEmpty())
    {
        QJsonArray jsonArrayFilenameFiltersTmp;
        for (const auto& filenameFilter_ite_con : filenameFilters_pri)
        {
            jsonArrayFilenameFiltersTmp.append(QJsonValue(filenameFilter_ite_con));
        }
        json["filenameFilters"] = jsonArrayFilenameFiltersTmp;
    }
    json["includeSubdirectories"] = includeSubdirectories_pri;
    json["includeDirectoriesWithFileX"] = includeDirectoriesWithFileX_pri;
    json["localCheckIntervalMilliseconds"] = localCheckIntervalMilliseconds_pri;
}

void mirrorConfigSourceDestinationMapping_c::localScan_f()
{
#ifdef DEBUGJOUVEN
    //QOUT_TS("(mirrorConfigSourceDestinationMapping_c::localScan_f() begin" << endl);
#endif
    if (not eines::signal::isRunning_f())
    {
        return;
    }
    //if the last checked interval + interval time is greater than the current time --> skip
    if ((localLastCheckedIntervalMilliseconds_pri + localCheckIntervalMilliseconds_pri) > std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count())
    {
        //do nothing
    }
    else
    {
        localLastCheckedIntervalMilliseconds_pri = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

        bool anyFileChangedTmp(false);
        QFileInfo sourceTmp(sourcePath_pri);
        if (sourceTmp.exists())
        {
            //case single file
            if (sourceTmp.isFile())
            {   
                anyFileChangedTmp = fileHashControl_pri.hashFileInUMAP_f(
                            sourceTmp
                            );
            }

            //case directory (0 or many files)
            if (sourceTmp.isDir())
            {
                anyFileChangedTmp = fileHashControl_pri.hashDirectoryInUMAP_f(
                            sourceTmp
                            , filenameFilters_pri
                            , includeSubdirectories_pri
                            , includeDirectoriesWithFileX_pri
                            );
            }
        }

        QMutexLocker lockerTmp(getAddMutex_f(fileHashControl_pri.mutexName_f()));
        //remove non-existing keys
        std::vector<std::string> localFileKeysToRemove;
        for (auto& localFileStatus_ite : fileHashControl_pri.fileStatusUMAP_pub)
        {
            if (not localFileStatus_ite.second.iterated_pub)
            {
                localFileKeysToRemove.emplace_back(localFileStatus_ite.first);
            }
            else
            {
                localFileStatus_ite.second.iterated_pub = false;
            }
        }

        for (const auto& localFileKey_ite_con : localFileKeysToRemove)
        {
            auto removeResult(fileHashControl_pri.fileStatusUMAP_pub.erase(localFileKey_ite_con));
            if (removeResult > 0)
            {
                anyFileChangedTmp = true;
            }
        }
        generateJSONRequired_pri = anyFileChangedTmp;
        QOUT_TS("Finished local scan changed=" << anyFileChangedTmp << " " << sourceTmp.filePath() << endl);
    }
#ifdef DEBUGJOUVEN
    //QOUT_TS("(mirrorConfigSourceDestinationMapping_c::localScan_f() end" << endl);
#endif
}

void mirrorConfigSourceDestinationMapping_c::checkValid_f()
{
    bool isValidTmp(true);
    if (sourcePath_pri.isEmpty())
    {
        isValidTmp = false;
        appendError_f("Source path is empty");
    }

    if (localCheckIntervalMilliseconds_pri < 1)
    {
        appendError_f("Local check interval time is invalid " + localCheckIntervalMilliseconds_pri);
        isValidTmp = false;
    }

    isValid_pri = isValidTmp;
}

///////////////////////////
//mirroConfig_c stuff
///////////////////////////

void mirrorConfig_c::localScan_f()
{
#ifdef DEBUGJOUVEN
    //QOUT_TS("(mirrorConfig_c::localScan_f() begin" << endl);
#endif
    bool generateJSONRequiredTmp(false);
    for (mirrorConfigSourceDestinationMapping_c& sourceDestinationMapping_ite : sourceDestinationMappings_pri)
    {
        sourceDestinationMapping_ite.localScan_f();
        generateJSONRequiredTmp = generateJSONRequiredTmp or sourceDestinationMapping_ite.generateJSONRequired_f();
    }

    if (generateJSONRequiredTmp or not JSONFileListGeneratedOnce_pri)
    {
        generateJSONFileList();
        if (JSONFileListGeneratedOnce_pri)
        {
            notifyClientsOfUpdate_pri = true;
        }
        else
        {
            JSONFileListGeneratedOnce_pri = true;
        }
    }
#ifdef DEBUGJOUVEN
    //QOUT_TS("(mirrorConfig_c::localScan_f() end" << endl);
#endif
}

//QString mirrorConfig_c::selfServerAddress_f() const
//{
//    return selfServerAddressStr_pri;
//}

//std::vector<mirrorConfigSourceDestinationMapping_c> mirrorConfig_c::sourceDestinationMappings_f() const
//{
//    return sourceDestinationMappings_pri;
//}


void mirrorConfig_c::checkValid_f()
{
    bool validResult(true);
    if (selfServerAddressStr_pri.isEmpty())
    {
        appendError_f("No ip interface set, selfServerAddress, in config file config.json");
        validResult = false;
    }

    if (not selfServerAddress_pri.setAddress(selfServerAddressStr_pri))
    {
        QHostInfo qHostInfoTmp(QHostInfo::fromName(selfServerAddressStr_pri));
        if (qHostInfoTmp.addresses().isEmpty())
        {
            appendError_f("No valid self server address or failed to resolve it");
            validResult = false;
        }
        else
        {
#ifdef DEBUGJOUVEN
            //QOUT_TS("mirrorConfig_c::checkValid_f() qHostInfoTmp.addresses().first().toString() " << qHostInfoTmp.addresses().first().toString() << endl);
            //QOUT_TS("mirrorConfig_c::checkValid_f() qHostInfoTmp.addresses().first().toIPv4Address() " << qHostInfoTmp.addresses().first().toIPv4Address() << endl);
#endif
            selfServerAddress_pri = qHostInfoTmp.addresses().first();
        }
    }

    if (fileListServerPort_pri == 0)
    {
        appendError_f("No port set for the request server, requestServerPort, in config file config.json");
        validResult = false;
    }

    if (downloadServerPort_pri == 0)
    {
        appendError_f("No port set for the file server, fileServerPort, in config file config.json");
        validResult = false;
    }

    if (sourceDestinationMappings_pri.empty())
    {
        appendError_f("Empty mappings section in config file config.json");
        validResult = false;
    }

    std::vector<int_fast64_t> validIntervals;
    for (mirrorConfigSourceDestinationMapping_c& sourceDestinationMapping_ite : sourceDestinationMappings_pri)
    {
        sourceDestinationMapping_ite.checkValid_f();
        appendError_f(sourceDestinationMapping_ite.getError_f());
        if (sourceDestinationMapping_ite.isValid_f())
        {
            validIntervals.emplace_back(sourceDestinationMapping_ite.localCheckIntervalMilliseconds_f());
        }
        else
        {
            validResult = false;
        }
    }


    if(validResult)
    {
        //the third field in std::acumulate is the initial/default value
        gcdWaitMillisecondsAll_pri = std::accumulate(validIntervals.cbegin(), validIntervals.cend(), validIntervals.front(), [](const int_fast64_t a, const int_fast64_t b)
        {
            return boost::math::gcd(a, b);
        });

        eines::removeIfPredicateTrue_f(sourceDestinationMappings_pri, [](const auto& item_par_con){return not item_par_con->isValid_f();});
    }

    valid_pri = validResult;
}

quint16 mirrorConfig_c::fileListServerPort_f() const
{
    return fileListServerPort_pri;
}

QString mirrorConfig_c::password_f() const
{
    return password_pri;
}


void mirrorConfig_c::generateJSONFileList()
{
    QMutexLocker locker1(getAddMutex_f("JSONFileList"));

#ifdef DEBUGJOUVEN
    //QOUT_TS("(mirrorConfig_c::generateJSONFileList()" << endl);
#endif
    std::unordered_map<std::string, fileStatus_s> allFilesTmp;
    {
        uint_fast32_t listSize(0);

        for (const mirrorConfigSourceDestinationMapping_c& mappingItem_ite_con : sourceDestinationMappings_pri)
        {
            QMutexLocker locker2(getAddMutex_f(mappingItem_ite_con.fileHashControlMutexName_f()));
            listSize = listSize + mappingItem_ite_con.localFileStatusUMAP_f().size();
        }
        allFilesTmp.reserve(listSize);
        for (const mirrorConfigSourceDestinationMapping_c& mappingItem_ite_con : sourceDestinationMappings_pri)
        {
            QMutexLocker locker2(getAddMutex_f(mappingItem_ite_con.fileHashControlMutexName_f()));
            for (const auto& item_ite_con : mappingItem_ite_con.localFileStatusUMAP_f())
            {
                allFilesTmp.emplace(item_ite_con);
            }
        }
    }
    fileStatusArray_s objToSerialize(allFilesTmp);
    QJsonObject jsonObjectTmp;
    objToSerialize.write_f(jsonObjectTmp);

    QJsonDocument jsonDocTmp(jsonObjectTmp);

    sourceJSONFileListData_pri = jsonDocTmp.toJson(QJsonDocument::JsonFormat::Compact);
}

QByteArray mirrorConfig_c::JSONFileListData_f()
{
    if (not JSONFileListGeneratedOnce_pri)
    {
        generateJSONFileList();
        if (JSONFileListGeneratedOnce_pri)
        {
            notifyClientsOfUpdate_pri = true;
        }
        else
        {
            JSONFileListGeneratedOnce_pri = true;
        }
    }
    else
    {
        bool changeDetectedTmp(false);
        for (mirrorConfigSourceDestinationMapping_c& item_ite : sourceDestinationMappings_pri)
        {
            if (item_ite.fileHashControlChanged_f())
            {
                changeDetectedTmp = true;
                break;
            }
        }
        if (changeDetectedTmp)
        {
            generateJSONFileList();
            notifyClientsOfUpdate_pri = true;
        }
    }
    QMutexLocker lockerTmp(getAddMutex_f("JSONFileList"));
    return sourceJSONFileListData_pri;
}

std::pair<std::unordered_map<std::string, fileStatus_s>::const_iterator, bool> mirrorConfig_c::getHashedFileIterator_f(const std::string &filename_par_con)
{
    for (mirrorConfigSourceDestinationMapping_c& item_ite_con : sourceDestinationMappings_pri)
    {
        QMutexLocker locker1(getAddMutex_f(item_ite_con.fileHashControlMutexName_f()));
        //it doesn't matter if its a copy (localFileStatusUMAP_f "copies" the umap) because it's const_iterator
        auto findResult(item_ite_con.getFileHashControlPair_f(filename_par_con));
        if (findResult.second)
        {
            return findResult;
        }
    }
    return {std::unordered_map<std::string, fileStatus_s>::const_iterator(), false};
}

void mirrorConfig_c::addRecentClient(
        const QHostAddress ip_par_con
        , const quint16 port_par_con)
{
#ifdef DEBUGJOUVEN
    //QOUT_TS("mirrorConfig_c::addRecentClient ip_par_con.toString() " << ip_par_con.toString() << endl);
    //QOUT_TS("mirrorConfig_c::addRecentClient ip_par_con.toIPv4Address() " << ip_par_con.toIPv4Address() << endl);
#endif
    clientInfo_s clientInfotmp(ip_par_con, port_par_con);
    std::string keyTmp(std::to_string(ip_par_con.toIPv4Address()) + "_" + std::to_string(port_par_con));
    QMutexLocker mutexLockerTmp(getAddMutex_f("recentClients"));
    auto findResult(recentClients_pri.find(keyTmp));
    if (findResult != recentClients_pri.end())
    {
        findResult->second.lastFileRequestTime_pub = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    else
    {
        recentClients_pri.emplace(keyTmp, clientInfotmp);
    }
}

void mirrorConfig_c::read_f(const QJsonObject &json)
{
    selfServerAddressStr_pri = json["selfServerAddress"].toString();
    fileListServerPort_pri = json["requestServerPort"].toInt();
    downloadServerPort_pri = json["fileServerPort"].toInt();

    QJsonArray arrayTmp(json["sourceDestinationMappings"].toArray());
    sourceDestinationMappings_pri.reserve(arrayTmp.size());
    for (const auto& item_ite_con : arrayTmp)
    {
        QJsonObject mirrorConfigSourceDestinationJsonObject = item_ite_con.toObject();
        mirrorConfigSourceDestinationMapping_c mirrorConfigSourceDestinationMappingTmp;
        mirrorConfigSourceDestinationMappingTmp.read_f(mirrorConfigSourceDestinationJsonObject);
        sourceDestinationMappings_pri.emplace_back(mirrorConfigSourceDestinationMappingTmp);
    }
    password_pri = json["password"].toString();
}
void mirrorConfig_c::write_f(QJsonObject &json) const
{
    QJsonArray sourceDestinationMappingArrayTmp;
    for (const auto& sourceDestinationMapping_ite_con : sourceDestinationMappings_pri)
    {
        QJsonObject jsonObjectTmp;
        sourceDestinationMapping_ite_con.write_f(jsonObjectTmp);
        sourceDestinationMappingArrayTmp.append(jsonObjectTmp);
    }

    json["sourceDestinationMappings"] = sourceDestinationMappingArrayTmp;
    json["selfServerAddress"] = selfServerAddressStr_pri;
    json["requestServerPort"] = fileListServerPort_pri;
    json["fileServerPort"] = downloadServerPort_pri;
    json["password"] = password_pri;
}

void mirrorConfig_c::initialSetup_f()
{
    QCoreApplication::setApplicationName("constantMirrorServerTcp");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser commandLineParser;
    commandLineParser.setApplicationDescription("constantMirrorServerTcp description");
    commandLineParser.addHelpOption();
    commandLineParser.addVersionOption();
    commandLineParser.addPositionalArgument("target", "Optional, config.json full path, by default it tries to read this file on the same path where the executable is");

    //Process the actual command line arguments given by the user
    commandLineParser.process(*qApp);

    QString errorStr;
    while (errorStr.isEmpty())
    {
        QString configFilePathStr;
        const QStringList parsedPositionalArgs(commandLineParser.positionalArguments());
        if (parsedPositionalArgs.size() > 0)
        {
            QString configjsonAlternativePathStr(parsedPositionalArgs.at(0));
            if (configjsonAlternativePathStr.isEmpty())
            {
                errorStr.append("Config.json path is empty");
                break;
            }

            if (not QFile::exists(configjsonAlternativePathStr))
            {
                errorStr.append("Config.json path doesn't exist " + configjsonAlternativePathStr);
                break;
            }
            configFilePathStr = configjsonAlternativePathStr;
        }

        if (configFilePathStr.isEmpty())
        {
            configFilePathStr = QCoreApplication::applicationDirPath() + "/config.json";
        }
        //qout_glo << "configFilePathStr " << configFilePathStr << endl;
        QFile configFile(configFilePathStr);
        if (not configFile.exists())
        {
            errorStr.append("Config file, config.json, doesn't exist.\nIt has to exist on the same path as the constantMirrorServerTcp executable and it must have the following structure:\n"
R"({
    "selfServerAddress": "192.168.1.5"
    , "requestServerPort": "30001"
    , "fileServerPort": "30002"
    , "sourceDestinationMappings": [
        {
            "localCheckIntervalMilliseconds": 1000
            , "sourcePath": "/source/path"
            , "filenameFilters": [
                "*.exe",
                "*.bat"
            ],
            "includeSubdirectories": true,
            "includeDirectoriesWithFileX": true
        }
    ]
}

"selfServerAddress" mandatory, ip/dns-interface to use when setting a server

"requestServerPort" which port to use to setup the server to make request to get the hash+size per file list

"fileServerPort" which port to use to setup the server to download files

"password" optional, servers, download and file request, will check for this password in the client/s request/s, failure to comply will result on their disconnection

"sourceDestinationMappings[x].localCheckIntervalMilliseconds" optional, interval to check for changes locally. 1000 ms by default

"sourceDestinationMappings[x].sourcePath" remote file or directory path to fetch files from

"sourceDestinationMappings[x].filenameFilters" optional, when dealing with a directory source (not for single files) filter filenames, all by default

"sourceDestinationMappings[x].includeSubdirectories" optional, when dealing with a remote directory include/recurse subdirectories for files to fetch, true by default

"sourceDestinationMappings[x].includeDirectoriesWithFileX" optional, for directory mirroring, it will only mirror remote diretories, where one of the files has a particular filename)");
                    break;
        }

        QByteArray jsonByteArray;
        if (configFile.open(QIODevice::ReadOnly))
        {
            jsonByteArray = configFile.readAll();
        }
        else
        {
            errorStr.append("Could not open config file config.json");
            break;
        }

        auto jsonDocObj(QJsonDocument::fromJson(jsonByteArray));
        if (jsonDocObj.isNull())
        {
            errorStr.append("Could not parse json from the config file config.json");
            break;
        }
        else
        {
            read_f(jsonDocObj.object());
        }

        checkValid_f();
        errorStr.append(getError_f());

        break;
    }

    if (not errorStr.isEmpty())
    {
        qtErrRef_ext() << "Errors:\n" << errorStr << endl;
        returnValue_ext = EXIT_FAILURE;
        eines::signal::stopRunning_f();
        QCoreApplication::quit();
        return;
    }
    else
    {
        if (valid_pri)
        {
            QSslConfiguration sslOptions(QSslConfiguration::defaultConfiguration());
            sslOptions.setSslOption(QSsl::SslOptionDisableCompression, false);
            sslOptions.setPeerVerifyMode(QSslSocket::VerifyNone);
            eines::sslUtils_c sslUtilsTmp;
            sslOptions.setPrivateKey(QSslKey(QByteArray::fromStdString(sslUtilsTmp.privateKeyStr_f()), QSsl::Rsa));
            sslOptions.setLocalCertificate(QSslCertificate(QByteArray::fromStdString(sslUtilsTmp.keyCertificateStr_f())));

            QSslConfiguration::setDefaultConfiguration(sslOptions);

            //start the download server
            downloadServer_pri = new downloadServer_c(
                        selfServerAddress_pri
                        , downloadServerPort_pri
                        , qApp
            );
            //no need it will be destroyed with the qcoreapplication instance when the program ends
            //QObject::connect(updateServerObj, &QTcpServer::destroyed, updateServerObj, &QObject::deleteLater);

            //start the the file list server
            fileRequestServer_pri = new fileListRequestServer_c(
                        selfServerAddress_pri
                        , fileListServerPort_pri
                        , qApp
            );

//            int_fast64_t tmpCycleTimeoutMilliseconds((gcdWaitMillisecondsAll_pri / 4) - 1);
//            if (tmpCycleTimeoutMilliseconds < 1)
//            {
//                tmpCycleTimeoutMilliseconds = 1;
//            }
//            //else if the machine goes down it might not wait for the program to finish
//            if (tmpCycleTimeoutMilliseconds > 5000)
//            {
//                tmpCycleTimeoutMilliseconds = 5000;
//            }

            mainLoopTimer_pri = new QTimer(qApp);
            QObject::connect(mainLoopTimer_pri, &QTimer::timeout, std::bind(&mirrorConfig_c::mainLoop_f, &mirrorConfig_ext));

            //QOUT_TS("operationsConfig_c::initialSetup_f() tmpCycleTimeout " << tmpCycleTimeoutMilliseconds << endl);
            mainLoopTimer_pri->start(100);
        }
    }
}

void mirrorConfig_c::mainLoop_f()
{
    //QOUT_TS("mirrorConfig_c::mainLoop() start" << endl);
    if (eines::signal::isRunning_f())
    {
        if (not localScanThreadExists_pri)
        {
            threadedFunction_c* localScanThreadTmp = new threadedFunction_c(std::bind(&mirrorConfig_c::localScan_f, this), qApp);
            QObject::connect(localScanThreadTmp, &QThread::destroyed, [this]
            {
                localScanThreadExists_pri = false;
                //QOUT_TS("localScanThreadExists_pri = false" << endl);
            });
            QObject::connect(localScanThreadTmp, &QThread::finished, localScanThreadTmp, &QThread::deleteLater);

            localScanThreadTmp->start();
            //QOUT_TS("operationsConfig_c::mainLoop() starting calculateNumberAsThread" << endl);
            localScanThreadExists_pri = true;
        }

        if (notifyClientsOfUpdate_pri)
        {
            notifyClientsOfUpdate_pri = false;
            //connect to update server
            QMutexLocker mutexLockerTmpA(getAddMutex_f("recentClients"));
#ifdef DEBUGJOUVEN
            //QOUT_TS("(mirrorConfig_c::mainLoop_f) recentClients_pri.size() " <<  recentClients_pri.size() << endl);
#endif
            for (auto& clientItem_ite : recentClients_pri)
            {
                if (not eines::signal::isRunning_f())
                {
                    break;
                }
#ifdef DEBUGJOUVEN
                //QOUT_TS("(mirrorConfig_c::mainLoop_f) update client " <<  clientItem_ite.second.address_pub.toString() << " port " << clientItem_ite.second.updatePort_pub << endl);
#endif
                updateClient_c* updateClientObj = new updateClient_c(clientItem_ite.second.address_pub, clientItem_ite.second.updatePort_pub, qApp);
                QObject::connect(updateClientObj, &QTcpSocket::disconnected
                                 , [this, updateClientObj, &clientItem_ite]()
                {
                    QMutexLocker mutexLockerTmpB(getAddMutex_f("recentClients"));
                    if (updateClientObj->success_f())
                    {
                        clientItem_ite.second.failedTries_pub = 0;
                    }
                    else
                    {
                        clientItem_ite.second.failedTries_pub = clientItem_ite.second.failedTries_pub + 1;
                        //TODO make a json setting
                        if (clientItem_ite.second.failedTries_pub > 5)
                        {
                            recentClients_pri.erase(clientItem_ite.first);
                        }
                        else
                        {
                            //nothing
                        }
                    }
                });
                QObject::connect(updateClientObj, &QTcpSocket::disconnected, updateClientObj, &QObject::deleteLater);
            }
        }
        else
        {
            //TODO someday?
            //remove recent clients that aren't recent anymore
        }
    }
    else
    {
        if (eines::signal::threadCount_f() > 1 or qThreadCount_f() > 0)
        {
            //QOUT_TS("qthreads counter " << qThreadCount_f() << endl);
            //QOUT_TS("eines::signal::threadCount_f() " << eines::signal::threadCount_f() << endl);
            //change the interval and wait another cycle
            if (mainLoopTimer_pri->interval() != 10)
            {
                mainLoopTimer_pri->start(10);
                if (downloadServer_pri->isListening())
                {
                    downloadServer_pri->close();
                }
                if (fileRequestServer_pri->isListening())
                {
                    fileRequestServer_pri->close();
                }
            }
        }
        else
        {
            //QOUT_TS("qthreads counter " << QThreadCount_f() << endl);
            //QOUT_TS("eines::signal::threadCount_f() " << eines::signal::threadCount_f() << endl);
            //QOUT_TS("QCoreApplication::exit();"<< endl);
            QCoreApplication::exit();
        }
    }

    //QOUT_TS("mirrorConfig_c::mainLoop() end" << endl);
}

mirrorConfig_c mirrorConfig_ext;


