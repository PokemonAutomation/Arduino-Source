/*  Hyperspace Reward Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_HyperspaceRewardReset_H
#define PokemonAutomation_PokemonLZA_HyperspaceRewardReset_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonLZA/Options/PokemonLZA_HyperspaceRewardTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class HyperspaceRewardReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    HyperspaceRewardReset_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class HyperspaceRewardReset : public SingleSwitchProgramInstance{
public:
    HyperspaceRewardReset();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    OCR::LanguageOCROption LANGUAGE;
    HyperspaceRewardTable TARGET_ITEMS;

    EventNotificationOption NOTIFICATION_REWARD_MATCH;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    void talk_to_trainer(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool check_reward(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
};

}
}
}
#endif



