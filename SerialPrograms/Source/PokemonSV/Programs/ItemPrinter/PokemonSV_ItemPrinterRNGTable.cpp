/*  Item Printer RNG Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/TimeQt.h"
#include "PokemonSV_ItemPrinterRNGTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



ItemPrinterRngRow::~ItemPrinterRngRow(){
    chain.remove_listener(*this);
    date.remove_listener(*this);
    jobs.remove_listener(*this);
    desired_item.remove_listener(*this);
}
ItemPrinterRngRow::ItemPrinterRngRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , chain(LockMode::UNLOCK_WHILE_RUNNING, false)
    , date(
        LockMode::UNLOCK_WHILE_RUNNING,
        DateTimeOption::DATE_HOUR_MIN_SEC,
        DateTime{2000, 1, 1, 0, 1, 0},
        DateTime{2060, 12, 31, 23, 59, 59},
        DateTime{2024,  6,  4,  0, 37,  8}
    )
    , jobs(
        ItemPrinterJobs_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ItemPrinterJobs::Jobs_1
    )
    , desired_item(
        ItemPrinter::PrebuiltOptions_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ItemPrinter::PrebuiltOptions::BALL_BONUS
    )
{
    PA_ADD_OPTION(chain);
    PA_ADD_OPTION(date);
    PA_ADD_OPTION(jobs);
    PA_ADD_OPTION(desired_item);

    chain.add_listener(*this);
    date.add_listener(*this);
    jobs.add_listener(*this);
    desired_item.add_listener(*this);
}
ItemPrinterRngRow::ItemPrinterRngRow(
    EditableTableOption& parent_table,
    bool p_chain, const DateTime& p_date, ItemPrinterJobs p_jobs
)
    : ItemPrinterRngRow(parent_table)
{
    chain = p_chain;
    date.set(p_date);
    jobs.set(p_jobs);
}
ItemPrinterRngRowSnapshot ItemPrinterRngRow::snapshot() const{
    return ItemPrinterRngRowSnapshot{chain, date, jobs};
}
std::unique_ptr<EditableTableRow> ItemPrinterRngRow::clone() const{
    std::unique_ptr<ItemPrinterRngRow> ret(new ItemPrinterRngRow(parent()));
    ret->chain = (bool)chain;
    ret->date.set(date);
    ret->jobs.set(jobs);
    return ret;
}



// - always update the date's visibility when chain is changed.
// - if desired_item has changed, set the seed (and number of jobs) accordingly
// - if any other value changes, set desired_item to NONE
void ItemPrinterRngRow::value_changed(void* object){
    date.set_visibility(chain ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);

    ItemPrinterRngTable& table = static_cast<ItemPrinterRngTable&>(parent());

    if (object == &desired_item){
        ItemPrinter::PrebuiltOptions option = desired_item;
        if (option != ItemPrinter::PrebuiltOptions::NONE){
            const ItemPrinter::ItemPrinterEnumOption& option_data = option_lookup_by_enum(option);
            chain = false;
            date.set(from_seconds_since_epoch(option_data.seed));
            jobs.set(option_data.jobs);
        }
    }else if (object == &chain){
        if (chain){
            desired_item.set(ItemPrinter::PrebuiltOptions::NONE);
        }
    }else if (object == &date){
        const ItemPrinter::ItemPrinterEnumOption* option_data = ItemPrinter::option_lookup_by_seed(to_seconds_since_epoch(date));
        if (option_data == nullptr){
            desired_item.set(ItemPrinter::PrebuiltOptions::NONE);
        }else{
            desired_item.set(option_data->enum_value);
        }
    }

    table.report_value_changed(object);
}

ItemPrinterRngTable::ItemPrinterRngTable(std::string label)
    : EditableTableOption_t<ItemPrinterRngRow>(
        std::move(label),
        LockMode::UNLOCK_WHILE_RUNNING
    )
{
    //  Need to do this separately because this prematurely accesses the table.
    set_default(make_defaults());
    restore_defaults();
}
std::vector<std::string> ItemPrinterRngTable::make_header() const{
    return std::vector<std::string>{
        "Continue Previous?",
        "Date Seed",
        "Jobs to Print",
        "Desired Item",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> ItemPrinterRngTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2024,  6,  4,  0, 37,  8}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2049,  8, 18, 23, 51,  8}, ItemPrinterJobs::Jobs_10));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2024,  6,  4,  0, 37,  8}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2031, 10,  8,  7,  9,  9}, ItemPrinterJobs::Jobs_10));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2024,  6,  4,  0, 37,  8}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2020,  3,  3,  6, 38, 18}, ItemPrinterJobs::Jobs_10));
    return ret;
}


}
}
}
