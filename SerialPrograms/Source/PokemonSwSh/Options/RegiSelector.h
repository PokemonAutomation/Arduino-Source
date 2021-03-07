/*  Regi Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RegiSelector_H
#define PokemonAutomation_RegiSelector_H

#include <QComboBox>
#include "CommonFramework/Options/ConfigOption.h"


enum class RegiGolem{
    Regirock,
    Regice,
    Registeel,
    Regieleki,
    Regidrago,
};

class RegiSelector : public ConfigOption{
public:
    RegiSelector();
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    operator RegiGolem() const{ return m_current; }
    RegiGolem value() const{ return m_current; }

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;


private:
    friend class RegiSelectorUI;
    const RegiGolem m_default;
    RegiGolem m_current;
};


class RegiSelectorUI : public ConfigOptionUI, public QWidget{
public:
    RegiSelectorUI(QWidget& parent, RegiSelector& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

private:
    RegiSelector& m_value;
    QComboBox* m_box;
};


#endif
