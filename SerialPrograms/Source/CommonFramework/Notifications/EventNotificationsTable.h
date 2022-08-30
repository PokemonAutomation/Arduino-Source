/*  Event Notifications Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EventNotificationsTable_H
#define PokemonAutomation_EventNotificationsTable_H

#include <map>
#include "Common/Cpp/Options/ConfigOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "EventNotificationOption.h"

namespace PokemonAutomation{


class EventNotificationsTable : public ConfigOption{
public:
    EventNotificationsTable(std::vector<EventNotificationOption*> options);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;
    virtual void reset_state() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

    void set_enabled(bool enabled);

private:
    friend class EventNotificationsTableWidget;
    std::vector<EventNotificationOption*> m_options;
    std::map<std::string, EventNotificationOption*> m_name_map;
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

