/*  Runnable Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_RunnableProgram_H
#define PokemonAutomation_NintendoSwitch_RunnableProgram_H

#include "CommonFramework/Globals.h"
#include "CommonFramework/ControllerDevices/SerialPortGlobals.h"
#include "CommonFramework/Panels/RunnablePanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class SwitchSetupFactory;


class RunnableSwitchProgramDescriptor : public RunnablePanelDescriptor{
public:
    RunnableSwitchProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        FeedbackType feedback, bool allow_commands_while_running,
        PABotBaseLevel min_pabotbase_level
    );

    FeedbackType feedback() const{ return m_feedback; }
    PABotBaseLevel min_pabotbase_level() const{ return m_min_pabotbase_level; }
    bool allow_commands_while_running() const{ return m_allow_commands_while_running; }

protected:
    const FeedbackType m_feedback;
    const PABotBaseLevel m_min_pabotbase_level;
    const bool m_allow_commands_while_running;
};



class RunnableSwitchProgramInstance : public RunnablePanelInstance{
public:
    using RunnablePanelInstance::RunnablePanelInstance;

    const RunnableSwitchProgramDescriptor& descriptor() const{
        return static_cast<const RunnableSwitchProgramDescriptor&>(m_descriptor);
    }

public:
    //  Serialization
    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

protected:
    friend class RunnableSwitchProgramWidget;

    SwitchSetupFactory* m_setup = nullptr;
};





}
}
#endif





