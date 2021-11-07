/*  Regi Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RegiSelector_H
#define PokemonAutomation_PokemonSwSh_RegiSelector_H

#include <QComboBox>
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
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    operator RegiGolem() const{ return m_current; }
    RegiGolem value() const{ return m_current; }

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;


private:
    friend class RegiSelectorOptionUI;
    QString m_label;
    const RegiGolem m_default;
    RegiGolem m_current;
};


class RegiSelectorOptionUI : public QWidget, public ConfigOptionUI{
public:
    RegiSelectorOptionUI(QWidget& parent, RegiSelectorOption& value);
    virtual void restore_defaults() override;

private:
    RegiSelectorOption& m_value;
    QComboBox* m_box;
};



}
}
}
#endif
