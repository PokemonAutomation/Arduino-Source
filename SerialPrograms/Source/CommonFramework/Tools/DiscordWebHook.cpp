/*  Discord WebHook
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
//#include <set>
#include <condition_variable>
#include <thread>
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
    DiscordWebHookRequest(QUrl p_url, QByteArray p_data, Logger* p_logger)
        : url(std::move(p_url))
        , data(std::move(p_data))
        , logger(p_logger)
    {}
    QUrl url;
    QByteArray data;
    Logger* logger;
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
    void send(const QUrl& url, const QJsonObject& obj, Logger* logger = nullptr){
        std::lock_guard<std::mutex> lg(m_lock);
        m_queue.emplace_back(
            url,
            QJsonDocument(obj).toJson(),
            logger
        );
        if (logger){
            logger->log("Sending Discord Notification... (queue = " + tostr_u_commas(m_queue.size()) + ")", "purple");
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

            QNetworkRequest request(item.url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

//            if (item.logger){
//                item.logger->log("Sending Discord Notification...", "purple");
//            }

            QNetworkAccessManager manager;
//            cout << reply.get() << endl;

            QEventLoop loop;
            loop.connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
            std::unique_ptr<QNetworkReply> reply(manager.post(request, item.data));
            loop.exec();

            if (reply->error() == QNetworkReply::NoError) {
//                QString contents = QString::fromUtf8(reply->readAll());
//                qDebug() << contents;
            }else{
                if (item.logger){
                    item.logger->log("Discord Request Response: " + reply->errorString(), "red");
                }
//                QString err = reply->errorString();
//                qDebug() << err;
            }
        }
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
    if (PERSISTENT_SETTINGS().DISCORD_WEBHOOK_URL.isEmpty()){
        return;
    }

    //  customize the message
    QString name = should_ping
        ? "<@" + PERSISTENT_SETTINGS().DISCORD_USER_ID + "> "
        : PERSISTENT_SETTINGS().DISCORD_USER_SHORT_NAME + " ";

    QJsonObject jsonContent;
    jsonContent["content"] = name + message;
    jsonContent["embeds"] = embeds;

    DiscordWebHookSender::instance().send(
        PERSISTENT_SETTINGS().DISCORD_WEBHOOK_URL,
        jsonContent,
        logger
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


}
}
