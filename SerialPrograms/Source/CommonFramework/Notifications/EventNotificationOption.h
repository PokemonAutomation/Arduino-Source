/*  Event Notification Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_EventNotificationOption_H
#define PokemonAutomation_EventNotificationOption_H

#include <chrono>
#include <vector>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"

namespace PokemonAutomation{


class EventNotificationOption;


class TestMessageButton : public ConfigOption{
public:
    TestMessageButton(EventNotificationOption& p_option);
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

    EventNotificationOption& option;
};



class EventNotificationOption : public StaticTableRow{
public:
    static std::string sanitize_tag(const std::string& token);
    static std::string tags_to_str(const std::vector<std::string>& tags);
    static std::vector<std::string> parse_tags(const std::string& str);

public:
    ~EventNotificationOption();
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

    const std::string&  label       () const;
    bool                ping        () const;
    ImageAttachmentMode screenshot  () const;
    std::vector<std::string> tags   () const;

//    void set_tags(std::vector<std::string> tags);

    void set_global_enable(bool enabled);
    void reset_rate_limit();

    bool ok_to_send_now(Logger& logger);

private:
    friend class EventNotificationsTable;

    struct Data;
    Pimpl<Data> m_data;

    TestMessageButton m_test_button;
};



}
#endif
