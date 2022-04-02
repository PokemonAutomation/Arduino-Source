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


class DistortionWaiter_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    DistortionWaiter_Descriptor();
};


class DistortionWaiter : public SingleSwitchProgramInstance{
public:
    DistortionWaiter(const DistortionWaiter_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;


private:
    class Stats;

    OCR::LanguageOCR LANGUAGE;

    EventNotificationOption NOTIFICATION_DISTORTION;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
