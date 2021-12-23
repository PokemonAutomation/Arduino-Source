/*  Event Notification Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/QtJsonTools.h"
#include "EventNotificationOption.h"

#include <iostream>
using std::cout;
using std::endl;

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



void EventNotificationSettings::load_json(bool enable_screenshot, const QJsonValue& json){
    QJsonObject obj = json.toObject();
    if (obj.empty()){
        return;
    }
    json_get_bool(enabled, obj, "Enabled");
    json_get_bool(ping, obj, "Ping");
    if (enable_screenshot){
        ScreenshotOption screenshot_option("");
        screenshot_option.load_json(json_get_object_nothrow(obj, "Screenshot"));
        screenshot = screenshot_option;
    }
    QJsonArray array = json_get_array_nothrow(obj, "Tags");
    if (!array.empty()){
        tags.clear();
        for (const auto& tag : array){
            QString token = sanitize_tag(tag.toString());
            if (!token.isEmpty()){
//                cout << token.toStdString() << endl;
                tags.emplace_back(std::move(token));
            }
        }
    }
    int rate_limit_seconds = 0;
    json_get_int(rate_limit_seconds, obj, "RateLimitSeconds");
    rate_limit = std::chrono::seconds(rate_limit_seconds);
}
QJsonValue EventNotificationSettings::to_json(bool enable_screenshot) const{
    QJsonObject obj;
    obj.insert("Enabled", enabled);
    obj.insert("Ping", ping);
    if (enable_screenshot){
        ScreenshotOption screenshot_option("");
        screenshot_option = screenshot;
        obj.insert("Screenshot", screenshot_option.to_json());
    }
    QJsonArray array;
    for (const QString& tag : tags){
//        cout << tag.toStdString() << endl;
        array.append(tag);
    }
    obj.insert("Tags", array);
    obj.insert("RateLimitSeconds", rate_limit.count());
    return obj;
}





EventNotificationOption::EventNotificationOption()
    : screenshot_supported(false)
    , m_default({false, false, ImageAttachmentMode::NO_SCREENSHOT, {}, std::chrono::seconds(0)})
    , m_current(m_default)
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
{
    reset_rate_limit();
}


void EventNotificationOption::load_json(const QJsonValue& json){
    m_current.load_json(screenshot_supported, json);
}
QJsonValue EventNotificationOption::to_json() const{
    return m_current.to_json(screenshot_supported);
}
void EventNotificationOption::restore_defaults(){
    m_current = m_default;
}
void EventNotificationOption::reset_rate_limit(){
    m_last_sent = std::chrono::system_clock::time_point::min();
}
bool EventNotificationOption::ok_to_send_now(Logger& logger){
    if (!m_enabled){
        logger.log("EventNotification(" + m_label + "): Notifications not enabled.", "purple");
        return false;
    }
    if (!m_current.enabled){
        logger.log("EventNotification(" + m_label + "): Notifications disabled for this event type.", "purple");
        return false;
    }
//    if (m_current.rate_limit == std::chrono::seconds(0)){
//        return true;
//    }
    auto now = std::chrono::system_clock::now();
    if (now < m_last_sent + m_current.rate_limit){
        logger.log("EventNotification(" + m_label + "): Notification dropped due to rate limit.", "purple");
        return false;
    }
    m_last_sent = now;
    logger.log("EventNotification(" + m_label + "): Sending notification.", Qt::blue);
    return true;
}








}
