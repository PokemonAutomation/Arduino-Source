/*  Catchability Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Catchability_H
#define PokemonAutomation_PokemonSwSh_Catchability_H

#include "CommonFramework/Options/ConfigOption.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class CatchabilitySelectorOption : public ConfigOption{
public:
    CatchabilitySelectorOption();
    virtual void load_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;

    operator Catchability() const{ return m_current; }
    Catchability value() const{ return m_current; }

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;


private:
    friend class CatchabilitySelectorWidget;
    QString m_label;
    const Catchability m_default;
    Catchability m_current;
};



}
}
}
#endif
