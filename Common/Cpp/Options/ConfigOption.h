/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_ConfigOption_H
#define PokemonAutomation_Options_ConfigOption_H

#include <string>
#include "Common/Compiler.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/Containers/Pimpl.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;
class ConfigWidget;


enum class LockWhileRunning{
    UNLOCKED,
    LOCK_WHILE_RUNNING,
    READ_ONLY,
};
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
        virtual void value_changed(){}
        virtual void visibility_changed(){}
        virtual void program_state_changed(bool program_is_running){}
    };
    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);
    size_t total_listeners() const;

public:
    virtual ~ConfigOption();
    ConfigOption(ConfigOption&&) = delete;
    void operator=(ConfigOption&&) = delete;
protected:
    ConfigOption(const ConfigOption& x);

public:
    ConfigOption();
    ConfigOption(LockWhileRunning lock_mode);
    ConfigOption(ConfigOptionState visibility);

//    //  Deep copy this entire config. This will not copy listeners.
//    //  Returns null if config cannot be copied.
//    virtual std::unique_ptr<ConfigOption> clone() const = 0;

    virtual void load_json(const JsonValue& json);
    virtual JsonValue to_json() const;

    //  Lifetime sanitizer
    void check_usage() const{
        m_lifetime_sanitizer.check_usage();
    }

public:
    LockWhileRunning lock_mode() const;

    //  Returns error message if invalid. Otherwise returns empty string.
    virtual std::string check_validity() const;

    virtual void restore_defaults();

    //  This is called by the framework at the start of a program to reset any
    //  transient state that the option object may have.
    virtual void reset_state(){};

    ConfigOptionState visibility() const;
    virtual void set_visibility(ConfigOptionState visibility);


public:
    //  Report that the visibility has changed. Attached UI elements should
    //  respond accordingly to show, hide, or lock this element.
    virtual void report_visibility_changed();

    //  Report that the program state has changed. This will cause UI elements
    //  to lock/unlock depending on whether they are allowed to be changed by
    //  the user while the program is running.
    virtual void report_program_state(bool program_is_running);

protected:
    //  Report that the value of this config has changed. This will be pushed to
    //  all listeners.
    void report_value_changed();


public:
    virtual ConfigWidget* make_QtWidget(QWidget& parent) = 0;

private:
    struct Data;
    Pimpl<Data> m_data;

    LifetimeSanitizer m_lifetime_sanitizer;
};




}
#endif
