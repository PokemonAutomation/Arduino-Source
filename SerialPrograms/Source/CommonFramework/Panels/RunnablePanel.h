/*  Runnable Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RunnablePanel_H
#define PokemonAutomation_RunnablePanel_H

#include "CommonFramework/Options/BatchOption/BatchOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "Panel.h"

namespace PokemonAutomation{

class StatsTracker;


#if 0
class RunnableProgramDescriptor : public PanelDescriptor{
public:
    RunnableProgramDescriptor(
        std::string identifier,
        QString display_name,
        QString doc_link,
        QString description
    );
};
#endif
using RunnablePanelDescriptor = PanelDescriptor;



class RunnablePanelInstance : public PanelInstance{
public:
    RunnablePanelInstance(const PanelDescriptor& descriptor);

    void add_option(ConfigOption& option, std::string serialization_string){
        m_options.add_option(option, std::move(serialization_string));
    }

    const RunnablePanelDescriptor& descriptor() const{
        return static_cast<const RunnablePanelDescriptor&>(m_descriptor);
    }

    virtual std::unique_ptr<StatsTracker> make_stats() const;

    virtual std::string check_validity() const;
    virtual void restore_defaults();
    virtual void reset_state();

public:
    //  Serialization
    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

private:
    friend class RunnablePanelWidget;
    BatchOption m_options;
protected:
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationOption NOTIFICATION_ERROR_RECOVERABLE;
    EventNotificationOption NOTIFICATION_ERROR_FATAL;
};






}
#endif
