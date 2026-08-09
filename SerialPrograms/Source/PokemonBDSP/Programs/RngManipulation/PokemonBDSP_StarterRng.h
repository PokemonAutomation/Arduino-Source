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
#include "PokemonBDSP/Options/PokemonBDSP_RngFilter.h"
#include "PokemonBDSP_BlinkRecovery.h"
#include "PokemonBDSP_RngAim.h"
#include "PokemonBDSP_RngDisplays.h"
#include "PokemonBDSP_StarterNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


enum class BdspStartPoint{
    Lakefront,
    Bedroom,
};


struct BdspSkipResult{
    bool success = false;
    Pokemon::Xorshift128State state;
    uint64_t target_advance = 0;
    uint64_t buffer = 0;
    std::string failure_reason;
};


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


class StarterRng : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    StarterRng();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual void on_config_value_changed(void* object) override;

    BdspAttemptOutcome run_attempt(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    //  Takes a snapshot rather than the table: the table clones itself on every query,
    //  which a scan over millions of advances cannot afford.
    static bool wanted(
        const BdspRngFilterSnapshot& filters, const Pokemon::BdspPokemonResult& pokemon
    );

    BdspSkipResult skip_advances_from_bedroom(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context
    );

    void report_skip_arrival(
        SingleSwitchProgramEnvironment& env,
        const BdspSkipResult& skip,
        const BlinkRecovery& arrival
    ) const;

private:
    RngAim m_aim;

private:
    OCR::LanguageOCROption LANGUAGE;
    PlayerModelOption PLAYER_MODEL;
    EnumDropdownOption<BdspStartPoint> START_POINT;
    EnumDropdownOption<BdspStarter> STARTER;

    BdspRngFilterTable FILTERS;

    BlinkCollectionDisplay COLLECTION_DISPLAY;
    RngStateDisplay STATE_DISPLAY;
    RngTargetDisplay TARGET_DISPLAY;

    SimpleIntegerOption<uint16_t> MAX_RESETS;
    SimpleIntegerOption<uint16_t> MAX_TARGET_WAIT_MINUTES;
    SimpleIntegerOption<uint16_t> MAX_SKIP_MINUTES;

    BooleanCheckBoxOption AUTO_CALIBRATE;

    BooleanCheckBoxOption USE_SOUND_DETECTION;
    BooleanCheckBoxOption TAKE_VIDEO;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
