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
    std::vector<std::string> tags;
    std::chrono::seconds rate_limit = std::chrono::seconds(0);

    void load_json(bool enable_screenshot, const JsonValue& json);
    JsonValue to_json(bool enable_screenshot) const;


    static std::string sanitize_tag(const std::string& token);
    static std::string tags_to_str(const std::vector<std::string>& tags);
    static std::vector<std::string> parse_tags(const std::string& str);
};



class EventNotificationOption{
public:
    EventNotificationOption();
    EventNotificationOption(
        std::string label,
        bool enabled, bool ping,
        std::chrono::seconds rate_limit
    );
    EventNotificationOption(
        std::string label,
        bool enabled, bool ping,
        std::vector<std::string> tags = {"Notifs"},
        std::chrono::seconds rate_limit = std::chrono::seconds(0)
    );
    EventNotificationOption(
        std::string label,
        bool enabled, bool ping,
        ImageAttachmentMode screenshot,
        std::vector<std::string> tags = {"Notifs"},
        std::chrono::seconds rate_limit = std::chrono::seconds(0)
    );

    const std::string&  label           () const{ return m_label; }
    bool                ping            () const{ return m_current.ping; }
    ImageAttachmentMode screenshot      () const{ return m_current.screenshot; }
    const std::vector<std::string>& tags() const{ return m_current.tags; }

    void load_json(const JsonValue& json);
    JsonValue to_json() const;

    void restore_defaults();
    void reset_rate_limit();

    bool ok_to_send_now(LoggerQt& logger);

private:
    friend class EventNotificationsTable;
    friend class EventNotificationsTableWidget;

    std::string m_label;
    bool screenshot_supported;
    EventNotificationSettings m_default;
    EventNotificationSettings m_current;
    std::atomic<WallClock> m_last_sent;

    //  Set by the table.
    bool m_enabled = true;
};



}
#endif
