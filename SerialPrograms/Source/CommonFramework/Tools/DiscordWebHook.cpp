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


class PendingFileSend{
public:
    PendingFileSend(const QString& file, bool keep_file)
        : m_keep_file(keep_file)
        , m_file(file)
    {}
    ~PendingFileSend(){
        if (!m_keep_file){
            QFile file(m_file);
            file.remove();
        }
    }
    const QString& file() const{ return m_file; }

private:
    bool m_keep_file;
    QString m_file;
};


struct DiscordWebHookRequest{
    DiscordWebHookRequest() = default;
    DiscordWebHookRequest(Logger& p_logger, QUrl p_url, QByteArray p_data)
        : logger(&p_logger)
        , url(std::move(p_url))
        , data(std::move(p_data))
    {}
    DiscordWebHookRequest(Logger& p_logger, QUrl p_url, std::shared_ptr<PendingFileSend> p_file)
        : logger(&p_logger)
        , url(std::move(p_url))
        , file(std::move(p_file))
    {}
    Logger* logger;
    QUrl url;

    QByteArray data;
    std::shared_ptr<PendingFileSend> file;
};


class DiscordWebHookSender : public QObject{
    static constexpr auto THROTTLE_DURATION = std::chrono::seconds(1);
    static constexpr size_t MAX_IN_WINDOW = 2;

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
    void send_json(Logger& logger, const QUrl& url, const QJsonObject& obj){
        std::lock_guard<std::mutex> lg(m_lock);
        m_queue.emplace_back(
            logger,
            url,
            QJsonDocument(obj).toJson()
        );
        logger.log("Sending JSON to Discord... (queue = " + tostr_u_commas(m_queue.size()) + ")", "purple");
        m_cv.notify_all();
    }
    void send_file(Logger& logger, const QUrl& url, std::shared_ptr<PendingFileSend> file){
        std::lock_guard<std::mutex> lg(m_lock);
        m_queue.emplace_back(logger, url, file);
        logger.log("Sending File to Discord... (queue = " + tostr_u_commas(m_queue.size()) + ")", "purple");
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

                //  Throttle the messages.
                auto now = std::chrono::system_clock::now();
                while (!m_sent.empty() && m_sent[0] + THROTTLE_DURATION < now){
                    m_sent.pop_front();
                }
                if (m_sent.size() >= MAX_IN_WINDOW){
                    m_cv.wait_for(
                        lg, THROTTLE_DURATION,
                        [&]{
                            return m_sent[0] + THROTTLE_DURATION < now;
                        }
                    );
                    m_sent.clear();
                }
                m_sent.push_back(now);
            }

            if (!item.file){
                internal_send_json(*item.logger, item.url, item.data);
            }else{
                internal_send_file(*item.logger, item.url, item.file->file());
            }
        }
    }

    void process_reply(Logger& logger, QNetworkReply* reply){
        if (!reply){
            logger.log("QNetworkReply is null.", "red");
        }else if (reply->error() == QNetworkReply::NoError){
//            QString contents = QString::fromUtf8(reply->readAll());
//            qDebug() << contents;
        }else{
            logger.log("Discord Request Response: " + reply->errorString(), "red");
//            QString err = reply->errorString();
//            qDebug() << err;
        }
    }
    void internal_send_json(Logger& logger, const QUrl& url, const QByteArray& data){
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkAccessManager manager;
        QEventLoop loop;
        loop.connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
        std::unique_ptr<QNetworkReply> reply(manager.post(request, data));
        loop.exec();
        process_reply(logger, reply.get());
    }
    void internal_send_file(Logger& logger, const QUrl& url, const QString& filename){
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)){
            logger.log("File doesn't exist: " + filename, "red");
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
    std::deque<std::chrono::system_clock::time_point> m_sent;
    std::thread m_thread;
};


std::vector<QString> split_lines(const QString& str){
    std::vector<QString> lines;
    lines.emplace_back();
    for (QChar ch : str){
        if (ch == '\n'){
            lines.emplace_back();
            continue;
        }
        if (ch < 32){
            continue;
        }
        lines.back() += ch;
    }
    return lines;
}



void send_message(Logger& logger, bool should_ping, const QString& message, const QJsonObject& embed){
    QJsonArray embeds;
    embeds.append(embed);
    send_message(logger, should_ping, message, embeds);
}
void send_message(Logger& logger, bool should_ping, const QString& message, const QJsonArray& embeds){
    const DiscordSettingsOption& settings = PERSISTENT_SETTINGS().discord_settings;
    for (const QString& url : split_lines(settings.webhook_urls)){
//        cout << url.toStdString() << endl;
        if (url.isEmpty()){
            return;
        }

        //  customize the message
        QString str;
        if (should_ping){
            str += "<@" + settings.user_id + ">";
        }
        if (settings.ping_once){
            should_ping = false;
        }

        const QString& discord_message = settings.message;
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

        DiscordWebHookSender::instance().send_json(
            logger,
            url,
            jsonContent
        );
    }
}
void send_file(Logger& logger, QString file, bool keep_file){
    std::shared_ptr<PendingFileSend> pending(new PendingFileSend(file, keep_file));
    for (const QString& url : split_lines(PERSISTENT_SETTINGS().discord_settings.webhook_urls)){
        if (url.isEmpty()){
            return;
        }
        DiscordWebHookSender::instance().send_file(logger, url, pending);
    }
}
void send_image(Logger& logger, const QImage& image, const QString& format, bool keep_file){
    if (image.isNull()){
        logger.log("Shiny screenshot is null.", "red");
        return;
    }

    QString name = QString::fromStdString(now_to_filestring()) + format;

    if (!image.save(name)){
        logger.log("Unable to save shiny screenshot to: " + name, "red");
        return;
    }

    logger.log("Saved shiny screenshot to: " + name, "blue");
    DiscordWebHook::send_file(logger, name, keep_file);
}
void send_screenshot(Logger& logger, const QImage& image, ScreenshotMode mode, bool keep_file){
    if (mode == ScreenshotMode::NO_SCREENSHOT){
        return;
    }

    if (image.isNull()){
        logger.log("Shiny screenshot is null.", "red");
        return;
    }

    QString format;
    switch (mode){
    case ScreenshotMode::NO_SCREENSHOT:
        return;
    case ScreenshotMode::JPG:
        format = ".jpg";
        break;
    case ScreenshotMode::PNG:
        format = ".png";
        break;
    }

    DiscordWebHook::send_image(logger, image, format, keep_file);
}





}
}








