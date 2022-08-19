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



std::string EventNotificationSettings::sanitize_tag(const std::string& token){
    std::string str;
    for (unsigned char ch : token){
        if (ch < 32) continue;
        if (ch == ' ') continue;
        str += ch;
    }
    return str;
}
std::string EventNotificationSettings::tags_to_str(const std::vector<std::string>& tags){
    std::string text;
    bool first = true;
    for (const std::string& tag : tags){
        if (!first){
            text += ", ";
        }
        first = false;
        text += tag;
    }
    return text;
}
std::vector<std::string> EventNotificationSettings::parse_tags(const std::string& str){
    std::vector<std::string> tags;
    for (QString token : QString::fromStdString(str).split(",")){
        std::string cstr = token.toStdString();
        cstr = sanitize_tag(cstr);
        if (!cstr.empty()){
            tags.emplace_back(std::move(cstr));
        }
    }
    return tags;
}



void EventNotificationSettings::load_json(bool enable_screenshot, const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    obj->read_boolean(enabled, "Enabled");
    obj->read_boolean(ping, "Ping");
    if (enable_screenshot){
        const JsonValue* value = obj->get_value("Screenshot");
        if (value){
            ScreenshotOption screenshot_option("");
            screenshot_option.load_json(*value);
            screenshot = screenshot_option;
        }
    }
    const JsonArray* array = obj->get_array("Tags");
    if (array){
        tags.clear();
        for (const auto& tag : *array){
            const std::string* token = tag.get_string();
            if (token){
                tags.emplace_back(*token);
            }
        }
    }
    int rate_limit_seconds = 0;
    obj->read_integer(rate_limit_seconds, "RateLimitSeconds");
    rate_limit = std::chrono::seconds(rate_limit_seconds);
}
JsonValue EventNotificationSettings::to_json(bool enable_screenshot) const{
    JsonObject obj;
    obj["Enabled"] = enabled;
    obj["Ping"] = ping;
    if (enable_screenshot){
        ScreenshotOption screenshot_option("");
        screenshot_option.set(screenshot);
        obj["Screenshot"] = screenshot_option.to_json();
    }
    JsonArray array;
    for (const std::string& tag : tags){
//        cout << tag.toStdString() << endl;
        array.push_back(tag);
    }
    obj["Tags"] = std::move(array);
    obj["RateLimitSeconds"] = rate_limit.count();
    return obj;
}





EventNotificationOption::EventNotificationOption(
    std::string label,
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
    std::string label,
    bool enabled, bool ping,
    std::vector<std::string> tags,
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
    std::string label,
    bool enabled, bool ping,
    ImageAttachmentMode screenshot,
    std::vector<std::string> tags,
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


void EventNotificationOption::load_json(const JsonValue& json){
    m_current.load_json(screenshot_supported, json);
}
JsonValue EventNotificationOption::to_json() const{
    return m_current.to_json(screenshot_supported);
}
void EventNotificationOption::restore_defaults(){
    m_current = m_default;
}
void EventNotificationOption::reset_rate_limit(){
    m_last_sent.store(WallClock::min(), std::memory_order_release);
}
bool EventNotificationOption::ok_to_send_now(Logger& logger){
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
