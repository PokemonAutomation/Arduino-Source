/*  Wild RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_WildRng_H
#define PokemonAutomation_PokemonFRLG_WildRng_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "CommonTools/Options/StringSelectOption.h"
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

class WildRng_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WildRng_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class WildRng : public SingleSwitchProgramInstance{
public:
    WildRng();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:

    enum class GameVersion{
        firered,
        leafgreen
    };

    enum class EncounterType{
        grass,
        rocksmash,
        surfing,
        oldrod,
        goodrod,
        superrod
    };

    bool have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t& TARGET_SEED, const AdvRngState& hit);
    
    SectionDividerOption m_calibration_displays;
    RngTargetDisplay RNG_TARGET;
    RngFilterDisplay RNG_FILTERS;
    RngCalibrationDisplay RNG_CALIBRATION;

    SectionDividerOption m_game_info;
    EnumDropdownOption<GameVersion> GAME_VERSION;
    OCR::LanguageOCROption LANGUAGE;

    SectionDividerOption m_target_settings;
    EnumDropdownOption<EncounterType> ENCOUNTER_TYPE;
    StringSelectDatabase LOCATIONS_DATABASE;
    StringSelectOption GAME_LOCATION;
    EnumDropdownOption<AdvRngMethod> RNG_METHOD;
    StringOption SEED; 
    TextEditOption SEED_LIST;
    EnumDropdownOption<SeedButton> SEED_BUTTON;
    EnumDropdownOption<BlackoutButton> EXTRA_BUTTON;
    SimpleIntegerOption<uint64_t> SEED_DELAY;
    SimpleIntegerOption<uint64_t>ADVANCES;

    SectionDividerOption m_program_settings;
    BooleanCheckBoxOption USE_TEACHY_TV;
    SimpleIntegerOption<uint64_t> MAX_RESETS;
    SimpleIntegerOption<uint64_t> MAX_RARE_CANDIES;
    SimpleIntegerOption<uint64_t> MAX_BALL_THROWS;
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
