/*  Wild Pokemon Finder
 *
 *  Search grass encounters until a selected species is found.
 */

#ifndef PokemonAutomation_PokemonFRLG_WildPokemonFinder_H
#define PokemonAutomation_PokemonFRLG_WildPokemonFinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_NameSelectOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class WildPokemonFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WildPokemonFinder_Descriptor();
    struct Stats;
    std::unique_ptr<StatsTracker> make_stats() const override;
};

class WildPokemonFinder : public SingleSwitchProgramInstance{
public:
    using Descriptor = WildPokemonFinder_Descriptor;
    WildPokemonFinder();

    void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    void start_program_border_check(VideoStream&, FeedbackType) override{}

private:
    enum class GameVersion{
        firered,
        leafgreen,
    };

    EnumDropdownOption<GameVersion> GAME_VERSION;
    Pokemon::PokemonNameSelectOption TARGET_POKEMON;
    OCR::LanguageOCROption LANGUAGE;

    EventNotificationOption NOTIFICATION_TARGET_FOUND;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}

#endif