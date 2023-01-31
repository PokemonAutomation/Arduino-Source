/*  Event Notification Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/LabelCellOption.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "EventNotificationOption.h"

#include <QFont>
#include <QPushButton>
#include "Common/Qt/Options/ConfigWidget.h"

namespace PokemonAutomation{


class TestButtonWidget : public ConfigWidget{
public:
    TestButtonWidget(QWidget& parent, TestMessageButton& value)
        : ConfigWidget(value)
    {
        QPushButton* button = new QPushButton(&parent);
        m_widget = button;

        QFont font;
        font.setBold(true);
        button->setFont(font);
        button->setText("Send Test Message");

        button->connect(
            button, &QPushButton::clicked,
            button, [&](bool){
                send_raw_program_notification(
                    global_logger_tagged(), value.option,
                    COLOR_GREEN,
                    ProgramInfo("Test Message"),
                    "Notification Test",
                    {
                        {"Event Type:", value.option.label()},
                    }
                );
            }
        );
    }
};
TestMessageButton::TestMessageButton(EventNotificationOption& p_option)
    : ConfigOption(LockWhileRunning::UNLOCKED)
    , option(p_option)
{}
ConfigWidget* TestMessageButton::make_QtWidget(QWidget& parent){
    return new TestButtonWidget(parent, *this);
}




std::string EventNotificationOption::sanitize_tag(const std::string& token){
    std::string str;
    for (unsigned char ch : token){
        if (ch < 32) continue;
        if (ch == ' ') continue;
        str += ch;
    }
    return str;
}
std::string EventNotificationOption::tags_to_str(const std::vector<std::string>& tags){
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
std::vector<std::string> EventNotificationOption::parse_tags(const std::string& str){
    std::vector<std::string> tags;

    //  TODO: Don't use Qt here.
    for (QString token : QString::fromStdString(str).split(",")){
        std::string cstr = token.toStdString();
        cstr = sanitize_tag(cstr);
        if (!cstr.empty()){
            tags.emplace_back(std::move(cstr));
        }
    }
    return tags;
}


struct EventNotificationOption::Data{
    Data(
        std::string label,
        bool enabled, bool ping,
        std::chrono::seconds rate_limit
    )
        : screenshot_supported(false)
        , m_enabled(LockWhileRunning::UNLOCKED, enabled)
        , m_label(LockWhileRunning::UNLOCKED, std::move(label))
        , m_ping(LockWhileRunning::UNLOCKED, ping)
        , m_null_screenshot(LockWhileRunning::UNLOCKED, "---")
        , m_screenshot(ImageAttachmentMode::NO_SCREENSHOT)
        , m_tags(false, LockWhileRunning::UNLOCKED, "Notifs", "")
        , m_rate_limit_seconds(LockWhileRunning::UNLOCKED, rate_limit.count())
        , m_last_sent(WallClock::min())
        , m_global_enable(true)
    {}
    Data(
        std::string label,
        bool enabled, bool ping,
        std::vector<std::string> tags,
        std::chrono::seconds rate_limit
    )
        : screenshot_supported(false)
        , m_enabled(LockWhileRunning::UNLOCKED, enabled)
        , m_label(LockWhileRunning::UNLOCKED, std::move(label))
        , m_ping(LockWhileRunning::UNLOCKED, ping)
        , m_null_screenshot(LockWhileRunning::UNLOCKED, "---")
        , m_screenshot(ImageAttachmentMode::NO_SCREENSHOT)
        , m_tags(false, LockWhileRunning::UNLOCKED, tags_to_str(tags), "")
        , m_rate_limit_seconds(LockWhileRunning::UNLOCKED, rate_limit.count())
        , m_last_sent(WallClock::min())
        , m_global_enable(true)
    {}
    Data(
        std::string label,
        bool enabled, bool ping,
        ImageAttachmentMode screenshot,
        std::vector<std::string> tags,
        std::chrono::seconds rate_limit
    )
        : screenshot_supported(true)
        , m_enabled(LockWhileRunning::UNLOCKED, enabled)
        , m_label(LockWhileRunning::UNLOCKED, std::move(label))
        , m_ping(LockWhileRunning::UNLOCKED, ping)
        , m_null_screenshot(LockWhileRunning::UNLOCKED, "---")
        , m_screenshot(screenshot)
        , m_tags(false, LockWhileRunning::UNLOCKED, tags_to_str(tags), "")
        , m_rate_limit_seconds(LockWhileRunning::UNLOCKED, rate_limit.count())
        , m_last_sent(WallClock::min())
        , m_global_enable(true)
    {}


    bool screenshot_supported;

    BooleanCheckBoxCell m_enabled;
    LabelCellOption m_label;
    BooleanCheckBoxCell m_ping;
    LabelCellOption m_null_screenshot;
    ScreenshotCell m_screenshot;
    StringCell m_tags;
    SimpleIntegerCell<uint32_t> m_rate_limit_seconds;

    std::atomic<WallClock> m_last_sent;
    std::atomic<bool> m_global_enable;
};



EventNotificationOption::~EventNotificationOption(){}
EventNotificationOption::EventNotificationOption(
    std::string label,
    bool enabled, bool ping,
    std::chrono::seconds rate_limit
)
    : StaticTableRow(label)
    , m_data(CONSTRUCT_TOKEN, std::move(label), enabled, ping, rate_limit)
    , m_test_button(*this)
{
    add_option(m_data->m_enabled, "Enabled");
    add_option(m_data->m_label, "");
    add_option(m_data->m_ping, "Ping");
    add_option(m_data->m_null_screenshot, "");
    add_option(m_data->m_tags, "Tags");
    add_option(m_data->m_rate_limit_seconds, "RateLimitSeconds");
    add_option(m_test_button, "");

    reset_rate_limit();
}
EventNotificationOption::EventNotificationOption(
    std::string label,
    bool enabled, bool ping,
    std::vector<std::string> tags,
    std::chrono::seconds rate_limit
)
    : StaticTableRow(label)
    , m_data(CONSTRUCT_TOKEN, std::move(label), enabled, ping, std::move(tags), rate_limit)
    , m_test_button(*this)
{
    add_option(m_data->m_enabled, "Enabled");
    add_option(m_data->m_label, "");
    add_option(m_data->m_ping, "Ping");
    add_option(m_data->m_null_screenshot, "");
    add_option(m_data->m_tags, "Tags");
    add_option(m_data->m_rate_limit_seconds, "RateLimitSeconds");
    add_option(m_test_button, "");

    reset_rate_limit();
}
EventNotificationOption::EventNotificationOption(
    std::string label,
    bool enabled, bool ping,
    ImageAttachmentMode screenshot,
    std::vector<std::string> tags,
    std::chrono::seconds rate_limit
)
    : StaticTableRow(label)
    , m_data(CONSTRUCT_TOKEN, std::move(label), enabled, ping, screenshot, std::move(tags), rate_limit)
    , m_test_button(*this)
{
    add_option(m_data->m_enabled, "Enabled");
    add_option(m_data->m_label, "");
    add_option(m_data->m_ping, "Ping");
    add_option(m_data->m_screenshot, "Screenshot");
    add_option(m_data->m_tags, "Tags");
    add_option(m_data->m_rate_limit_seconds, "RateLimitSeconds");
    add_option(m_test_button, "");

    reset_rate_limit();
}

const std::string& EventNotificationOption::label() const{
    return m_data->m_label.text();
}
bool EventNotificationOption::ping() const{
    return m_data->m_ping;
}
ImageAttachmentMode EventNotificationOption::screenshot() const{
    return m_data->m_screenshot;
}
std::vector<std::string> EventNotificationOption::tags() const{
    return parse_tags(m_data->m_tags);
}

//void EventNotificationOption::set_tags(std::vector<std::string> tags){
//    m_data->m_tags.set(tags_to_str(tags));
//}

void EventNotificationOption::set_global_enable(bool enabled){
    m_data->m_global_enable.store(enabled, std::memory_order_release);
}
void EventNotificationOption::reset_rate_limit(){
    m_data->m_last_sent.store(WallClock::min(), std::memory_order_release);
}
bool EventNotificationOption::ok_to_send_now(Logger& logger){
    const std::string& label = m_data->m_label.text();

    if (!m_data->m_global_enable.load(std::memory_order_relaxed)){
        logger.log("EventNotification(" + label + "): Notifications not enabled.", COLOR_PURPLE);
        return false;
    }
    if (!m_data->m_enabled){
        logger.log("EventNotification(" + label + "): Notifications disabled for this event type.", COLOR_PURPLE);
        return false;
    }
//    if (m_current.rate_limit == std::chrono::seconds(0)){
//        return true;
//    }

    WallClock now;
    WallClock last;
    do{
        now = current_time();
        last = m_data->m_last_sent.load(std::memory_order_acquire);

        if (now < last + std::chrono::seconds(m_data->m_rate_limit_seconds)){
            logger.log("EventNotification(" + label + "): Notification dropped due to rate limit.", COLOR_PURPLE);
            return false;
        }

    }while (!m_data->m_last_sent.compare_exchange_weak(last, now));

    logger.log("EventNotification(" + label + "): Sending notification.", COLOR_BLUE);
    return true;
}








}
