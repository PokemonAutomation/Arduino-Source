/*  Item Printer RNG
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterRNG_H
#define PokemonAutomation_PokemonSV_ItemPrinterRNG_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/DateOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class ItemPrinterRNG_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ItemPrinterRNG_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class ItemPrinterRNG : public SingleSwitchProgramInstance{
public:
    ItemPrinterRNG();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;


private:
    void run_print(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context,
        const DateTime& date, uint8_t jobs
    ) const;


private:
    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint16_t> TOTAL_ROUNDS;

    DateTimeOption DATE0;
    DateTimeOption DATE1;

    SimpleIntegerOption<uint16_t> DELAY_MILLIS;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    BooleanCheckBoxOption FIX_TIME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif



