/*  Discord Webhook
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
#include <QString>
#include <QFile>
#include <QHttpMultiPart>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include "Common/Cpp/PrettyPrint.h"
//#include "Common/Cpp/PanicDump.h"
//#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "DiscordSettingsOption.h"
#include "DiscordWebhook.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Integration{
namespace DiscordWebhook{



DiscordWebhookSender::DiscordWebhookSender()
    : m_logger(global_logger_raw(), "DiscordWebhookSender")
    , m_stopping(false)
    , m_dispatcher(nullptr, 1)
    , m_queue(m_dispatcher)
{}

DiscordWebhookSender::~DiscordWebhookSender(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
        m_cv.notify_all();
    }
}

DiscordWebhookSender& DiscordWebhookSender::instance(){
    static DiscordWebhookSender sender;
    return sender;
}

void DiscordWebhookSender::send_json(
    Logger& logger,
    const QUrl& url, std::chrono::milliseconds delay,
    const JsonObject& obj,
    std::shared_ptr<PendingFileSend> file
){
    cleanup_stuck_requests();
    m_queue.add_event(
        delay,
        [this, url, data = QByteArray::fromStdString(obj.dump()), file = std::move(file)]{
            throttle();
            if (!file && !data.isEmpty()){
                internal_send_json(url, data);
            }else if (file && !data.isEmpty()){
                internal_send_image_embed(url, data, file->filepath(), file->filename());
            }else{
                internal_send_file(url, file->filepath());
            }
        }
    );
    logger.log("Scheduling Webhook Message... (queue = " + tostr_u_commas(m_queue.size()) + ")", COLOR_PURPLE);
}

void DiscordWebhookSender::send_file(
    Logger& logger,
    const QUrl& url, std::chrono::milliseconds delay,
    std::shared_ptr<PendingFileSend> file
){
    cleanup_stuck_requests();
//    m_queue.emplace_back(url, file);
    m_queue.add_event(
        delay,
        [this, url, file = std::move(file)]{
            throttle();
            internal_send_file(url, file->filepath());
        }
    );
    logger.log("Scheduling Webhook Message... (queue = " + tostr_u_commas(m_queue.size()) + ")", COLOR_PURPLE);
}

void DiscordWebhookSender::cleanup_stuck_requests(){
    std::lock_guard<std::mutex> lg(m_lock);
    WallClock next = m_queue.next_event();
    if (next == WallClock::max()){
        return;
    }

    WallClock now = current_time();
    WallClock threshold = now - std::chrono::seconds(60);
    if (next < threshold){
        m_logger.log("Purging request that appears to be stuck.", COLOR_RED);
        emit stop_event_loop();
    }
}
void DiscordWebhookSender::throttle(){
    //  Throttle the messages.
    auto duration = THROTTLE_DURATION;
    auto now = current_time();
    while (!m_sent.empty() && m_sent[0] + duration < now){
        m_sent.pop_front();
    }
    if (m_sent.size() >= GlobalSettings::instance().DISCORD.webhooks.sends_per_second){
        m_logger.log("Throttling webhook messages due to rate limit...", COLOR_RED);
        std::unique_lock<std::mutex> lg(m_lock);
        m_cv.wait_for(
            lg, duration,
            [&]{ return m_sent[0] + duration < now || m_stopping; }
        );
        if (m_stopping){
            return;
        }
        m_sent.clear();
    }
    m_sent.push_back(now);
}


void DiscordWebhookSender::process_reply(QNetworkReply* reply){
    if (!reply){
        m_logger.log("QNetworkReply is null.", COLOR_RED);
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
        m_logger.log("Discord Request Response: " + error_string.toStdString(), COLOR_RED);
//        QString err = reply->errorString();
//        qDebug() << err;
    }
}

void DiscordWebhookSender::internal_send_json(const QUrl& url, const QByteArray& data){
    QEventLoop event_loop;
    connect(
        this, &DiscordWebhookSender::stop_event_loop,
        &event_loop, &QEventLoop::quit
    );

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkAccessManager manager;
    event_loop.connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
    m_logger.log("Sending Webhook Message...", COLOR_BLUE);
    std::unique_ptr<QNetworkReply> reply(manager.post(request, data));
    event_loop.exec();
    process_reply(reply.get());
}

void DiscordWebhookSender::internal_send_file(const QUrl& url, const std::string& filename){
    QEventLoop event_loop;
    connect(
        this, &DiscordWebhookSender::stop_event_loop,
        &event_loop, &QEventLoop::quit
    );

    QFile file(QString::fromStdString(filename));
    if (!file.open(QIODevice::ReadOnly)){
        m_logger.log("File doesn't exist: " + filename, COLOR_RED);
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
    event_loop.connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
    m_logger.log("Sending Webhook Message...", COLOR_BLUE);
    std::unique_ptr<QNetworkReply> reply(manager.post(request, &multiPart));
    event_loop.exec();
    process_reply(reply.get());
}

void DiscordWebhookSender::internal_send_image_embed(const QUrl& url, const QByteArray& data, const std::string& filepath, const std::string& filename){
    QEventLoop event_loop;
    connect(
        this, &DiscordWebhookSender::stop_event_loop,
        &event_loop, &QEventLoop::quit
    );

    QFile file(QString::fromStdString(filepath));
    if (!file.open(QIODevice::ReadOnly)){
        m_logger.log("File doesn't exist: " + filepath, COLOR_RED);
        return;
    }

    QNetworkRequest request(url);

    QHttpPart imagePart;
    imagePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant("application/octet-stream; name=file0; filename=" + QString::fromStdString(filename))
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
    event_loop.connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
    m_logger.log("Sending Webhook Message...", COLOR_BLUE);
    std::unique_ptr<QNetworkReply> reply(manager.post(request, &multiPart));
    event_loop.exec();
    process_reply(reply.get());
}




void send_embed(
    Logger& logger,
    bool should_ping,
    const std::vector<std::string>& tags,
    const JsonArray& embeds,
    std::shared_ptr<PendingFileSend> file
){
    DiscordSettingsOption& settings = GlobalSettings::instance().DISCORD;
    if (!settings.webhooks.enabled()){
        return;
    }

    MessageBuilder builder(tags);

    std::vector<std::unique_ptr<DiscordWebhookUrl>> list = settings.webhooks.urls.copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const DiscordWebhookUrl& url = *list[c];
        if (!url.enabled || ((std::string)url.url).empty()){
            continue;
        }
        if (!builder.should_send(EventNotificationOption::parse_tags(url.tags_text))){
            continue;
        }

        JsonObject json;
        json["content"] = builder.build_message(
            should_ping && url.ping,
            settings.message.user_id,
            settings.message.message
        );
        if (!embeds.empty()){
            json["embeds"] = embeds.clone();
        }

        DiscordWebhookSender::instance().send_json(
            logger,
            QString::fromStdString(url.url),
            std::chrono::seconds(url.delay),
            std::move(json),
            file
        );
    }
}




}
}
}








