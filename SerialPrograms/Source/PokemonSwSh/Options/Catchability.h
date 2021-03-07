/*  Catchability Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Catchability_H
#define PokemonAutomation_Catchability_H

#include <QComboBox>
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "CommonFramework/Options/ConfigOption.h"


namespace PokemonAutomation{


class CatchabilitySelector : public ConfigOption{
public:
    CatchabilitySelector();
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    operator Catchability() const{ return m_current; }
    Catchability value() const{ return m_current; }

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;


private:
    friend class CatchabilitySelectorUI;
    const Catchability m_default;
    Catchability m_current;
};


class CatchabilitySelectorUI : public ConfigOptionUI, public QWidget{
public:
    CatchabilitySelectorUI(QWidget& parent, CatchabilitySelector& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

private:
    CatchabilitySelector& m_value;
    QComboBox* m_box;
};



}
#endif
