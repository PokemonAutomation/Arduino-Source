/*  File Downloader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <memory>
#include <QObject>
#include <QByteArray>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QSslError>
#include "Common/Cpp/Filesystem/FileIO.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "FileDownloader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace FileDownloader{

std::string download_file(Logger& logger, const std::string& url, std::chrono::seconds overall_timeout){
//    cout << "download_file()" << endl;
    QNetworkAccessManager network_access_manager;
    QByteArray downloaded_data;
    std::unique_ptr<QNetworkReply> reply;
    QEventLoop loop;
    QList<QSslError> ssl_errors;

    QObject::connect(
        &network_access_manager, &QNetworkAccessManager::finished,
        &loop, [&downloaded_data, &loop](QNetworkReply* reply){
//            cout << "QNetworkAccessManager::finished" << endl;
            //  Only read on success: reading from a reply whose connection
            //  already failed logs a spurious "QIODevice::read ... device
            //  not open" warning.
            if (reply->error() == QNetworkReply::NoError){
                downloaded_data = reply->readAll();
            }
//            reply->deleteLater();
            loop.exit();
        }
    );
    QObject::connect(
        &network_access_manager, &QNetworkAccessManager::sslErrors,
        &loop, [&ssl_errors, &loop](QNetworkReply* reply, const QList<QSslError>& errors){
//            cout << "QNetworkAccessManager::sslErrors" << endl;
            //  Record the SSL errors and bail out; the handshake has already
            //  failed, so don't read from the reply. Note: at this point the
            //  reply is still marked as running, so this must be reported as
            //  an SSL error (see below), not as a timeout.
            ssl_errors = errors;
//            reply->deleteLater();
            loop.exit();
        }
    );

    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    request.setTransferTimeout(std::chrono::seconds(5));
    reply.reset(network_access_manager.get(request));

    //  Deadline for the whole request, including the connection phase
    //  (setTransferTimeout() above only limits data transfer). Without
    //  this, a connection that stalls would block loop.exec() forever.
    QTimer overall_timer;
    if (overall_timeout.count() > 0){
        overall_timer.setSingleShot(true);
        QObject::connect(
            &overall_timer, &QTimer::timeout,
            &loop, [&loop]{ loop.exit(); }
        );
        overall_timer.start(
            (int)std::chrono::duration_cast<std::chrono::milliseconds>(overall_timeout).count()
        );
    }

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
    overall_timer.stop();

    if (!reply){
        std::string str = "QNetworkReply is null.";
//        logger.log(str, COLOR_RED);
        throw_and_log<OperationFailedException>(logger, ErrorReportMode::NO_ERROR_REPORT, str);
    }else if (!ssl_errors.isEmpty()){
        //  The TLS handshake failed (missing or expired certificate,
        //  untrusted root CA, MITM proxy, ...). This branch must come
        //  before the isRunning() check below: when sslErrors() fires,
        //  the reply is still marked as running, so without this check an
        //  SSL failure would be misreported as a timeout.
        reply->abort();
        std::string str = "SSL error downloading " + url + ": ";
        for (int i = 0; i < ssl_errors.size(); i++){
            if (i != 0){
                str += "; ";
            }
            str += ssl_errors.at(i).errorString().toStdString();
        }
        throw_and_log<OperationFailedException>(logger, ErrorReportMode::NO_ERROR_REPORT, str);
    }else if (overall_timeout.count() > 0 && reply->isRunning()){
        //  The overall deadline elapsed while the request (typically the
        //  connection) was still in progress.
        reply->abort();
        throw_and_log<OperationFailedException>(logger, ErrorReportMode::NO_ERROR_REPORT,
            "Timed out after " + std::to_string(overall_timeout.count()) + " seconds downloading " + url + ".");
    }else if (reply->error() == QNetworkReply::NoError){
//        QString contents = QString::fromUtf8(reply->readAll());
//        qDebug() << contents;
    }else{
        QString error_string = reply->errorString();
        std::string str = "Network Error: " + error_string.toStdString();
//        logger.log(str, COLOR_RED);
        throw_and_log<OperationFailedException>(logger, ErrorReportMode::NO_ERROR_REPORT, str);
//        QString err = reply->errorString();
//        qDebug() << err;
    }

    return std::string(downloaded_data.data(), downloaded_data.size());
}

void download_file_to_disk(
    CancellableScope& scope,
    Logger& logger, 
    const std::string& url, 
    const std::string& file_path,
    uint64_t expected_size,
    std::function<void(uint64_t bytes_done, uint64_t total_bytes)> progress_callback
){
//    cout << "download_file()" << endl;
    QNetworkAccessManager network_access_manager;
    QEventLoop loop;

    //  ensure the directory exists
    const Filesystem::Path save_path(file_path);
    Filesystem::create_directories(save_path.parent_path());

    //  1. Write to a unique temporary file in the same directory, then
    //     rename it onto the destination on success (atomic save).
    const Filesystem::Path save_temp_path = Filesystem::unique_temp_path(save_path);
    FileIO file(save_temp_path, FileMode::WRITE | FileMode::BINARY);
    if (!file.is_open()){
        throw_and_log<OperationFailedException>(logger, ErrorReportMode::NO_ERROR_REPORT, 
            "Could not open save file: " + file_path);
    }

    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    // request.setAttribute(QNetworkRequest::AutoRedirectionPolicyAttribute, true);  // enable auto-redirects
    request.setTransferTimeout(std::chrono::seconds(5));
    
    // 2. Start the GET request
    QNetworkReply* reply = network_access_manager.get(request);

    // Progress Bar Logic. and check for Cancel
    QObject::connect(reply, &QNetworkReply::downloadProgress, 
        [reply, &scope, expected_size, progress_callback](qint64 bytesReceived, qint64 bytesTotal){

            if (scope.cancelled()){
                reply->abort();
            }
            
            // Use expected_size if the network doesn't provide one
            uint64_t total = (bytesTotal > 0) ? (uint64_t)bytesTotal : expected_size;

            progress_callback(bytesReceived, total);
        }
    );

    // 3. Stream chunks directly to the temporary file
    QObject::connect(reply, &QNetworkReply::readyRead, [&file, reply](){
        QByteArray data = reply->readAll();
        file.write(data.constData(), data.size());
    });

    // 4. Handle completion and errors
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    
    // Start the loop. local wait mechanism that pauses execution of the function 
    // while Qt handles the network request. 
    // the loop stops once we see the signal QNetworkReply::finished.
    loop.exec();

    // // Final check for remaining data
    if (reply->bytesAvailable() > 0){
        QByteArray data = reply->readAll();
        file.write(data.constData(), data.size());
    }
    file.close();

    // 5. Finalize the transaction
    if (reply->error() == QNetworkReply::NoError){
        //  This moves the temporary file to the final destination 'file_path'
        std::error_code ec;
        Filesystem::rename(save_temp_path, save_path, ec);
        if (ec){
            Filesystem::remove(save_temp_path);
            throw_and_log<OperationFailedException>(logger, ErrorReportMode::NO_ERROR_REPORT, 
                "Failed to commit file to disk: " + file_path);
        }
    }else{
        Filesystem::remove(save_temp_path);
        if (scope.cancelled()){
            logger.log("Download cancelled by user.");
            throw OperationCancelledException();
        }else{
            QString error_string = reply->errorString();
            throw_and_log<OperationFailedException>(logger, ErrorReportMode::NO_ERROR_REPORT, 
                "Network Error: " + error_string.toStdString());
        }
    }

    reply->deleteLater();
}

JsonValue download_json_file(Logger& logger, const std::string& url, std::chrono::seconds overall_timeout){
    std::string downloaded_data = download_file(logger, url, overall_timeout);
    return parse_json(downloaded_data);
}





}
}
