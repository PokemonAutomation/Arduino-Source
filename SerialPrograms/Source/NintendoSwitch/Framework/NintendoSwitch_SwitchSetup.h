/*  Switch Setup
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSetup_H
#define PokemonAutomation_NintendoSwitch_SwitchSetup_H

#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Logging/Logger.h"

class QJsonValue;
class QWidget;

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchSetupWidget;

class SwitchSetupFactory{
public:
    SwitchSetupFactory(
        PABotBaseLevel min_pabotbase, FeedbackType feedback
    )
        : m_min_pabotbase(min_pabotbase)
        , m_feedback(feedback)
    {}
    virtual ~SwitchSetupFactory() = default;

    virtual void load_json(const QJsonValue& json) = 0;
    virtual QJsonValue to_json() const = 0;

    virtual SwitchSetupWidget* make_ui(QWidget& parent, Logger& logger, uint64_t program_id) = 0;

protected:
    const PABotBaseLevel m_min_pabotbase;
    const FeedbackType m_feedback;
};




}
}
#endif
