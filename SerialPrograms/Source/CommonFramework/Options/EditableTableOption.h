/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EditableTableOption_H
#define PokemonAutomation_EditableTableOption_H

#include "Common/Qt/Options/EditableTable/EditableTableBaseOption.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class EditableTableOption : public ConfigOption, public EditableTableBaseOption{
public:
    EditableTableOption(
        QString label, const EditableTableFactory& factory, bool margin,
        std::vector<std::unique_ptr<EditableTableRow>> default_value = {}
    );

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual QString check_validity() const override;
    virtual void restore_defaults() override final;

    virtual ConfigWidget* make_ui(QWidget& parent) override;
};





}
#endif
