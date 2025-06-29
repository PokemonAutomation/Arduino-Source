/*  String Select Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A simple editable table of StringSelect options.
 *
 */

#ifndef PokemonAutomation_CommonTools_Options_StringSelectTableOption_H
#define PokemonAutomation_CommonTools_Options_StringSelectTableOption_H

#include "Common/Cpp/Options/EditableTableOption.h"
#include "StringSelectOption.h"

namespace PokemonAutomation{


class StringSelectTableRow : public EditableTableRow{
public:
    StringSelectTableRow(
        EditableTableOption& parent_table,
        const StringSelectDatabase& database, const std::string& default_slug
    )
        : EditableTableRow(parent_table)
        , cell(database, LockMode::LOCK_WHILE_RUNNING, default_slug)
    {
        PA_ADD_OPTION(cell);
    }
    virtual std::unique_ptr<EditableTableRow> clone() const{
        std::unique_ptr<StringSelectTableRow> ret(new StringSelectTableRow(parent(), cell.database(), cell.default_slug()));
        ret->cell.set_by_index(cell.index());
        return ret;
    }

public:
    StringSelectCell cell;
};


class StringSelectTableOption : public EditableTableOption{
public:
    StringSelectTableOption(
        std::string label,
        std::string header, const StringSelectDatabase& database, std::string default_slug
    )
        : EditableTableOption(std::move(label), LockMode::LOCK_WHILE_RUNNING)
        , m_header(std::move(header))
        , m_database(database)
        , m_default_slug(std::move(default_slug))
    {}

    std::vector<std::string> all_slugs() const{
        std::vector<std::string> slugs;
        for (const auto& item : copy_snapshot<StringSelectTableRow>()){
            slugs.emplace_back(item->cell.slug());
        }
        return slugs;
    }

    virtual std::vector<std::string> make_header() const{
        return {m_header};
    }
    virtual std::unique_ptr<EditableTableRow> make_row(){
        return std::make_unique<StringSelectTableRow>(*this, m_database, m_default_slug);
    }

private:
    const std::string m_header;
    const StringSelectDatabase& m_database;
    const std::string m_default_slug;
};



}
#endif
