/*  File Downloader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <memory>
#include <QObject>
#include <QByteArray>
#include <QEventLoop>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "FileDownloader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace FileDownloader{

std::string download_file(Logger& logger, const std::string& url){
    QNetworkAccessManager network_access_manager;
    QByteArray downloaded_data;
    std::unique_ptr<QNetworkReply> reply;
    QEventLoop loop;

    QObject::connect(
        &network_access_manager, &QNetworkAccessManager::finished,
        &loop, [&downloaded_data, &loop](QNetworkReply* reply){
            downloaded_data = reply->readAll();
//            reply->deleteLater();
            loop.exit();
        }
    );

    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    reply.reset(network_access_manager.get(request));

#if 0
    QTimer timer;
    timer.singleShot(
        5000, [&downloaded_data, &url](){
        if (downloaded_data.isEmpty()){
            throw ConnectionException(nullptr, "Couldn't retrieved file from url : " + url + " after 5s");
        }
    });
    timer.stop();
    cout << "Stop timer" << endl;
#endif

    loop.exec();

    if (!reply){
        std::string str = "QNetworkReply is null.";
//        logger.log(str, COLOR_RED);
        throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, str);
    }else if (reply->error() == QNetworkReply::NoError){
//        QString contents = QString::fromUtf8(reply->readAll());
//        qDebug() << contents;
    }else{
        QString error_string = reply->errorString();
        std::string str = "Network Error: " + error_string.toStdString();
//        logger.log(str, COLOR_RED);
        throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, str);
//        QString err = reply->errorString();
//        qDebug() << err;
    }

    return std::string(downloaded_data.data(), downloaded_data.size());
}
JsonValue download_json_file(Logger& logger, const std::string& url){
    std::string downloaded_data = download_file(logger, url);
    return parse_json(downloaded_data);
}





}
}
