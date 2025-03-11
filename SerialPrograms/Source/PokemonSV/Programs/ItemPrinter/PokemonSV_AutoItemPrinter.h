/*  Auto Item Printer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoItemPrinter_H
#define PokemonAutomation_PokemonSV_AutoItemPrinter_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class AutoItemPrinter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoItemPrinter_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};



class AutoItemPrinter : public SingleSwitchProgramInstance{
public:
    AutoItemPrinter();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint16_t> NUM_ROUNDS;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    void enter_printing_mode(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
};



}
}
}
#endif
