/*  Item Printer RNG Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Qt/TimeQt.h"
#include "PokemonSV_ItemPrinterRNGTable.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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



// - if desired_item has changed, set the seed (and number of jobs) accordingly
// - if any of date/jobs/chain changes, check if the date and jobs match one of ItemPrinterEnumOption, 
// and that chain is disabled. If so, set the desired item to the enum_value. Else set desired_item to NONE.
//  - also, hide the date if chain enabled.
// - trigger the listener for the parent table.
void ItemPrinterRngRow::on_config_value_changed(void* object){
    //  This is really ugly due to the circular update dependency.
    //  TODO: Redesign this.

    {
        WriteSpinLock lg1(m_pending_lock);
        m_pending.emplace_back(object);
    }

    bool keep_going;
    do{
        std::unique_lock<std::mutex> lg(m_update_lock, std::try_to_lock_t());
        if (!lg.owns_lock()){
            return;
        }
        {
            WriteSpinLock lg1(m_pending_lock);
            object = m_pending.front();
            m_pending.pop_front();
        }

        ItemPrinterRngTable& table = static_cast<ItemPrinterRngTable&>(parent());

        if (object == &desired_item){
            ItemPrinter::PrebuiltOptions option = desired_item;
            if (option != ItemPrinter::PrebuiltOptions::NONE){
                const ItemPrinter::ItemPrinterEnumOption& option_data = option_lookup_by_enum(option);
                chain = false;
                jobs.set(option_data.jobs);
                date.set(from_seconds_since_epoch(option_data.seed));
            }
        }else if (object == &date || object == &jobs || object == &chain){
            date.set_visibility(chain ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);
            const ItemPrinter::ItemPrinterEnumOption* option_data = ItemPrinter::option_lookup_by_seed(to_seconds_since_epoch(date));
//            cout << "option_data = " << option_data << endl;
            // seed found in the PrebuiltOptions table, and jobs number matches and chain disabled
            if (option_data != nullptr && option_data->jobs == jobs && !chain){
                desired_item.set(option_data->enum_value);
            }else{
//                cout << "option_data->jobs = " << (size_t)option_data->jobs << ", jobs = " << jobs.current_value() << ", chain = " << chain << endl;
                desired_item.set(ItemPrinter::PrebuiltOptions::NONE);
            }
        }

        table.report_value_changed(object);

        WriteSpinLock lg1(m_pending_lock);
        keep_going = !m_pending.empty();
    }while (keep_going);
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



ItemPrinterDesiredItemRow::ItemPrinterDesiredItemRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , desired_item(
        ItemPrinter::PrebuiltOptions_AutoMode_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ItemPrinter::PrebuiltOptions::ABILITY_PATCH
    )
    , desired_quantity(
        LockMode::UNLOCK_WHILE_RUNNING,
        999, 1, 999
    )
{
    PA_ADD_OPTION(desired_item);
    PA_ADD_OPTION(desired_quantity);

}

ItemPrinterDesiredItemRow::ItemPrinterDesiredItemRow(
    EditableTableOption& parent_table,
    ItemPrinter::PrebuiltOptions item, uint16_t quantity
)
    : ItemPrinterDesiredItemRow(parent_table)
{
    desired_item.set(item);
    desired_quantity.set(quantity);
}

ItemPrinterDesiredItemRowSnapshot ItemPrinterDesiredItemRow::snapshot() const{
    
    return ItemPrinterDesiredItemRowSnapshot{desired_item, desired_quantity};
}


std::unique_ptr<EditableTableRow> ItemPrinterDesiredItemRow::clone() const{
    std::unique_ptr<ItemPrinterDesiredItemRow> ret(new ItemPrinterDesiredItemRow(parent()));
    ret->desired_item.set(desired_item);
    ret->desired_quantity.set(desired_quantity);
    return ret;
}


void ItemPrinterDesiredItemRow::on_config_value_changed(void* object){

}

ItemPrinterDesiredItemTable::ItemPrinterDesiredItemTable(std::string label)
    : EditableTableOption_t<ItemPrinterDesiredItemRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING
    )
{
    //  Need to do this separately because this prematurely accesses the table.
    set_default(make_defaults());
    restore_defaults();
}
std::vector<std::string> ItemPrinterDesiredItemTable::make_header() const{
    return std::vector<std::string>{
        "Desired Item",
        "Quantity",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> ItemPrinterDesiredItemTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<ItemPrinterDesiredItemRow>(*this, ItemPrinter::PrebuiltOptions::ABILITY_PATCH, (uint16_t)999));
    ret.emplace_back(std::make_unique<ItemPrinterDesiredItemRow>(*this, ItemPrinter::PrebuiltOptions::EXP_CANDY, (uint16_t)999));
    return ret;
}


}
}
}
