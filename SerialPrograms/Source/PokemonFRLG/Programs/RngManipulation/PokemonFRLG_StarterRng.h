/*  Starter RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_StarterRng_H
#define PokemonAutomation_PokemonFRLG_StarterRng_H

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
#include "PokemonFRLG_RngDisplays.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class StarterRng_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StarterRng_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class StarterRng : public SingleSwitchProgramInstance{
public:
    StarterRng();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    enum class Starter{
        bulbasaur,
        squirtle,
        charmander
    };

    struct StarterRngAdvanceHistory{
        std::vector<double> seed_calibrations;
        std::vector<std::vector<AdvRngState>> results;
    };

    struct StarterRngCalibrationHistory{
        std::vector<double> seed_calibrations;
        std::vector<double> advance_calibrations;
        std::vector<double> continue_screen_adjustments;
        std::vector<AdvRngState> results;

    };

    bool have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t& TARGET_SEED, const AdvRngState& hit);

    AdvObservedPokemon read_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    void update_filters(
        AdvRngFilters& filters, 
        AdvObservedPokemon& pokemon, 
        const StatReads& stats, 
        const EVs& evyield, 
        const BaseStats& BASE_STATS
    );
    std::map<AdvRngState, AdvPokemonResult> get_starter_search_results(
        SingleSwitchProgramEnvironment& env,
        AdvRngSearcher& searcher, 
        AdvRngFilters& filters,
        const std::vector<uint16_t>& SEED_VALUES,
        const uint64_t& ADVANCES, 
        const uint64_t& advances_radius, 
        const AdvObservedPokemon& pokemon
    );
    double get_seed_calibration_frames(
        const StarterRngCalibrationHistory& CALIBRATION_HISTORY,
        const std::vector<uint16_t>& SEED_VALUES, 
        const int16_t& SEED_POSITION
    );
    double get_advances_calibration_frames(
        const StarterRngCalibrationHistory& CALIBRATION_HISTORY, 
        const uint64_t& ADVANCES
    );

    bool update_history(
        SingleSwitchProgramEnvironment& env,
        StarterRngAdvanceHistory& ADVANCE_HISTORY,
        StarterRngCalibrationHistory& CALIBRATION_HISTORY, 
        const uint16_t& MAX_HISTORY_LENGTH,
        const double& SEED_CALIBRATION_FRAMES,
        const double& ADVANCES_CALIBRATION,
        const double& CONTINUE_SCREEN_ADJUSTMENT,
        const std::map<AdvRngState, AdvPokemonResult>& search_hits,
        bool force_finish = false
    );

    bool walk_to_rival_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool auto_battle_rival(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context, 
        AdvObservedPokemon& pokemon,
        AdvRngFilters& filters,
        const BaseStats& BASE_STATS
    );

    bool walk_to_route1_from_lab(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool walk_to_route1_from_home(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    int autolevel_on_route1(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context, 
        AdvObservedPokemon& pokemon,
        AdvRngFilters& filters,
        const BaseStats& BASE_STATS
    );

    
    OCR::LanguageOCROption LANGUAGE;

    EnumDropdownOption<Starter> STARTER;

    SimpleIntegerOption<uint64_t> MAX_RESETS;

    RngFilterDisplay RNG_FILTERS;
    PossibleHitsDisplay POSSIBLE_HITS;

    StringOption SEED; 
    TextEditOption SEED_LIST;
    EnumDropdownOption<SeedButton> SEED_BUTTON;
    EnumDropdownOption<BlackoutButton> EXTRA_BUTTON;
    SimpleIntegerOption<uint64_t> SEED_DELAY;
    
    SimpleIntegerOption<uint64_t>ADVANCES;
    // SimpleIntegerOption<uint64_t>CONTINUE_SCREEN_FRAMES;

    BooleanCheckBoxOption USE_COPYRIGHT_TEXT;

    BooleanCheckBoxOption IGNORE_WILD_SHINIES;

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



