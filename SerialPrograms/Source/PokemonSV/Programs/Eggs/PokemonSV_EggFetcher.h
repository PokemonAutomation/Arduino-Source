/*  Egg Fetcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_EggFetcher_H
#define PokemonAutomation_PokemonSV_EggFetcher_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV/Options/PokemonSV_EggPowerSandwichOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class EggFetcher_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggFetcher_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class EggFetcher : public SingleSwitchProgramInstance{
public:
    EggFetcher();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCROption LANGUAGE;

    SimpleIntegerOption<uint16_t> EGGS_TO_FETCH;
    EggPowerSandwichOption EGG_SANDWICH;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
