/*  Event Notifications Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_EventNotificationsTable_H
#define PokemonAutomation_EventNotificationsTable_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "EventNotificationOption.h"

namespace PokemonAutomation{



class EventNotificationsTable : public StaticTableOption{
public:
    EventNotificationsTable(std::vector<EventNotificationOption*> options);

    void set_enabled(bool enabled);

    virtual std::vector<std::string> make_header() const override;

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

