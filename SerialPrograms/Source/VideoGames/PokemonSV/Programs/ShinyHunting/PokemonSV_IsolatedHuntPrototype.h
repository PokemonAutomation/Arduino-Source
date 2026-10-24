/*  Isolated Hunt Prototype
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_IsolatedHuntPrototype_H
#define PokemonAutomation_PokemonSV_IsolatedHuntPrototype_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "VideoGames/PokemonSV/Options/PokemonSV_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class IsolatedHuntPrototype_Descriptor : public SingleSwitchProgramDescriptor{
public:
    IsolatedHuntPrototype_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class IsolatedHuntPrototype : public SingleSwitchProgramInstance{
public:
    using Descriptor = IsolatedHuntPrototype_Descriptor;
    IsolatedHuntPrototype();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    enum class Start{
        PREPARED_SPOT,
        CENTRAL_PLAZA,
    };

    OCR::LanguageOCROption LANGUAGE;
    EnumDropdownOption<Start> START;
    SimpleIntegerOption<uint16_t> RUN_SECONDS;
    SimpleIntegerOption<uint8_t> EMPTY_PASSES;
    EncounterBotCommonOptions ENCOUNTER_OPTIONS;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif
