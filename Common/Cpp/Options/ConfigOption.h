/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_ConfigOption_H
#define PokemonAutomation_Options_ConfigOption_H

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
    struct Listener{
        virtual void value_changed() = 0;
    };
    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);

public:
    ConfigOption(ConfigOption&&) = delete;
    void operator=(ConfigOption&&) = delete;
    virtual ~ConfigOption();

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

    ConfigOptionState visibility() const;
    virtual void set_visibility(ConfigOptionState visibility);


public:
    virtual ConfigWidget* make_ui(QWidget& parent) = 0;

protected:
    void push_update();

private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
