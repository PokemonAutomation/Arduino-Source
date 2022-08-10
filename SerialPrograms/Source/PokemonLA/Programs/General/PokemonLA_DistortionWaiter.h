/*  Distortion Waiter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_DistortionWaiter_H
#define PokemonAutomation_PokemonLA_DistortionWaiter_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
//#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class DistortionWaiter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DistortionWaiter_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class DistortionWaiter : public SingleSwitchProgramInstance2{
public:
    DistortionWaiter();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;


private:
    OCR::LanguageOCR LANGUAGE;

    EventNotificationOption NOTIFICATION_DISTORTION;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
