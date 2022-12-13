/*  File Downloader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QObject>
#include <QByteArray>
#include <QEventLoop>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>

#include "Common/Cpp/Exceptions.h"

#include "FileDownloader.h"

namespace PokemonAutomation{
namespace FileDownloader{

std::string download_json_file(const std::string& url){
	QNetworkAccessManager network_access_manager;
	QByteArray downloaded_data;
	QEventLoop loop;

	QObject::connect(&network_access_manager, &QNetworkAccessManager::finished,
		[&downloaded_data, &loop](QNetworkReply* reply){
			downloaded_data = reply->readAll();
			reply->deleteLater();
			loop.exit();
		});

	QNetworkRequest request(QUrl(QString::fromStdString(url)));
	network_access_manager.get(request);

	QTimer timer;
	timer.singleShot(5000, [&downloaded_data, &url](){
		if (downloaded_data.isEmpty()){
			throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "Couldn't retrieved file from url : " + url + " after 5s");
		}
	});
	loop.exec();

	QJsonDocument json_doc = QJsonDocument::fromJson(downloaded_data);
	QString json_string = json_doc.toJson();

	if (json_string.isEmpty() || json_string.isNull()){
		throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "Retrieved file from url : " + url + " is invalid");
	}

	return json_string.toStdString();
}

}
}
