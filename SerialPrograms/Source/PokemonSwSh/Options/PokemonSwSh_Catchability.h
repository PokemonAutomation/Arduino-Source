/*  Catchability Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Catchability_H
#define PokemonAutomation_PokemonSwSh_Catchability_H

#include <QComboBox>
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class CatchabilitySelectorOption : public ConfigOption{
public:
    CatchabilitySelectorOption();
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    operator Catchability() const{ return m_current; }
    Catchability value() const{ return m_current; }

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;


private:
    friend class CatchabilitySelectorUI;
    QString m_label;
    const Catchability m_default;
    Catchability m_current;
};


class CatchabilitySelectorUI : public ConfigOptionUI, public QWidget{
public:
    CatchabilitySelectorUI(QWidget& parent, CatchabilitySelectorOption& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

private:
    CatchabilitySelectorOption& m_value;
    QComboBox* m_box;
};



}
}
}
#endif
