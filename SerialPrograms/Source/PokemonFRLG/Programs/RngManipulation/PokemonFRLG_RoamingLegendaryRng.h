/*  Roaming Legendary RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RoamingLegendaryRng_H
#define PokemonAutomation_PokemonFRLG_RoamingLegendaryRng_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
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
#include "PokemonFRLG_SeedsDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class RoamingLegendaryRng_Descriptor : public SingleSwitchProgramDescriptor{
public:
    RoamingLegendaryRng_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class RoamingLegendaryRng : public SingleSwitchProgramInstance{
public:
    RoamingLegendaryRng();
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

    SectionDividerOption m_calibration_displays;
    RngTargetDisplay RNG_TARGET;
    RngFilterDisplay RNG_FILTERS;
    RngCalibrationDisplay RNG_CALIBRATION;

    SectionDividerOption m_game_info;
    EnumDropdownOption<GameVersion> GAME_VERSION;
    OCR::LanguageOCROption LANGUAGE;
    EnumDropdownOption<SoundSetting> SOUND;

    SectionDividerOption m_target_settings;
    EnumDropdownOption<PokemonFRLG_RngTarget> TARGET;
    StringOption SEED;
    SimpleIntegerOption<uint64_t>ADVANCES;

    SectionDividerOption m_program_settings;
    BooleanCheckBoxOption USE_TEACHY_TV;
    SimpleIntegerOption<uint16_t> SEED_RADIUS;
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
