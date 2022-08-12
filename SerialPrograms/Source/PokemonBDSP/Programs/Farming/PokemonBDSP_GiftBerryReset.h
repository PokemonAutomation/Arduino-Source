/*  Gift Berry Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GiftBerryReset_H
#define PokemonAutomation_PokemonBDSP_GiftBerryReset_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "PokemonBDSP/Options/PokemonBDSP_BerrySelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class GiftBerryReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GiftBerryReset_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class GiftBerryReset : public SingleSwitchProgramInstance{
public:
    GiftBerryReset();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCR LANGUAGE;
    
    BerrySelectorOption TARGET_BERRIES;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
