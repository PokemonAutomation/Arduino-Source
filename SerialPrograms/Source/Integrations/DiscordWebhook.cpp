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
#include "Common/Cpp/PanicDump.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "DiscordWebhook.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Integration{
namespace DiscordWebhook{


#if 0
DiscordWebhookRequest::DiscordWebhookRequest(QUrl p_url, QByteArray p_data, std::shared_ptr<PendingFileSend> p_file)
    : url(std::move(p_url))
    , data(std::move(p_data))
    , file(std::move(p_file))
{}

DiscordWebhookRequest::DiscordWebhookRequest( QUrl p_url, std::shared_ptr<PendingFileSend> p_file)
    : url(std::move(p_url))
    , file(std::move(p_file))
{}
#endif


DiscordWebhookSender::DiscordWebhookSender()
    : m_logger(global_logger_raw(), "DiscordWebhookSender")
    , m_stopping(false)
    , m_dispatcher(nullptr, 1)
    , m_queue(m_dispatcher)
//    , m_thread(run_with_catch, "DiscordWebhookSender::thread_loop()", [this]{ thread_loop(); })
{}

DiscordWebhookSender::~DiscordWebhookSender(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
        m_cv.notify_all();
    }
//    m_thread.join();
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
    std::lock_guard<std::mutex> lg(m_lock);
    cleanup_stuck_requests();
//    m_queue.emplace_back(
//        url,
//        QByteArray::fromStdString(obj.dump()),
//        file
//    );
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
    m_cv.notify_all();
}

void DiscordWebhookSender::send_file(
    Logger& logger,
    const QUrl& url, std::chrono::milliseconds delay,
    std::shared_ptr<PendingFileSend> file
){
    std::lock_guard<std::mutex> lg(m_lock);
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
    m_cv.notify_all();
}

void DiscordWebhookSender::cleanup_stuck_requests(){
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

#if 0
    if (m_queue.empty()){
        return;
    }
    WallClock now = current_time();
    WallClock threshold = now - std::chrono::seconds(60);
    size_t reqs_purged = 0;
    while (!m_queue.empty()){
        if (m_queue.front().timestamp < threshold){
            m_queue.pop_front();
            reqs_purged++;
        }else{
            break;
        }
    }
    if (reqs_purged == 0){
        return;
    }
    m_logger.log("Purged " + std::to_string(reqs_purged) + " requests which appear to be stuck.", COLOR_RED);

    emit stop_event_loop();
#endif
}
void DiscordWebhookSender::throttle(){
    //  Throttle the messages.
    std::unique_lock<std::mutex> lg(m_lock);
    auto duration = THROTTLE_DURATION;
    auto now = current_time();
    while (!m_sent.empty() && m_sent[0] + duration < now){
        m_sent.pop_front();
    }
    if (m_sent.size() >= GlobalSettings::instance().DISCORD.webhooks.sends_per_second){
        m_logger.log("Throttling webhook messages due to rate limit...", COLOR_RED);
        m_cv.wait_for(
            lg, duration,
            [&]{ return m_sent[0] + duration < now; }
        );
        m_sent.clear();
    }
    m_sent.push_back(now);
}

#if 0
void DiscordWebhookSender::thread_loop(){
//    QEventLoop event_loop;
//    connect(
//        this, &DiscordWebhookSender::stop_event_loop,
//        &event_loop, &QEventLoop::quit
//    );
    while (true){
        DiscordWebhookRequest item;
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
            auto now = current_time();
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
#endif

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

void send_message(
    Logger& logger,
    bool should_ping,
    const std::vector<std::string>& tags,
    const std::string& message,
    const JsonArray& embeds,
    std::shared_ptr<PendingFileSend> file
){
    DiscordSettingsOption& settings = GlobalSettings::instance().DISCORD;
    if (!settings.webhooks.enabled()){
        return;
    }

    std::set<std::string> tag_set;
    for (const std::string& tag : tags){
        tag_set.insert(to_lower(tag));
    }

    std::vector<std::unique_ptr<DiscordWebhookUrl>> list = settings.webhooks.urls.copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const DiscordWebhookUrl& url = *list[c];
        if (!url.enabled){
            continue;
        }
        if (((std::string)url.url).empty()){
            continue;
        }
//        cout << url.url.toStdString() << " : tags = " << url.tags.size() << endl;
//        for (const QString& tag : tag_set){
//            cout << "event tag: " << tag.toStdString() << endl;
//        }

        //  See if a tag matches.
        bool send = false;
        for (const std::string& tag : EventNotificationOption::parse_tags(url.tags_text)){
//            cout << "find tag: " << tag.toStdString() << endl;
            auto iter = tag_set.find(to_lower(tag));
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
        if (std::atoll(((std::string)settings.message.user_id).c_str()) == 0){
            should_ping = false;
        }

        //  Message
        std::string str;
        if (should_ping && url.ping){
            str += "<@" + (std::string)settings.message.user_id + ">";
        }

        const std::string& discord_message = settings.message.message;
        if (!discord_message.empty()){
            if (!str.empty()){
                str += " ";
            }
            for (char ch : discord_message){
                if (ch != '@'){
                    str += ch;
                }
            }
//            str += discord_message;
        }
        if (!message.empty()){
            if (!str.empty()){
                str += " ";
            }
            str += message;
        }

        JsonObject jsonContent;
        jsonContent["content"] = str;
        jsonContent["embeds"] = embeds.clone();

        DiscordWebhookSender::instance().send_json(
            logger,
            QString::fromStdString(url.url),
            std::chrono::seconds(url.delay),
            jsonContent, file
        );
    }
}




}
}
}








