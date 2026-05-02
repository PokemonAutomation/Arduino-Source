/*  StaticRng
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_StaticRng_H
#define PokemonAutomation_PokemonFRLG_StaticRng_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
#include "Pokemon/Pokemon_AdvRng.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_RngCalibration.h"
#include "PokemonFRLG_RngDisplays.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class StaticRng_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StaticRng_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class StaticRng : public SingleSwitchProgramInstance{
public:
    StaticRng();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:

    bool have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t& TARGET_SEED, const AdvRngState& hit);

    AdvObservedPokemon read_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    bool auto_catch(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& MAX_BALL_THROWS);

    bool use_rare_candy(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        AdvObservedPokemon& pokemon,
        AdvRngFilters& filters,
        const BaseStats& BASE_STATS,
        bool first
    );
    
    OCR::LanguageOCROption LANGUAGE;

    EnumDropdownOption<PokemonFRLG_RngTarget> TARGET;

    SimpleIntegerOption<uint64_t> MAX_RESETS;
    SimpleIntegerOption<uint64_t> MAX_RARE_CANDIES;
    SimpleIntegerOption<uint64_t> MAX_BALL_THROWS;

    RngFilterDisplay RNG_FILTERS;
    RngCalibrationDisplay RNG_CALIBRATION;

    StringOption SEED; 
    TextEditOption SEED_LIST;
    EnumDropdownOption<SeedButton> SEED_BUTTON;
    EnumDropdownOption<BlackoutButton> EXTRA_BUTTON;
    SimpleIntegerOption<uint64_t> SEED_DELAY;

    SimpleIntegerOption<uint64_t>ADVANCES;

    BooleanCheckBoxOption USE_TEACHY_TV;

    SimpleIntegerOption<uint8_t> PROFILE;

    BooleanCheckBoxOption TAKE_VIDEO;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif
