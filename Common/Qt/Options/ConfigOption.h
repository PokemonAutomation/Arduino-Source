/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConfigOption_H
#define PokemonAutomation_ConfigOption_H

#include <string>
#include "Common/Cpp/Pimpl.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;
class ConfigWidget;


enum class ConfigOptionState{
    ENABLED,
    DISABLED,
    HIDDEN,
};


// An option of a program, like the number of boxes of eggs to hatch,
// the number of frames to skip, or what type of pokeballs to throw.
// It is responsible for setting the UI (by calling make_ui()) of this option.
// It also uses load_json() and to_json() to load and save the option to
// a json file, so that the program can remember what user has selected.
class ConfigOption{
public:
    ConfigOption(ConfigOption&&) = delete;
    void operator=(ConfigOption&&) = delete;
    virtual ~ConfigOption();

public:
    struct Listener{
        virtual void value_changed() = 0;
    };
    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);

public:
    ConfigOption();

    virtual void load_json(const JsonValue& json) = 0;
    virtual JsonValue to_json() const = 0;

    //  Returns error message if invalid. Otherwise returns empty string.
    virtual std::string check_validity() const;

    virtual void restore_defaults(){};

    //  This is called by the framework at the start of a program to reset any
    //  transient state that the option object may have.
    virtual void reset_state(){};


public:
    virtual ConfigWidget* make_ui(QWidget& parent) = 0;

public:
    ConfigOptionState visibility = ConfigOptionState::ENABLED;

protected:
    void push_update();

private:
    struct Listeners;
    Pimpl<Listeners> m_listeners;
};


class ConfigWidget{
public:
    virtual ~ConfigWidget() = default;
    ConfigWidget(ConfigOption& m_value);
    ConfigWidget(ConfigOption& m_value, QWidget& widget);

    const ConfigOption& option() const{ return m_value; }
    ConfigOption& option(){ return m_value; }

    QWidget& widget(){ return *m_widget; }

    virtual void restore_defaults() = 0;
    virtual void update_ui();
    virtual void update_visibility();

protected:
    ConfigOption& m_value;
    QWidget* m_widget = nullptr;
};


}
#endif
