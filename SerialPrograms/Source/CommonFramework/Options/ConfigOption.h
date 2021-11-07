/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConfigOption_H
#define PokemonAutomation_ConfigOption_H

#include <QString>
#include <QJsonValue>
#include <QWidget>
#include "Common/Compiler.h"

namespace PokemonAutomation{


enum class ConfigOptionState{
    ENABLED,
    DISABLED,
    HIDDEN,
};


class ConfigOptionUI;

class ConfigOption{
public:
    virtual ~ConfigOption() = default;

    virtual void load_json(const QJsonValue& json) = 0;
    virtual QJsonValue to_json() const = 0;

    //  Returns error message if invalid. Otherwise returns empty string.
    virtual QString check_validity() const{ return QString(); };

    virtual void restore_defaults(){};

    //  This is called by the framework at the start of a program to reset any
    //  transient state that the option object may have.
    virtual void reset_state(){};

    virtual ConfigOptionUI* make_ui(QWidget& parent) = 0;

public:
    ConfigOptionState visibility = ConfigOptionState::ENABLED;
};


class ConfigOptionUI{
public:
    virtual ~ConfigOptionUI() = default;
    ConfigOptionUI(ConfigOption& m_value, QWidget& widget)
        : m_value(m_value)
        , m_widget(widget)
    {
        ConfigOptionUI::update_visibility();
    }

    const ConfigOption& option() const{ return m_value; }
    ConfigOption& option(){ return m_value; }

    QWidget& widget(){ return m_widget; }
    virtual void restore_defaults() = 0;

    virtual void update_visibility(){
        switch (m_value.visibility){
        case ConfigOptionState::ENABLED:
            m_widget.setEnabled(true);
            m_widget.setVisible(true);
            break;
        case ConfigOptionState::DISABLED:
            m_widget.setEnabled(false);
            m_widget.setVisible(true);
            break;
        case ConfigOptionState::HIDDEN:
            m_widget.setEnabled(false);
            m_widget.setVisible(false);
            break;
        }
    }

private:
    ConfigOption& m_value;
    QWidget& m_widget;
};


}
#endif
