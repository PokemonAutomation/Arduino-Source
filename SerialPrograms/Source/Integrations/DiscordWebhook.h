/*  Discord Webhook
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DiscordWebhook_H
#define PokemonAutomation_DiscordWebhook_H

#include <deque>
#include <condition_variable>
#include <QNetworkReply>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Concurrency/ScheduledTaskRunner.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Notifications/MessageAttachment.h"

class QEventLoop;

namespace PokemonAutomation{
    class JsonArray;
    class JsonObject;
namespace Integration{
namespace DiscordWebhook{


struct DiscordFileAttachment{
    std::string name;
    std::string filepath;
};


class DiscordWebhookSender : public QObject{
    Q_OBJECT

    static constexpr auto THROTTLE_DURATION = std::chrono::seconds(1);
//    static constexpr size_t MAX_IN_WINDOW = 2;

private:
    DiscordWebhookSender();
    ~DiscordWebhookSender();


public:
    void send(
        Logger& logger,
        const QUrl& url, std::chrono::milliseconds delay,
        const JsonObject& obj,
        std::shared_ptr<PendingFileSend> file,
        std::function<void()> finish_callback = nullptr
    );
    void send(
        Logger& logger,
        const QUrl& url, std::chrono::milliseconds delay,
        const JsonObject& obj,
        std::vector<std::shared_ptr<PendingFileSend>> files,
        std::function<void()> finish_callback = nullptr
    );

    static DiscordWebhookSender& instance();


private:
    void cleanup_stuck_requests();
//    void thread_loop();
    void throttle();

    void process_reply(QNetworkReply* reply);
    void internal_send(
        const QUrl& url, const JsonValue& json,
        const std::vector<DiscordFileAttachment>& files
    );

private:
    TaggedLogger m_logger;
    std::atomic<bool> m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;

    std::mutex m_send_lock;
    std::unique_ptr<QEventLoop> m_event_loop;

    std::deque<WallClock> m_sent;
    AsyncDispatcher m_dispatcher;
    ScheduledTaskRunner m_queue;
};




void send_embed(
    Logger& logger,
    bool should_ping,
    const std::vector<std::string>& tags,
    const JsonArray& embeds,
    std::shared_ptr<PendingFileSend> file
);
void send_embed(
    Logger& logger,
    bool should_ping,
    const std::vector<std::string>& tags,
    const JsonArray& embeds,
    const std::vector<std::shared_ptr<PendingFileSend>>& files
);



}
}
}
#endif
