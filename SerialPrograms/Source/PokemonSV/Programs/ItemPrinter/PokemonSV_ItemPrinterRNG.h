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
#include "Common/Cpp/Options/EditableTableOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV_ItemPrinterTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


struct ItemPrinterRngRowSnapshot{
    bool chain;
    DateTime date;
    ItemPrinterJobs jobs;
};
class ItemPrinterRngRow : public EditableTableRow, public ConfigOption::Listener{
public:

    ~ItemPrinterRngRow();
    ItemPrinterRngRow();
    ItemPrinterRngRow(bool p_chain, const DateTime& p_date, ItemPrinterJobs p_jobs);

    ItemPrinterRngRowSnapshot snapshot() const;

    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual void value_changed() override;

    void set_seed_based_on_desired_item();

public:
    BooleanCheckBoxCell chain;
    DateTimeCell date;
    EnumDropdownCell<ItemPrinterJobs> jobs;
    EnumDropdownCell<ItemPrinterItems> desired_item;
    ItemPrinterItems prev_desired_item;
};
class ItemPrinterRngTable : public EditableTableOption_t<ItemPrinterRngRow>{
public:
    ItemPrinterRngTable(std::string label);
    virtual std::vector<std::string> make_header() const override;
    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};



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
    void run_print_at_date(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context,
        const DateTime& date, ItemPrinterJobs jobs
    ) const;

    void print_again(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context,
        ItemPrinterJobs jobs
    ) const;


private:
    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint16_t> TOTAL_ROUNDS;

//    DateTimeOption DATE0;
//    DateTimeOption DATE1;
    ItemPrinterRngTable TABLE;

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



