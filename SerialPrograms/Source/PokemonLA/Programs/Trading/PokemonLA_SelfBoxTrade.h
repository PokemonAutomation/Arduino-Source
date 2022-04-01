/*  Self Box Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_SelfBoxTrade_H
#define PokemonAutomation_PokemonLA_SelfBoxTrade_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



class SelfBoxTrade_Descriptor : public MultiSwitchProgramDescriptor{
public:
    SelfBoxTrade_Descriptor();
};


class SelfBoxTrade : public MultiSwitchProgramInstance{
public:
    SelfBoxTrade(const SelfBoxTrade_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(MultiSwitchProgramEnvironment& env) override;

private:
    bool move_to_next(MultiSwitchProgramEnvironment& env, uint8_t& row, uint8_t& col);

private:
    class Stats;

    OCR::LanguageOCR LANGUAGE_LEFT;
    OCR::LanguageOCR LANGUAGE_RIGHT;

    SimpleIntegerOption<uint8_t> BOXES_TO_TRADE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
