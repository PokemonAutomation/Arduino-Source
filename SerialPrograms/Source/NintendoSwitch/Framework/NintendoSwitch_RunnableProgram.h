/*  Runnable Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_RunnableProgram_H
#define PokemonAutomation_NintendoSwitch_RunnableProgram_H

#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Panels/RunnablePanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class SwitchSetupFactory;


class RunnableSwitchProgramDescriptor : public RunnablePanelDescriptor{
public:
    RunnableSwitchProgramDescriptor(
        std::string identifier,
        QString category, QString display_name,
        QString doc_link,
        QString description,
        FeedbackType feedback,
        PABotBaseLevel min_pabotbase_level
    );

    FeedbackType feedback() const{ return m_feedback; }
    PABotBaseLevel min_pabotbase_level() const{ return m_min_pabotbase_level; }

protected:
    const FeedbackType m_feedback;
    const PABotBaseLevel m_min_pabotbase_level;
};



class RunnableSwitchProgramInstance : public RunnablePanelInstance{
public:
    using RunnablePanelInstance::RunnablePanelInstance;

    const RunnableSwitchProgramDescriptor& descriptor() const{
        return static_cast<const RunnableSwitchProgramDescriptor&>(m_descriptor);
    }

public:
    //  Serialization
    virtual void from_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

protected:
    friend class RunnableSwitchProgramWidget;

    SwitchSetupFactory* m_setup = nullptr;
};





}
}
#endif





