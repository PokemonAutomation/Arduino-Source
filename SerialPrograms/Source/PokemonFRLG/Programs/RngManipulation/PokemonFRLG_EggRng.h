/*  Egg RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_EggRng_H
#define PokemonAutomation_PokemonFRLG_EggRng_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LabelCellOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Pokemon/Options/Pokemon_NameSelectOption.h"
#include "Pokemon/Pokemon_AdvRng.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_RngCalibration.h"
#include "PokemonFRLG_RngStatsDatabase.h"
#include "PokemonFRLG_RngDisplays.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class ParentIVsRow : public StaticTableRow{
public:
    ParentIVsRow(int index, std::string&& label);
    LabelCellOption parent;
    SimpleIntegerCell<uint8_t> hp;
    SimpleIntegerCell<uint8_t> atk;
    SimpleIntegerCell<uint8_t> def;
    SimpleIntegerCell<uint8_t> spa;
    SimpleIntegerCell<uint8_t> spd;
    SimpleIntegerCell<uint8_t> spe;
    int index;
};

class ParentIVsTable : public StaticTableOption {
public:
    ParentIVsTable();
    virtual std::vector<std::string> make_header() const;
};


class EggRng_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggRng_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class EggRng : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    EggRng();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    virtual void on_config_value_changed(void* object) override;

    enum class GameVersion{
        firered,
        leafgreen
    };

    enum class EggProgramState{
        held_prep,
        held_calibration,
        pickup_calibration,
        finished
    };

    bool have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t& TARGET_SEED, const uint64_t& TARGET_ADVANCES, const AdvRngState& hit);

    void prep_held_resets(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    bool reset_and_check_seed(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        EggRng_Descriptor::Stats& stats,
        RngUncertainHistory& wild_uncertain_history,
        RngUncertainHistory& egg_uncertain_history,    
        RngCalibrationHistory& wild_history,
        RngCalibrationHistory& egg_history,
        AdvRngWildSearcher& wild_searcher,
        RngCalibrations& calibrations,
        WallClock& timestamp,
        uint16_t& current_seed,
        uint64_t& balls_left, 
        uint64_t& candies_left, 
        uint16_t& failed_searches,
        uint16_t& times_not_held,
        bool& shiny_found,
        const bool& previously_hit_held_frame,
        Milliseconds& launch_delay,
        const uint64_t& SEED_DELAY,
        const std::vector<uint16_t>& SEED_VALUES,
        const int16_t& SEED_POSITION,
        const std::set<std::string>& SPECIES_LIST,
        const RngStatsDatabase& STATS_DATA,
        bool pickup_frame
    );
    
    bool held_frame_check(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        EggRng_Descriptor::Stats& stats,
        RngUncertainHistory& egg_uncertain_history,
        RngCalibrationHistory& held_calibration_history,
        AdvRngEggSearcher& egg_searcher,
        RngCalibrations& calibrations,
        const WallClock& timestamp,
        const uint16_t& current_seed, 
        uint64_t& candies_left,
        uint16_t& failed_searches,
        bool& shiny_found,
        bool& previously_hit_held_frame,
        const uint16_t& TARGET_HELD_SEED,
        const RngStats& EGG_STATS,
        AdvIVs& PARENT_A,
        AdvIVs& PARENT_B
    );

    bool pickup_frame_check(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        EggRng_Descriptor::Stats& stats,
        RngUncertainHistory& egg_uncertain_history,
        RngCalibrationHistory& pickup_calibration_history,
        AdvRngEggSearcher& egg_searcher,
        RngCalibrations& calibrations,
        uint64_t& candies_left,
        uint16_t& failed_searches,
        bool& shiny_found,
        const uint16_t& TARGET_HELD_SEED,
        const uint16_t& TARGET_PICKUP_SEED,
        const RngStats& EGG_STATS,
        AdvIVs& PARENT_A,
        AdvIVs& PARENT_B
    );


    SectionDividerOption m_calibration_displays;
    RngTargetDisplay RNG_TARGET;
    RngFilterDisplay RNG_FILTERS;
    RngCalibrationDisplay HELD_CALIBRATION;
    RngCalibrationDisplay PICKUP_CALIBRATION;

    SectionDividerOption m_game_info;
    OCR::LanguageOCROption LANGUAGE;
    EnumDropdownOption<GameVersion> GAME_VERSION;

    SectionDividerOption m_target_settings;
    PokemonNameSelectOption EGG_SPECIES;
    EnumDropdownOption<AdvEggCompatibility> COMPATIBILITY;
    ParentIVsTable PARENT_IVS;

    SectionDividerOption m_held_settings;
    StringOption HELD_SEED; 
    TextEditOption HELD_SEED_LIST;
    EnumDropdownOption<SeedButton> HELD_SEED_BUTTON;
    EnumDropdownOption<BlackoutButton> HELD_EXTRA_BUTTON;
    SimpleIntegerOption<uint64_t> HELD_SEED_DELAY;
    SimpleIntegerOption<uint64_t>HELD_ADVANCES;

    SectionDividerOption m_pickup_settings;
    StringOption PICKUP_SEED; 
    TextEditOption PICKUP_SEED_LIST;
    EnumDropdownOption<SeedButton> PICKUP_SEED_BUTTON;
    EnumDropdownOption<BlackoutButton> PICKUP_EXTRA_BUTTON;
    SimpleIntegerOption<uint64_t> PICKUP_SEED_DELAY;
    SimpleIntegerOption<uint64_t>PICKUP_ADVANCES;

    SectionDividerOption m_program_settings;
    EnumDropdownOption<EggProgramState> STARTING_POINT;
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
