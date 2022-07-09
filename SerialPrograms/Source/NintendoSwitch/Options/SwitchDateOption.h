/*  Switch Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SwitchDate_H
#define PokemonAutomation_SwitchDate_H

#include "Common/Qt/Options/SwitchDate/SwitchDateBaseOption.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchDateOption : public ConfigOption, private SwitchDateBaseOption{
public:
    SwitchDateOption(
        QString label,
        QDate default_value
    );

    using SwitchDateBaseOption::label;
    using SwitchDateBaseOption::operator QDate;
    using SwitchDateBaseOption::get;
    using SwitchDateBaseOption::set;

    virtual void load_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;

    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class SwitchDateWidget;
};




}
}
#endif

