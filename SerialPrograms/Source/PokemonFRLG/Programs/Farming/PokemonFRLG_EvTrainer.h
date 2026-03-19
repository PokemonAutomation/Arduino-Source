/*  EV Trainer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_EvTrainer_H
#define PokemonAutomation_PokemonFRLG_EvTrainer_H

#include <optional>
#include <string>
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class EvTrainer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EvTrainer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class EvTrainer : public SingleSwitchProgramInstance{
public:
    EvTrainer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    enum class EvTrainingLocation{
        viridianforest,
        route22,
        rocktunnel,
        pokemontower,
        surfspot,
        route1
    };
    
    struct EffortValues{
        uint64_t hp = 0;
        uint64_t attack = 0;
        uint64_t defense = 0;
        uint64_t spatk = 0;
        uint64_t spdef = 0;
        uint64_t speed = 0;
    };

    std::string get_encounter_species(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EvTrainingLocation& location);

    EffortValues get_ev_yield(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::string& species);

    // OCR::LanguageOCROption LANGUAGE;

    SimpleIntegerOption<uint64_t> HP_EVS;
    SimpleIntegerOption<uint64_t> ATK_EVS;
    SimpleIntegerOption<uint64_t> DEF_EVS;
    SimpleIntegerOption<uint64_t> SPATK_EVS;
    SimpleIntegerOption<uint64_t> SPDEF_EVS;
    SimpleIntegerOption<uint64_t> SPEED_EVS;

    BooleanCheckBoxOption STOP_ON_MOVE_LEARN;
    BooleanCheckBoxOption IGNORE_SHINIES;

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



