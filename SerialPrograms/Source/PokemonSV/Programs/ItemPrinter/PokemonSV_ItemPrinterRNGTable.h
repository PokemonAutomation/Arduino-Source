/*  Item Printer RNG Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterRNGTable_H
#define PokemonAutomation_PokemonSV_ItemPrinterRNGTable_H

#include <deque>
#include <mutex>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/DateOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "PokemonSV_ItemPrinterTools.h"
#include "PokemonSV_ItemPrinterDatabase.h"

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
    ItemPrinterRngRow(EditableTableOption& parent_table);
    ItemPrinterRngRow(
        EditableTableOption& parent_table,
        bool p_chain, const DateTime& p_date, ItemPrinterJobs p_jobs
    );

    ItemPrinterRngRowSnapshot snapshot() const;

    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual void on_config_value_changed(void* object) override;

//    void set_seed_based_on_desired_item();

public:
    BooleanCheckBoxCell chain;
    DateTimeCell date;
    EnumDropdownCell<ItemPrinterJobs> jobs;
    EnumDropdownCell<ItemPrinter::PrebuiltOptions> desired_item;

private:
    //  Brutal work-around for circular callback dependency.
    SpinLock m_pending_lock;
    std::deque<void*> m_pending;
    std::mutex m_update_lock;
};


class ItemPrinterRngTable : public EditableTableOption_t<ItemPrinterRngRow>{
public:
    ItemPrinterRngTable(std::string label);
    virtual std::vector<std::string> make_header() const override;
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

    friend class ItemPrinterRngRow;
};

struct ItemPrinterDesiredItemRowSnapshot{
    ItemPrinter::PrebuiltOptions item;
    uint16_t quantity;
};

class ItemPrinterDesiredItemRow : public EditableTableRow, public ConfigOption::Listener{
public:
    ItemPrinterDesiredItemRow(EditableTableOption& parent_table);

    ItemPrinterDesiredItemRow(
        EditableTableOption& parent_table,
        ItemPrinter::PrebuiltOptions item, uint16_t quantity
    );

    ItemPrinterDesiredItemRowSnapshot snapshot() const;

    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual void on_config_value_changed(void* object) override;

public:
    EnumDropdownCell<ItemPrinter::PrebuiltOptions> desired_item;
    SimpleIntegerCell<uint16_t> desired_quantity;

};

class ItemPrinterDesiredItemTable : public EditableTableOption_t<ItemPrinterDesiredItemRow>{
public:
    ItemPrinterDesiredItemTable(std::string label);
    virtual std::vector<std::string> make_header() const override;
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

    friend class ItemPrinterDesiredItemRow;
};



}
}
}

#endif
