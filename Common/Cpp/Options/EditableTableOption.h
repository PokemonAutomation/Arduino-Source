/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_EditableTableOption_H
#define PokemonAutomation_Options_EditableTableOption_H

#include <memory>
#include <vector>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


//  Represents a row of the table. Users should subclass this and add all the
//  configs/fields that is in the row.
class EditableTableRow{
public:
    virtual ~EditableTableRow() = default;
    EditableTableRow();

    //  Duplicate/deep-copy the entire row. Does not copy over listeners.
    virtual std::unique_ptr<EditableTableRow> clone() const = 0;

    virtual void load_json(const JsonValue& json);
    virtual JsonValue to_json() const;

    virtual std::string check_validity() const;


protected:
    void add_option(ConfigOption& option, std::string serialization_string);

#define PA_ADD_OPTION(x)    add_option(x, #x)


public:
    //  Internal use by table.
    uint64_t seqnum() const{ return m_seqnum; }
    size_t index() const{ return m_index.load(std::memory_order_relaxed); }
    std::vector<ConfigOption*> make_cells();


private:
    friend class EditableTableOption;

    //  A unique # for this row within its table.
    uint64_t m_seqnum = 0;

    //  The index in the table it resides in.
    //  -1 means it's orphaned and not in the table.
    std::atomic<size_t> m_index;

    std::vector<std::pair<std::string, ConfigOption*>> m_options;
};




//  This is the table itself.
class EditableTableOption : public ConfigOption{
public:
    EditableTableOption(
        std::string label,
        std::vector<std::unique_ptr<EditableTableRow>> default_value = {}
    );

    const std::string& label() const{ return m_label; }

    //  Returns the # of rows at this moment of time.
    //  Since this value can be out-of-date before you return, do not use
    //  this for bound-checking where it's unsafe to read out-of-bounds.
    size_t current_rows() const;

    //  Return a list of references to all the rows at this exact moment.
    //  These reference are live in that they may be asynchronously changed.
    std::vector<std::shared_ptr<EditableTableRow>> current_refs() const;

    //  Return a copy of the entire table at the exact moment this is called.
    template <typename RowType>
    std::vector<std::unique_ptr<RowType>> copy_snapshot() const{
        std::vector<std::unique_ptr<RowType>> ret;
        SpinLockGuard lg(m_lock);
        ret.reserve(m_current.size());
        for (auto& item : m_current){
            std::unique_ptr<EditableTableRow> parent = item->clone();
            std::unique_ptr<RowType> ptr(static_cast<RowType*>(parent.release()));
            ret.emplace_back(std::move(ptr));
        }
        return ret;
    }

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override final;

public:
    virtual std::vector<std::string> make_header() const = 0;
    virtual std::unique_ptr<EditableTableRow> make_row() const = 0;

    //  Undefined behavior to call these on rows that aren't part of the table.
    void insert_row(size_t index, std::unique_ptr<EditableTableRow> row);
    void clone_row(const EditableTableRow& row);
    void remove_row(EditableTableRow& row);

public:
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    const std::string m_label;
    const std::vector<std::unique_ptr<EditableTableRow>> m_default;

    mutable SpinLock m_lock;
    uint64_t m_seqnum = 0;
    std::vector<std::shared_ptr<EditableTableRow>> m_current;
};


//  Convenience helper class that's type-aware.
template <typename RowType>
class EditableTableOption_t : public EditableTableOption{
public:
    using EditableTableOption::EditableTableOption;

    std::vector<std::unique_ptr<RowType>> copy_snapshot() const{
        return EditableTableOption::copy_snapshot<RowType>();
    }

    virtual std::unique_ptr<EditableTableRow> make_row() const override{
        return std::unique_ptr<EditableTableRow>(new RowType());
    }
};




}
#endif
