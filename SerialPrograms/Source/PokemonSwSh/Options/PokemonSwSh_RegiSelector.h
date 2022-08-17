/*  Regi Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RegiSelector_H
#define PokemonAutomation_PokemonSwSh_RegiSelector_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


enum class RegiGolem{
    Regirock,
    Regice,
    Registeel,
    Regieleki,
    Regidrago,
};

class RegiSelectorOption : public ConfigOption{
public:
    RegiSelectorOption();
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    operator RegiGolem() const{ return (RegiGolem)m_option.current_index(); }

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;


private:
    DropdownOption m_option;
};




}
}
}
#endif
