/*  Event Notification Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EventNotificationOption_H
#define PokemonAutomation_EventNotificationOption_H

#include <vector>
#include "Common/Cpp/Time.h"
#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Logging/LoggerQt.h"

namespace PokemonAutomation{


struct EventNotificationSettings{
    bool enabled = true;
    bool ping = false;
    ImageAttachmentMode screenshot = ImageAttachmentMode::NO_SCREENSHOT;
    std::vector<QString> tags;
    std::chrono::seconds rate_limit = std::chrono::seconds(0);

    void load_json(bool enable_screenshot, const QJsonValue& json);
    QJsonValue to_json(bool enable_screenshot) const;


    static QString sanitize_tag(const QString& token);
    static QString tags_to_str(const std::vector<QString>& tags);
    static std::vector<QString> parse_tags(const QString& str);
};



class EventNotificationOption{
public:
    EventNotificationOption();
    EventNotificationOption(
        QString label,
        bool enabled, bool ping,
        std::chrono::seconds rate_limit
    );
    EventNotificationOption(
        QString label,
        bool enabled, bool ping,
        std::vector<QString> tags = {"Notifs"},
        std::chrono::seconds rate_limit = std::chrono::seconds(0)
    );
    EventNotificationOption(
        QString label,
        bool enabled, bool ping,
        ImageAttachmentMode screenshot,
        std::vector<QString> tags = {"Notifs"},
        std::chrono::seconds rate_limit = std::chrono::seconds(0)
    );

    const QString&      label       () const{ return m_label; }
    bool                ping        () const{ return m_current.ping; }
    ImageAttachmentMode screenshot  () const{ return m_current.screenshot; }
    const std::vector<QString>& tags() const{ return m_current.tags; }

    void load_json(const QJsonValue& json);
    QJsonValue to_json() const;

    void restore_defaults();
    void reset_rate_limit();

    bool ok_to_send_now(LoggerQt& logger);

private:
    friend class EventNotificationsTable;
    friend class EventNotificationsTableUI;

    QString m_label;
    bool screenshot_supported;
    EventNotificationSettings m_default;
    EventNotificationSettings m_current;
    WallClock m_last_sent;

    //  Set by the table.
    bool m_enabled = true;
};



}
#endif
