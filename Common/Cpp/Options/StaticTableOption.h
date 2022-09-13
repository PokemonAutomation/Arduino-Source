/*  Static Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StaticTableOption_H
#define PokemonAutomation_Options_StaticTableOption_H

#include <memory>
#include <vector>
#include <map>
#include "ConfigOption.h"

namespace PokemonAutomation{


//  Represents a row of the table. Users should subclass this and add all the
//  configs/fields that is in the row.
class StaticTableRow{
public:
    virtual ~StaticTableRow() = default;
    StaticTableRow(std::string slug);

    const std::string& slug() const{ return m_slug; }

    virtual void load_json(const JsonValue& json);
    virtual JsonValue to_json() const;

    virtual std::string check_validity() const;
    virtual void restore_defaults();


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
    StaticTableOption(std::string label);
protected:
    //  Construction Steps:

    //  Call this to populate the rows.
    void add_row(std::unique_ptr<StaticTableRow> row);

    //  You must call this when done populating rows. Afterwards, you cannot add more.
    void finish_construction();

public:
    const std::string& label() const{ return m_label; }
    const std::vector<std::unique_ptr<StaticTableRow>>& table() const{ return m_table; }

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override final;

public:
    virtual std::vector<std::string> make_header() const = 0;

public:
    virtual ConfigWidget* make_ui(QWidget& parent) override;




private:
    const std::string m_label;
    std::vector<std::unique_ptr<StaticTableRow>> m_table;

    bool m_finished = false;
    std::map<std::string, size_t> m_index_map;
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
