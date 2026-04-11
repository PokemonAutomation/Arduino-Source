/*  File Downloader
 *
 *  From: https://github.com/PokemonAutomation/
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
#include <QSaveFile>
#include <QFileInfo>
#include <QDir>
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "FileDownloader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace FileDownloader{

std::string download_file(Logger& logger, const std::string& url){
//    cout << "download_file()" << endl;
    QNetworkAccessManager network_access_manager;
    QByteArray downloaded_data;
    std::unique_ptr<QNetworkReply> reply;
    QEventLoop loop;

    QObject::connect(
        &network_access_manager, &QNetworkAccessManager::finished,
        &loop, [&downloaded_data, &loop](QNetworkReply* reply){
//            cout << "QNetworkAccessManager::finished" << endl;
            downloaded_data = reply->readAll();
//            reply->deleteLater();
            loop.exit();
        }
    );
    QObject::connect(
        &network_access_manager, &QNetworkAccessManager::sslErrors,
        &loop, [&downloaded_data, &loop](QNetworkReply* reply){
//            cout << "QNetworkAccessManager::sslErrors" << endl;
            downloaded_data = reply->readAll();
//            reply->deleteLater();
            loop.exit();
        }
    );

    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    request.setTransferTimeout(std::chrono::seconds(5));
    reply.reset(network_access_manager.get(request));

#if 0
    QTimer timer;
    timer.singleShot(
        5000, [&downloaded_data, &url](){
        if (downloaded_data.isEmpty()){
            throw ConnectionException(nullptr, "Couldn't retrieved file from url : " + url + " after 5 seconds.");
        }
    });
    timer.stop();
    cout << "Stop timer" << endl;
#endif

//    cout << "loop.exec() - enter" << endl;
    loop.exec();
//    cout << "loop.exec() - exit" << endl;

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

void download_file_to_disk(
    Logger& logger, 
    const std::string& url, 
    const std::string& file_path,
    qint64 expected_size,
    std::function<void(int)> progress_callback,
    std::function<bool()> is_cancelled
){
//    cout << "download_file()" << endl;
    QNetworkAccessManager network_access_manager;
    QEventLoop loop;

    // ensure the directory exists
    QString filePath = QString::fromStdString(file_path);
    QFileInfo fileInfo(filePath);
    QString dirPath = fileInfo.absolutePath();
    QDir().mkpath(dirPath);
    
    // 1. Initialize QSaveFile
    QSaveFile file(QString::fromStdString(file_path));
    if (!file.open(QIODevice::WriteOnly)) {
        throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
            "Could not open save file: " + file_path);
    }

    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    // request.setAttribute(QNetworkRequest::AutoRedirectionPolicyAttribute, true);  // enable auto-redirects
    request.setTransferTimeout(std::chrono::seconds(5));
    
    // 2. Start the GET request
    QNetworkReply* reply = network_access_manager.get(request);

    // Progress Bar Logic. and check for Cancel
    QObject::connect(reply, &QNetworkReply::downloadProgress, 
        [reply, expected_size, progress_callback, is_cancelled](qint64 bytesReceived, qint64 bytesTotal) {

            if (is_cancelled()){
                reply->abort();
            }
            
            // Use expected_size if the network doesn't provide one
            qint64 total = (bytesTotal > 0) ? bytesTotal : expected_size;

            int percentage_progress = (total > 0) ? static_cast<int>((bytesReceived * 100) / total) : 0;
            progress_callback(std::min(percentage_progress, 100));
        }
    );

    // 3. Stream chunks directly to the temporary file
    QObject::connect(reply, &QNetworkReply::readyRead, [&file, reply]() {
        file.write(reply->readAll());
    });

    // 4. Handle completion and errors
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    
    // Start the loop. local wait mechanism that pauses execution of the function 
    // while Qt handles the network request. 
    // the loop stops once we see the signal QNetworkReply::finished.
    loop.exec();

    // // Final check for remaining data
    if (reply->bytesAvailable() > 0) {
        file.write(reply->readAll());
    }

    // 5. Finalize the transaction
    if (reply->error() == QNetworkReply::NoError) {
        // This moves the temporary file to the final destination 'file_path'
        if (!file.commit()) {
            throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
                "Failed to commit file to disk: " + file_path);
        }
    } else {
        if (is_cancelled()){
            logger.log("Download cancelled by user.");
            throw OperationCancelledException();
        }else{
            QString error_string = reply->errorString();
            // QSaveFile automatically deletes the temp file if commit() isn't called
            throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
                "Network Error: " + error_string.toStdString());
        }
    }

    reply->deleteLater();
}

JsonValue download_json_file(Logger& logger, const std::string& url){
    std::string downloaded_data = download_file(logger, url);
    return parse_json(downloaded_data);
}





}
}
