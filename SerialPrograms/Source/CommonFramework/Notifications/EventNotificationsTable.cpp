/*  Program Notification Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "EventNotificationsTable.h"

namespace PokemonAutomation{


EventNotificationsTable::EventNotificationsTable(std::vector<EventNotificationOption*> options)
    : StaticTableOption("", LockMode::UNLOCK_WHILE_RUNNING, false)
{
    for (EventNotificationOption* option : options){
        add_row(option);
    }
    finish_construction();
}

void EventNotificationsTable::set_enabled(bool enabled){
    for (StaticTableRow* row : this->table()){
        EventNotificationOption* option = static_cast<EventNotificationOption*>(row);
        option->set_global_enable(enabled);
    }
}

std::vector<std::string> EventNotificationsTable::make_header() const{
    std::vector<std::string> ret{
        "Enable",
        "Event",
        "Should Ping",
        "Screenshot",
        "Tags",
        "Rate Limit (seconds)",
        "",
    };
    return ret;
}


EventNotificationsOption::EventNotificationsOption(std::vector<EventNotificationOption*> options)
    : GroupOption(
        "Discord Notifications",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_ENABLED, false
    )
    , m_table(std::move(options))
{
    PA_ADD_OPTION(m_table);
}
void EventNotificationsOption::on_set_enabled(bool enabled){
    m_table.set_enabled(enabled);
}











}
