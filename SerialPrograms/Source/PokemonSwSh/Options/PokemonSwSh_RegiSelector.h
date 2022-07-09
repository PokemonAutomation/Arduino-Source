/*  Regi Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RegiSelector_H
#define PokemonAutomation_PokemonSwSh_RegiSelector_H

#include <QString>
#include "CommonFramework/Options/ConfigOption.h"

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
    virtual void load_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;

    operator RegiGolem() const{ return m_current; }
    RegiGolem value() const{ return m_current; }

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;


private:
    friend class RegiSelectorWidget;
    QString m_label;
    const RegiGolem m_default;
    RegiGolem m_current;
};




}
}
}
#endif
