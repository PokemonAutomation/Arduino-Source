/*  Event Notification Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QStringList>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "EventNotificationOption.h"

namespace PokemonAutomation{



QString EventNotificationSettings::sanitize_tag(const QString& token){
    QString str;
    for (QChar ch : token){
        if (ch.unicode() < 32) continue;
        if (ch == ' ') continue;
        str += ch;
    }
    return str;
}
QString EventNotificationSettings::tags_to_str(const std::vector<QString>& tags){
    QString text;
    bool first = true;
    for (const QString& tag : tags){
        if (!first){
            text += ", ";
        }
        first = false;
        text += tag;
    }
    return text;
}
std::vector<QString> EventNotificationSettings::parse_tags(const QString& str){
    std::vector<QString> tags;
    for (QString token : str.split(",")){
        token = sanitize_tag(token);
        if (!token.isEmpty()){
            tags.emplace_back(std::move(token));
        }
    }
    return tags;
}



void EventNotificationSettings::load_json(bool enable_screenshot, const JsonValue2& json){
    const JsonObject2* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    obj->read_boolean(enabled, "Enabled");
    obj->read_boolean(ping, "Ping");
    if (enable_screenshot){
        const JsonValue2* value = obj->get_value("Screenshot");
        if (value){
            ScreenshotOption screenshot_option("");
            screenshot_option.load_json(*value);
            screenshot = screenshot_option;
        }
    }
    const JsonArray2* array = obj->get_array("Tags");
    if (array){
        tags.clear();
        for (const auto& tag : *array){
            const std::string* token = tag.get_string();
            if (token){
                tags.emplace_back(QString::fromStdString(*token));
            }
        }
    }
    int rate_limit_seconds = 0;
    obj->read_integer(rate_limit_seconds, "RateLimitSeconds");
    rate_limit = std::chrono::seconds(rate_limit_seconds);
}
JsonValue2 EventNotificationSettings::to_json(bool enable_screenshot) const{
    JsonObject2 obj;
    obj["Enabled"] = enabled;
    obj["Ping"] = ping;
    if (enable_screenshot){
        ScreenshotOption screenshot_option("");
        screenshot_option = screenshot;
        obj["Screenshot"] = screenshot_option.to_json();
    }
    JsonArray2 array;
    for (const QString& tag : tags){
//        cout << tag.toStdString() << endl;
        array.push_back(tag.toStdString());
    }
    obj["Tags"] = std::move(array);
    obj["RateLimitSeconds"] = rate_limit.count();
    return obj;
}





EventNotificationOption::EventNotificationOption()
    : screenshot_supported(false)
    , m_default({false, false, ImageAttachmentMode::NO_SCREENSHOT, {}, std::chrono::seconds(0)})
    , m_current(m_default)
    , m_last_sent(WallClock::min())
{
    reset_rate_limit();
}
EventNotificationOption::EventNotificationOption(
    QString label,
    bool enabled, bool ping,
    std::chrono::seconds rate_limit
)
    : m_label(std::move(label))
    , screenshot_supported(false)
    , m_default({enabled, ping, ImageAttachmentMode::NO_SCREENSHOT, {"Notifs"}, rate_limit})
    , m_current(m_default)
    , m_last_sent(WallClock::min())
{
    reset_rate_limit();
}
EventNotificationOption::EventNotificationOption(
    QString label,
    bool enabled, bool ping,
    std::vector<QString> tags,
    std::chrono::seconds rate_limit
)
    : m_label(std::move(label))
    , screenshot_supported(false)
    , m_default({enabled, ping, ImageAttachmentMode::NO_SCREENSHOT, std::move(tags), rate_limit})
    , m_current(m_default)
    , m_last_sent(WallClock::min())
{
    reset_rate_limit();
}
EventNotificationOption::EventNotificationOption(
    QString label,
    bool enabled, bool ping,
    ImageAttachmentMode screenshot,
    std::vector<QString> tags,
    std::chrono::seconds rate_limit
)
    : m_label(std::move(label))
    , screenshot_supported(true)
    , m_default({enabled, ping, screenshot, std::move(tags), rate_limit})
    , m_current(m_default)
    , m_last_sent(WallClock::min())
{
    reset_rate_limit();
}


void EventNotificationOption::load_json(const JsonValue2& json){
    m_current.load_json(screenshot_supported, json);
}
JsonValue2 EventNotificationOption::to_json() const{
    return m_current.to_json(screenshot_supported);
}
void EventNotificationOption::restore_defaults(){
    m_current = m_default;
}
void EventNotificationOption::reset_rate_limit(){
    m_last_sent.store(WallClock::min(), std::memory_order_release);
}
bool EventNotificationOption::ok_to_send_now(LoggerQt& logger){
    if (!m_enabled){
        logger.log("EventNotification(" + m_label + "): Notifications not enabled.", COLOR_PURPLE);
        return false;
    }
    if (!m_current.enabled){
        logger.log("EventNotification(" + m_label + "): Notifications disabled for this event type.", COLOR_PURPLE);
        return false;
    }
//    if (m_current.rate_limit == std::chrono::seconds(0)){
//        return true;
//    }

    WallClock now;
    WallClock last;
    do{
        now = current_time();
        last = m_last_sent.load(std::memory_order_acquire);

        if (now < last + m_current.rate_limit){
            logger.log("EventNotification(" + m_label + "): Notification dropped due to rate limit.", COLOR_PURPLE);
            return false;
        }

    }while (!m_last_sent.compare_exchange_weak(last, now));

    logger.log("EventNotification(" + m_label + "): Sending notification.", COLOR_BLUE);
    return true;
}








}
