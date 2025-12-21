/*  Discord Webhook
 *
 *  From: https://github.com/PokemonAutomation/
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
    m_stopping.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_cv.notify_all();
    }
    std::lock_guard<std::mutex> lg(m_send_lock);
    if (m_event_loop){
        m_event_loop->exit();
    }
}

DiscordWebhookSender& DiscordWebhookSender::instance(){
    static DiscordWebhookSender sender;
    return sender;
}

void DiscordWebhookSender::send(
    Logger& logger,
    const QUrl& url, std::chrono::milliseconds delay,
    const JsonObject& obj,
    std::shared_ptr<PendingFileSend> file,
    std::function<void()> finish_callback
){
    cleanup_stuck_requests();
    std::shared_ptr<JsonValue> json(new JsonValue(obj.clone()));
//    cout << "Scheduling Webhook Message... (queue = " + tostr_u_commas(m_queue.size()) + ")" << endl;
    logger.log("Scheduling Webhook Message... (queue = " + tostr_u_commas(m_queue.size()) + ")", COLOR_PURPLE);
    m_queue.add_event(
        delay,
        [
            this, url,
            json = std::move(json),
            file = std::move(file),
            finish_callback = std::move(finish_callback)
        ]{
            throttle();
            std::vector<DiscordFileAttachment> attachments;
            if (file){
                attachments.emplace_back(
                    DiscordFileAttachment{file->filename(), file->filepath()}
                );
            }
            internal_send(url, *json, attachments);
            if (finish_callback){
                finish_callback();
            }
        }
    );
}
void DiscordWebhookSender::send(
    Logger& logger,
    const QUrl& url, std::chrono::milliseconds delay,
    const JsonObject& obj,
    std::vector<std::shared_ptr<PendingFileSend>> files,
    std::function<void()> finish_callback
){
    cleanup_stuck_requests();
    std::shared_ptr<JsonValue> json(new JsonValue(obj.clone()));
//    cout << "Scheduling Webhook Message... (queue = " + tostr_u_commas(m_queue.size()) + ")" << endl;
    logger.log("Scheduling Webhook Message... (queue = " + tostr_u_commas(m_queue.size()) + ")", COLOR_PURPLE);
    m_queue.add_event(
        delay,
        [
            this, url,
            json = std::move(json),
            files = std::move(files),
            finish_callback = std::move(finish_callback)
        ]{
            throttle();
            std::vector<DiscordFileAttachment> attachments;
            for (auto& file : files){
                attachments.emplace_back(
                    DiscordFileAttachment{file->filename(), file->filepath()}
                );
            }
            internal_send(url, *json, attachments);
            if (finish_callback){
                finish_callback();
            }
        }
    );
}

void DiscordWebhookSender::cleanup_stuck_requests(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        WallClock next = m_queue.next_event();
        if (next == WallClock::max()){
            return;
        }

        WallClock now = current_time();
        WallClock threshold = now - std::chrono::seconds(60);
        if (next >= threshold){
            return;
        }
    }

    m_logger.log("Purging request that appears to be stuck.", COLOR_RED);
    std::lock_guard<std::mutex> lg(m_send_lock);
    if (m_event_loop){
        m_event_loop->exit();
    }
}
void DiscordWebhookSender::throttle(){
    //  Throttle the messages.
    auto duration = THROTTLE_DURATION;
    auto now = current_time();
    while (!m_sent.empty() && m_sent[0] + duration < now){
        m_sent.pop_front();
    }
    if (!m_sent.empty() && m_sent.size() >= GlobalSettings::instance().DISCORD->webhooks.sends_per_second){
        m_logger.log("Throttling webhook messages due to rate limit...", COLOR_RED);
        std::unique_lock<std::mutex> lg(m_lock);
        m_cv.wait_for(
            lg, duration,
            [&]{ return m_stopping.load(std::memory_order_relaxed) || m_sent.empty() || m_sent[0] + duration < now; }
        );
        if (m_stopping.load(std::memory_order_relaxed)){
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
        qsizetype index = error_string.indexOf(url);
        if (index >= 0){
            error_string.replace(index, url.size(), "****************");
        }
        m_logger.log("Discord Request Response: " + error_string.toStdString(), COLOR_RED);
//        QString err = reply->errorString();
//        qDebug() << err;
    }
}

void DiscordWebhookSender::internal_send(
    const QUrl& url, const JsonValue& json,
    const std::vector<DiscordFileAttachment>& files
){
    if (m_stopping.load(std::memory_order_acquire)){
        return;
    }

    {
        std::lock_guard<std::mutex> lg(m_send_lock);
        m_event_loop.reset(new QEventLoop);
    }

    try{
        QHttpMultiPart multiPart(QHttpMultiPart::FormDataType);
        if (!json.is_null()){
            QHttpPart json_part;
            json_part.setHeader(
                QNetworkRequest::ContentDispositionHeader,
                QVariant("form-data; name=payload_json")
            );
            json_part.setBody(QByteArray::fromStdString(json.dump()));
            multiPart.append(json_part);
        }

        std::vector<QHttpPart> file_parts;
        std::deque<QFile> file_readers;
        file_parts.reserve(files.size());
        size_t c = 0;
        for (const auto& file : files){
            QFile& reader = file_readers.emplace_back(QString::fromStdString(file.filepath));
            if (!reader.open(QIODevice::ReadOnly)){
                m_logger.log("File doesn't exist: " + file.filepath, COLOR_RED);
                continue;
            }
            QHttpPart& part = file_parts.emplace_back();
            part.setHeader(
                QNetworkRequest::ContentDispositionHeader,
                QVariant(QString::fromStdString("application/octet-stream; name=file" + std::to_string(c) + "; filename=" + file.name))
            );
            part.setBodyDevice(&reader);
            multiPart.append(part);
            c++;
        }

        QNetworkRequest request(url);
        QNetworkAccessManager manager;
        m_event_loop->connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
//        cout << "Sending Webhook Message... (queue = " + tostr_u_commas(m_queue.size()) + ")" << endl;
        m_logger.log("Sending Webhook Message... (queue = " + tostr_u_commas(m_queue.size()) + ")", COLOR_BLUE);
        std::unique_ptr<QNetworkReply> reply(manager.post(request, &multiPart));

        if (!m_stopping.load(std::memory_order_acquire)){
//            cout << "internal_send() - exec" << endl;

            m_event_loop->exec();
            process_reply(reply.get());

//            cout << "internal_send() - end" << endl;
        }
    }catch (...){
        std::lock_guard<std::mutex> lg(m_send_lock);
        m_event_loop.reset();
        throw;
    }

    std::lock_guard<std::mutex> lg(m_send_lock);
    m_event_loop.reset();
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

        std::chrono::seconds delay(url.delay);

        JsonObject json;
        json["content"] = builder.build_message(
            delay,
            should_ping && url.ping,
            settings.message.user_id,
            settings.message.message
        );
        if (!embeds.empty()){
            json["embeds"] = embeds.clone();
        }

        DiscordWebhookSender::instance().send(
            logger,
            QString::fromStdString(url.url),
            delay,
            std::move(json),
            file
        );
    }
}

void send_embed(
    Logger& logger,
    bool should_ping,
    const std::vector<std::string>& tags,
    const JsonArray& embeds,
    const std::vector<std::shared_ptr<PendingFileSend>>& files
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

        std::chrono::seconds delay(url.delay);

        JsonObject json;
        json["content"] = builder.build_message(
            delay,
            should_ping && url.ping,
            settings.message.user_id,
            settings.message.message
        );
        if (!embeds.empty()){
            json["embeds"] = embeds.clone();
        }

        DiscordWebhookSender::instance().send(
            logger,
            QString::fromStdString(url.url),
            delay,
            std::move(json),
            files
        );
    }
}



}
}
}








