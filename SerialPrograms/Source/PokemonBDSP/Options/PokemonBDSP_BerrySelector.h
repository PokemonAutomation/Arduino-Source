/*  Berry Selector, UI component to select multiple berries
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BerrySelector_H
#define PokemonAutomation_PokemonBDSP_BerrySelector_H

#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

class BerrySelectorRow : public EditableTableRow{
public:
    BerrySelectorRow();
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    QWidget* make_berry_box(QWidget& parent);

public:
    std::string berry_slug;
};

class BerrySelectorOptionFactory : public EditableTableFactory{
public:
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};


class BerrySelectorOption : public ConfigOption{
public:
    BerrySelectorOption(std::string label);

    // Whether berry_slug is among the selected berries.
    bool find_berry(const std::string& berry_slug) const;
    // Return the berry slugs that the user has selected via the berry table UI.
    std::vector<std::string> selected_berries() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults() const;

private:
    BerrySelectorOptionFactory m_factory;
    EditableTableOption m_table;
};





}
}
}
#endif
