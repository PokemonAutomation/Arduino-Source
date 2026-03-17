/*  EV Trainer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_EVTrainer_H
#define PokemonAutomation_PokemonFRLG_EVTrainer_H

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

class EVTrainer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EVTrainer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class EVTrainer : public SingleSwitchProgramInstance{
public:
    EVTrainer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    enum class EV{
        hp,
        atk,
        def,
        spatk,
        spdef,
        speed
    };

    uint16_t get_ev_yield(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EV ev);

    // OCR::LanguageOCROption LANGUAGE;

    SimpleIntegerOption<uint64_t> HP_EVS;
    SimpleIntegerOption<uint64_t> ATK_EVS;
    SimpleIntegerOption<uint64_t> DEF_EVS;
    SimpleIntegerOption<uint64_t> SPATK_EVS;
    SimpleIntegerOption<uint64_t> SPDEF_EVS;
    SimpleIntegerOption<uint64_t> SPEED_EVS;

    SimpleIntegerOption<uint64_t> MOVE_PP;

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



