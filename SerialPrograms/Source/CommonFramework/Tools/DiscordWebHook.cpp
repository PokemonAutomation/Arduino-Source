/*  Discord WebHook
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
//#include <set>
#include <condition_variable>
#include <thread>
#include <QFile>
#include <QHttpMultiPart>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/Logger.h"
#include "DiscordWebHook.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace DiscordWebHook{


struct DiscordWebHookRequest{
    DiscordWebHookRequest() = default;
    DiscordWebHookRequest(Logger* p_logger, QUrl p_url, QByteArray p_data)
        : logger(p_logger)
        , url(std::move(p_url))
        , data(std::move(p_data))
    {}
    DiscordWebHookRequest(Logger* p_logger, QUrl p_url, QString p_file)
        : logger(p_logger)
        , url(std::move(p_url))
        , file(std::move(p_file))
    {}
    Logger* logger;
    QUrl url;

    QByteArray data;
    QString file;
};


class DiscordWebHookSender : public QObject{
private:
    DiscordWebHookSender()
        : m_stopping(false)
        , m_thread(&DiscordWebHookSender::thread_loop, this)
    {}
    ~DiscordWebHookSender(){
        {
            std::lock_guard<std::mutex> lg(m_lock);
            m_stopping = true;
            m_cv.notify_all();
        }
        m_thread.join();
    }


public:
    void send_json(Logger* logger, const QUrl& url, const QJsonObject& obj){
        std::lock_guard<std::mutex> lg(m_lock);
        m_queue.emplace_back(
            logger,
            url,
            QJsonDocument(obj).toJson()
        );
        if (logger){
            logger->log("Sending JSON to Discord... (queue = " + tostr_u_commas(m_queue.size()) + ")", "purple");
        }
        m_cv.notify_all();
    }
    void send_file(Logger* logger, const QUrl& url, QString file){
        std::lock_guard<std::mutex> lg(m_lock);
        m_queue.emplace_back(
            logger,
            url,
            std::move(file)
        );
        if (logger){
            logger->log("Sending File to Discord... (queue = " + tostr_u_commas(m_queue.size()) + ")", "purple");
        }
        m_cv.notify_all();
    }

    static DiscordWebHookSender& instance(){
        static DiscordWebHookSender sender;
        return sender;
    }


private:
    void thread_loop(){
        while (true){
            DiscordWebHookRequest item;
            {
                std::unique_lock<std::mutex> lg(m_lock);
                if (m_stopping){
                    break;
                }
                if (m_queue.empty()){
                    m_cv.wait(lg);
                    continue;
                }

                item = std::move(m_queue.front());
                m_queue.pop_front();
            }

            if (item.file.isNull()){
                internal_send_json(item.logger, item.url, item.data);
            }else{
                internal_send_file(item.logger, item.url, item.file);
            }
        }
    }

    void process_reply(Logger* logger, QNetworkReply* reply){
        if (!reply){
            if (logger){
                logger->log("QNetworkReply is null.", "red");
            }
        }else if (reply->error() == QNetworkReply::NoError){
//            QString contents = QString::fromUtf8(reply->readAll());
//            qDebug() << contents;
        }else{
            if (logger){
                logger->log("Discord Request Response: " + reply->errorString(), "red");
            }
//            QString err = reply->errorString();
//            qDebug() << err;
        }
    }
    void internal_send_json(Logger* logger, const QUrl& url, const QByteArray& data){
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkAccessManager manager;
        QEventLoop loop;
        loop.connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
        std::unique_ptr<QNetworkReply> reply(manager.post(request, data));
        loop.exec();
        process_reply(logger, reply.get());
    }
    void internal_send_file(Logger* logger, const QUrl& url, const QString& filename){
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)){
            if (logger){
                logger->log("File doesn't exist: " + filename, "red");
            }
            return;
        }

        QNetworkRequest request(url);

        QHttpPart imagePart;
        imagePart.setHeader(
            QNetworkRequest::ContentDispositionHeader,
            QVariant("form-data; name=\"file1\"; filename=\"" + file.fileName() + "\"")
        );
        imagePart.setBodyDevice(&file);

        QHttpMultiPart multiPart(QHttpMultiPart::FormDataType);
        multiPart.append(imagePart);

        QNetworkAccessManager manager;
        QEventLoop loop;
        loop.connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
        std::unique_ptr<QNetworkReply> reply(manager.post(request, &multiPart));
        loop.exec();
        process_reply(logger, reply.get());
    }


private:
    bool m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;
    std::deque<DiscordWebHookRequest> m_queue;
    std::thread m_thread;
};



void send_message(bool should_ping, const QString& message, const QJsonObject& embed, Logger* logger){
    QJsonArray embeds;
    embeds.append(embed);
    send_message(should_ping, message, embeds, logger);
}
void send_message(bool should_ping, const QString& message, const QJsonArray& embeds, Logger* logger){
    QString webhook_url = PERSISTENT_SETTINGS().DISCORD_WEBHOOK_URL;
    if (webhook_url.isEmpty()){
        return;
    }

    //  customize the message
    QString name = should_ping
        ? "<@" + PERSISTENT_SETTINGS().DISCORD_USER_ID + "> "
        : PERSISTENT_SETTINGS().DISCORD_USER_SHORT_NAME + " ";

    QJsonObject jsonContent;
    jsonContent["content"] = name + message;
    jsonContent["embeds"] = embeds;

    DiscordWebHookSender::instance().send_json(
        logger,
        webhook_url,
        jsonContent
    );
}
void send_message_old(bool should_ping, const QString& message, const QJsonArray& fields) {
    if (PERSISTENT_SETTINGS().DISCORD_WEBHOOK_URL.isEmpty()){
        return;
    }

    //customize the message
    QString name = should_ping ? "<@" + PERSISTENT_SETTINGS().DISCORD_USER_ID + "> " : PERSISTENT_SETTINGS().DISCORD_USER_SHORT_NAME + " ";

    //create the embeds holding the data to send
    QJsonObject embeds;
    embeds["title"] = "Serial Programs Update";
    embeds["color"] = 0;
    embeds["author"] = QJsonObject();
    embeds["image"] = QJsonObject();
    embeds["thumbnail"] = QJsonObject();
    embeds["footer"] = QJsonObject();
    embeds["fields"] = fields;

    QJsonArray embedsArray;
    embedsArray.append(embeds);

    send_message(should_ping, message, embedsArray, nullptr);
}
void send_file(QString file, Logger* logger){
    QString webhook_url = PERSISTENT_SETTINGS().DISCORD_WEBHOOK_URL;
    if (webhook_url.isEmpty()){
        return;
    }

    DiscordWebHookSender::instance().send_file(
        logger,
        webhook_url,
//        QString("https://discord.requestcatcher.com/test"),
        std::move(file)
    );
}


}
}
