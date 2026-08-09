/*  BDSP Starter RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_StarterRng_H
#define PokemonAutomation_PokemonBDSP_StarterRng_H

#include <stdint.h>
#include <vector>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_BdspRng.h"
#include "Pokemon/Pokemon_NatureChecker.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "PokemonBDSP/Options/PokemonBDSP_PlayerModelOption.h"
#include "PokemonBDSP_RngAim.h"
#include "PokemonBDSP_RngDisplays.h"
#include "PokemonBDSP_StarterNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  How an attempt ended.
enum class BdspAttemptOutcome{
    Abandoned,
    Missed,
    Hit,
    Unverifiable,
};


class StarterRng_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StarterRng_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class StarterRng : public SingleSwitchProgramInstance{
public:
    StarterRng();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    //  One attempt, from a freshly loaded save to a starter in hand.
    BdspAttemptOutcome run_attempt(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    //  Whether a generated starter is one worth pressing for.
    bool wanted(const Pokemon::BdspPokemonResult& pokemon) const;

private:
    //  How far the aim is shifted by what past attempts measured.
    RngAim m_aim;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EnumDropdownOption<BdspStarter> STARTER;
    PlayerModelOption PLAYER_MODEL;

    Pokemon::StatsHuntIvRangeFilterTable FILTERS;

    SimpleIntegerOption<uint16_t> MAX_RESETS;
    SimpleIntegerOption<uint16_t> MAX_TARGET_WAIT_MINUTES;

    BooleanCheckBoxOption AUTO_CALIBRATE;

    OCR::LanguageOCROption LANGUAGE;
    BooleanCheckBoxOption USE_SOUND_DETECTION;

    BlinkCollectionDisplay COLLECTION_DISPLAY;
    RngStateDisplay STATE_DISPLAY;
    RngTargetDisplay TARGET_DISPLAY;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
