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

    AdvObservedPokemon read_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    void update_filter_display(AdvRngFilters& filters);
    void update_search_results(std::map<AdvRngState, AdvPokemonResult>& possible_hits);

    void walk_to_rival_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Starter STARTER);
    void auto_battle_rival(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AdvObservedPokemon& pokemon);

    void walk_to_route1_from_lab(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void walk_home_from_route1(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void heal_at_home(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void walk_to_route1_from_home(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool autolevel_on_route1(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AdvObservedPokemon& pokemon);

    
    OCR::LanguageOCROption LANGUAGE;

    EnumDropdownOption<Starter> STARTER;

    SimpleIntegerOption<uint64_t> MAX_RESETS;

    RngFilterDisplay RNG_FILTERS;
    PossibleHitsDisplay POSSIBLE_HITS;

    StringOption SEED; 
    TextEditOption SEED_LIST;
    EnumDropdownOption<SeedButton> SEED_BUTTON;
    SimpleIntegerOption<uint64_t> SEED_DELAY;
    
    SimpleIntegerOption<uint64_t>ADVANCES;
    SimpleIntegerOption<uint64_t>CONTINUE_SCREEN_FRAMES;

    BooleanCheckBoxOption USE_COPYRIGHT_TEXT;

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



