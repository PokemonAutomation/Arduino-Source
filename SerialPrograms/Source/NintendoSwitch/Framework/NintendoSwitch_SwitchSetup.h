/*  Switch Setup
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSetup_H
#define PokemonAutomation_NintendoSwitch_SwitchSetup_H

#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Logging/LoggerQt.h"

class QJsonValue;
class QWidget;

namespace PokemonAutomation{
    class JsonValue2;
namespace NintendoSwitch{


class SwitchSetupWidget;

class SwitchSetupFactory{
public:
    SwitchSetupFactory(
        PABotBaseLevel min_pabotbase,
        FeedbackType feedback, bool allow_commands_while_running
    )
        : m_min_pabotbase(min_pabotbase)
        , m_feedback(feedback)
        , m_allow_commands_while_running(allow_commands_while_running)
    {}
    virtual ~SwitchSetupFactory() = default;

    virtual void load_json(const JsonValue2& json) = 0;
    virtual JsonValue2 to_json() const = 0;

    virtual SwitchSetupWidget* make_ui(QWidget& parent, LoggerQt& logger, uint64_t program_id) = 0;

protected:
    const PABotBaseLevel m_min_pabotbase;
    const FeedbackType m_feedback;
    const bool m_allow_commands_while_running;
};




}
}
#endif
