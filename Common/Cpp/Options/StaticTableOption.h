/*  Static Table Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_StaticTableOption_H
#define PokemonAutomation_Options_StaticTableOption_H

#include <memory>
#include <vector>
#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


//  Represents a row of the table. Users should subclass this and add all the
//  configs/fields that is in the row.
class StaticTableRow{
    StaticTableRow(const StaticTableRow&) = delete;
    void operator=(const StaticTableRow&) = delete;
public:
    virtual ~StaticTableRow() = default;
    StaticTableRow(std::string slug);

    const std::string& slug() const{ return m_slug; }

    virtual void load_json(const JsonValue& json);
    virtual JsonValue to_json() const;

    virtual std::string check_validity() const;
    virtual void restore_defaults();

    void report_program_state(bool program_is_running);


protected:
    void add_option(ConfigOption& option, std::string serialization_string);

#define PA_ADD_STATIC(x)    add_option(x, "")
#define PA_ADD_OPTION(x)    add_option(x, #x)


public:
    //  Internal use by table.
    std::vector<ConfigOption*> make_cells();


private:
    friend class StaticTableOption;
    std::string m_slug;
    std::vector<std::pair<std::string, ConfigOption*>> m_options;
};




//  This is the table itself.
class StaticTableOption : public ConfigOption{
public:
    ~StaticTableOption();
    StaticTableOption(
        std::string label,
        LockMode lock_while_program_is_running,
        bool enable_saveload = true
    );
protected:
    //  Construction Steps:

    //  Call this to populate the rows.
    void add_row(std::unique_ptr<StaticTableRow> row);
    void add_row(StaticTableRow* row);

    //  You must call this when done populating rows. Afterwards, you cannot add more.
    void finish_construction();

public:
    const std::string& label() const;
    const std::vector<StaticTableRow*>& table() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override final;

    virtual void report_program_state(bool program_is_running) override;

public:
    bool saveload_enabled() const;
    virtual std::vector<std::string> make_header() const = 0;

public:
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};


#if 0
//  Convenience helper class that's type-aware.
template <typename RowType>
class StaticTableOption_t : public StaticTableOption{
public:
    using StaticTableOption::StaticTableOption;


};
#endif






}
#endif
