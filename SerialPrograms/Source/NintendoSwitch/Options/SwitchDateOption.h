/*  Switch Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SwitchDate_H
#define PokemonAutomation_SwitchDate_H

#include "Common/Qt/Options/ConfigOption.h"
#include "Common/Qt/Options/SwitchDate/SwitchDateBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchDateOption : public ConfigOption, private SwitchDateBaseOption{
public:
    SwitchDateOption(
        std::string label,
        QDate default_value
    );

    using SwitchDateBaseOption::label;
    using SwitchDateBaseOption::operator QDate;
    using SwitchDateBaseOption::get;
    using SwitchDateBaseOption::set;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class SwitchDateWidget;
};




}
}
#endif

