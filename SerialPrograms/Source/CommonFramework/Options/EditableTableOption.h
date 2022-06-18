/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EditableTableOption_H
#define PokemonAutomation_EditableTableOption_H

#include "Common/Qt/Options/EditableTable/EditableTableBaseOption.h"
#include "ConfigOption.h"

class QWidget;

namespace PokemonAutomation{


class EditableTableOption : public ConfigOption, public EditableTableBaseOption{
public:
    EditableTableOption(
        QString label, const EditableTableFactory& factory,
        std::vector<std::unique_ptr<EditableTableRow>> default_value = {}
    );

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual QString check_validity() const override;
    virtual void restore_defaults() override final;

    virtual ConfigWidget* make_ui(QWidget& parent) override;
};

QWidget* make_boolean_table_cell(QWidget& parent, bool& value);

template<typename T> QWidget* make_integer_table_cell(QWidget& parent, T& value);

QWidget* make_double_table_cell(QWidget& parent, double& value, double min, double max);

QWidget* make_limited_integer_table_cell(QWidget& parent, uint8_t& value, uint8_t min, uint8_t max);

// See EditableTableOption-EnumTableCell.h for template<typename T> QWidget* make_enum_table_cell(QWidget& parent, T& value);





}
#endif
