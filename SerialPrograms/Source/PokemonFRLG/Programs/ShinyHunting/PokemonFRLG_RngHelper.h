/*  RNG Helper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngHelper_H
#define PokemonAutomation_PokemonFRLG_RngHelper_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class RngHelper_Descriptor : public SingleSwitchProgramDescriptor{
public:
    RngHelper_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class RngHelper : public SingleSwitchProgramInstance{
public:
    RngHelper();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    enum class ResetType{
        hard,
        soft,
    };
    enum class Target{
        starters,
        magikarp,
        hitmon,
        eevee,
        lapras,
        fossils,
        gamecornerabra,
        gamecornerclefairy,
        gamecornerdratini,
        gamecornerbug,
        gamecornerporygon,
        // togepi,
        staticencounter,
        snorlax,
        mewtwo,
        hooh,
        hypno,
        sweetscent,
        fishing,
        safarizonecenter,
        safarizoneeast,
        safarizonenorth,
        safarizonewest,
        safarizonesurf,
        safarizonefish,
        // roaming
    };

    EnumDropdownOption<Target> TARGET;

    SimpleIntegerOption<uint64_t> NUM_RESETS;

    SimpleIntegerOption<uint64_t> SEED_DELAY;
    SimpleIntegerOption<int64_t> SEED_CALIBRATION;

    SimpleIntegerOption<uint64_t> LOAD_ADVANCES;
    FloatingPointOption LOAD_CALIBRATION;

    SimpleIntegerOption<uint64_t> INGAME_ADVANCES;
    FloatingPointOption INGAME_CALIBRATION;

    BooleanCheckBoxOption USE_COPYRIGHT_TEXT;
    BooleanCheckBoxOption USE_TEACHY_TV;

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



