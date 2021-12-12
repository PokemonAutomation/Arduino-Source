/*  Discord WebHook
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
#include <QFile>
#include <QHttpMultiPart>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QString>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Tools/Logger.h"
#include "DiscordWebHook.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Integration{
namespace DiscordWebHook{


DiscordWebHookRequest::DiscordWebHookRequest(QUrl p_url, QByteArray p_data, std::shared_ptr<PendingFileSend> p_file)
    : url(std::move(p_url))
    , data(std::move(p_data))
    , file(std::move(p_file))
{}

DiscordWebHookRequest::DiscordWebHookRequest( QUrl p_url, std::shared_ptr<PendingFileSend> p_file)
    : url(std::move(p_url))
    , file(std::move(p_file))
{}


DiscordWebHookSender::DiscordWebHookSender()
    : m_logger(global_logger_raw(), "DiscordWebHookSender")
    , m_stopping(false)
    , m_thread(&DiscordWebHookSender::thread_loop, this)
{}

DiscordWebHookSender::~DiscordWebHookSender(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
        m_cv.notify_all();
    }
    m_thread.join();
}

DiscordWebHookSender& DiscordWebHookSender::instance(){
    static DiscordWebHookSender sender;
    return sender;
}

void DiscordWebHookSender::send_json(Logger& logger, const QUrl& url, const QJsonObject& obj, std::shared_ptr<PendingFileSend> file){
    std::lock_guard<std::mutex> lg(m_lock);
    m_queue.emplace_back(
        url,
        QJsonDocument(obj).toJson(),
        file
    );
    logger.log("Sending JSON to Discord... (queue = " + tostr_u_commas(m_queue.size()) + ")", "purple");
    m_cv.notify_all();
}

void DiscordWebHookSender::send_file(Logger& logger, const QUrl& url, std::shared_ptr<PendingFileSend> file){
    std::lock_guard<std::mutex> lg(m_lock);
    m_queue.emplace_back(url, file);
    logger.log("Sending File to Discord... (queue = " + tostr_u_commas(m_queue.size()) + ")", "purple");
    m_cv.notify_all();
}

void DiscordWebHookSender::thread_loop(){
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

            //  Throttle the messages.
            auto duration = THROTTLE_DURATION;
            auto now = std::chrono::system_clock::now();
            while (!m_sent.empty() && m_sent[0] + duration < now){
                m_sent.pop_front();
            }
            if (m_sent.size() >= MAX_IN_WINDOW){
                m_cv.wait_for(
                    lg, duration,
                    [&]{
                        return m_sent[0] + duration < now;
                    }
                );
                m_sent.clear();
            }
            m_sent.push_back(now);
        }

        if (!item.file && !item.data.isEmpty()){
            internal_send_json(item.url, item.data);
        }else if (item.file && !item.data.isEmpty()){
            internal_send_image_embed(item.url, item.data, item.file->filepath(), item.file->filename());
        }else{
            internal_send_file(item.url, item.file->filepath());
        }
    }
}

void DiscordWebHookSender::process_reply(QNetworkReply* reply){
    if (!reply){
        m_logger.log("QNetworkReply is null.", "red");
    }else if (reply->error() == QNetworkReply::NoError){
//        QString contents = QString::fromUtf8(reply->readAll());
//        qDebug() << contents;
    }else{
        QString error_string = reply->errorString();
        QString url = reply->url().toString();
        int index = error_string.indexOf(url);
        if (index >= 0){
            error_string.replace(index, url.size(), "****************");
        }
        m_logger.log("Discord Request Response: " + error_string, "red");
//        QString err = reply->errorString();
//        qDebug() << err;
    }
}

void DiscordWebHookSender::internal_send_json(const QUrl& url, const QByteArray& data){
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkAccessManager manager;
    QEventLoop loop;
    loop.connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
    std::unique_ptr<QNetworkReply> reply(manager.post(request, data));
    loop.exec();
    process_reply(reply.get());
}

void DiscordWebHookSender::internal_send_file(const QUrl& url, const QString& filename){
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)){
        m_logger.log("File doesn't exist: " + filename, "red");
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
    process_reply(reply.get());
}

void DiscordWebHookSender::internal_send_image_embed(const QUrl& url, const QByteArray& data, const QString& filepath, const QString& filename){
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)){
        m_logger.log("File doesn't exist: " + filepath, "red");
        return;
    }

    QNetworkRequest request(url);

    QHttpPart imagePart;
    imagePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant("application/octet-stream; name=file0; filename=" + filename)
    );
    imagePart.setBodyDevice(&file);

    QHttpPart jsonPart;
    jsonPart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant("form-data; name=payload_json")
    );
    jsonPart.setBody(data);

    QHttpMultiPart multiPart(QHttpMultiPart::FormDataType);
    multiPart.append(imagePart);
    multiPart.append(jsonPart);

    QNetworkAccessManager manager;
    QEventLoop loop;
    loop.connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
    std::unique_ptr<QNetworkReply> reply(manager.post(request, &multiPart));
    loop.exec();
    process_reply(reply.get());
}

void send_message(
    Logger& logger,
    bool should_ping,
    const std::vector<QString>& tags,
    const QString& message,
    const QJsonArray& embeds,
    std::shared_ptr<PendingFileSend> file
){
    DiscordSettingsOption& settings = GlobalSettings::instance().DISCORD;
    if (!settings.webhooks.enabled()){
        return;
    }

    std::set<QString> tag_set;
    for (const QString& tag : tags){
        tag_set.insert(tag.toLower());
    }

    for (size_t c = 0; c < settings.webhooks.urls.size(); c++){
        const DiscordWebhookUrl& url = static_cast<const DiscordWebhookUrl&>(settings.webhooks.urls[c]);
        if (!url.enabled){
            continue;
        }
        if (url.url.isEmpty()){
            continue;
        }
//        cout << url.url.toStdString() << " : tags = " << url.tags.size() << endl;
//        for (const QString& tag : tag_set){
//            cout << "event tag: " << tag.toStdString() << endl;
//        }

        //  See if a tag matches.
        bool send = false;
        for (const QString& tag : url.tags){
//            cout << "find tag: " << tag.toStdString() << endl;
            auto iter = tag_set.find(tag.toLower());
            if (iter != tag_set.end()){
//                cout << "found" << endl;
                send = true;
                break;
            }
        }
        if (!send){
            continue;
        }

        //  Sanitize user ID.
        if (settings.message.user_id.get().toULongLong() == 0){
            should_ping = false;
        }

        //  Message
        QString str;
        if (should_ping && url.ping){
            str += "<@" + settings.message.user_id + ">";
        }

        const QString& discord_message = settings.message.message;
        if (!discord_message.isEmpty()){
            if (!str.isEmpty()){
                str += " ";
            }
            for (QChar ch : discord_message){
                if (ch != '@'){
                    str += ch;
                }
            }
//            str += discord_message;
        }
        if (!message.isEmpty()){
            if (!str.isEmpty()){
                str += " ";
            }
            str += message;
        }

        QJsonObject jsonContent;
        jsonContent["content"] = str;
        jsonContent["embeds"] = embeds;

//        cout << QJsonDocument(jsonContent).toJson().data() << endl;

        DiscordWebHookSender::instance().send_json(logger, url.url, jsonContent, file);
    }
}




}
}
}








