/*  Event Notifications Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EventNotificationsTable_H
#define PokemonAutomation_EventNotificationsTable_H

#include <map>
#include "Common/Qt/AutoHeightTable.h"
#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/BatchOption/GroupOption.h"
#include "EventNotificationOption.h"

namespace PokemonAutomation{


class EventNotificationsTable : public ConfigOption{
public:
    EventNotificationsTable(std::vector<EventNotificationOption*> options);

    virtual void load_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;

    virtual void restore_defaults() override;
    virtual void reset_state() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

    void set_enabled(bool enabled);

private:
    friend class EventNotificationsTableWidget;
    std::vector<EventNotificationOption*> m_options;
    std::map<QString, EventNotificationOption*> m_name_map;
};

class EventNotificationsTableWidget : public QWidget, public ConfigWidget{
public:
    EventNotificationsTableWidget(QWidget& parent, EventNotificationsTable& value);

    virtual void restore_defaults();
    virtual void update_ui();

private:
    void redraw_table();
    QWidget* make_enabled_box   (EventNotificationOption& entry);
    QWidget* make_ping_box      (EventNotificationOption& entry);
    QWidget* make_screenshot_box(EventNotificationOption& entry);
    QWidget* make_tags_box      (EventNotificationOption& entry);
    QWidget* make_rate_limit_box(EventNotificationOption& entry);
    QWidget* make_test_box      (EventNotificationOption& entry);

private:
    EventNotificationsTable& m_value;
    AutoHeightTableWidget* m_table;
};




class EventNotificationsOption : public GroupOption{
public:
    EventNotificationsOption(std::vector<EventNotificationOption*> options);
    virtual void on_set_enabled(bool enabled) override;
private:
    EventNotificationsTable m_table;
};





}
#endif

