/*  Discord Webhook
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordWebhook_H
#define PokemonAutomation_DiscordWebhook_H

#include <deque>
#include <condition_variable>
#include <thread>
#include <QImage>
#include <QNetworkReply>
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/MessageAttachment.h"

class QJsonArray;
class QJsonObject;
class QString;

namespace PokemonAutomation{
namespace Integration{
namespace DiscordWebhook{


struct DiscordWebhookRequest{
    DiscordWebhookRequest() = default;
    DiscordWebhookRequest(QUrl p_url, QByteArray p_data, std::shared_ptr<PendingFileSend> file);
    DiscordWebhookRequest(QUrl p_url, std::shared_ptr<PendingFileSend> p_file);

    QUrl url;

    QByteArray data;
    std::shared_ptr<PendingFileSend> file;
};

class DiscordWebhookSender : public QObject{
    static constexpr auto THROTTLE_DURATION = std::chrono::seconds(1);
    static constexpr size_t MAX_IN_WINDOW = 2;

private:
    DiscordWebhookSender();
    ~DiscordWebhookSender();


public:
    void send_json(Logger& logger, const QUrl& url, const QJsonObject& obj, std::shared_ptr<PendingFileSend> file);
    void send_file(Logger& logger, const QUrl& url, std::shared_ptr<PendingFileSend> file);

    static DiscordWebhookSender& instance();


private:
    void thread_loop();

    void process_reply(QNetworkReply* reply);
    void internal_send_json(const QUrl& url, const QByteArray& data);
    void internal_send_file(const QUrl& url, const QString& filename);
    void internal_send_image_embed(const QUrl& url, const QByteArray& data, const QString& filepath, const QString& filename);


private:
    TaggedLogger m_logger;
    bool m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;
    std::deque<DiscordWebhookRequest> m_queue;
    std::deque<std::chrono::system_clock::time_point> m_sent;
    std::thread m_thread;
};




void send_message(
    Logger& logger,
    bool should_ping,
    const std::vector<QString>& tags,
    const QString& message,
    const QJsonArray& embeds,
    std::shared_ptr<PendingFileSend> file
);



}
}
}
#endif
