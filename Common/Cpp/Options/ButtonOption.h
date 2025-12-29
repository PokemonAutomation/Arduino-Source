/*  Button Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_ButtonOption_H
#define PokemonAutomation_Options_ButtonOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


struct ButtonListener{
    virtual void on_press() = 0;
};


class ButtonCell : public ConfigOptionImpl<ButtonCell>{
public:
    enum Enabled : bool{
        DISABLED,
        ENABLED,
    };


public:
    virtual ~ButtonCell();
    ButtonCell(const ButtonCell& x);
    ButtonCell(
        std::string text,
        int button_height = 0,
        int text_size = 0
    );
    ButtonCell(
        Enabled state,
        std::string text,
        int button_height = 0,
        int text_size = 0
    );

    using ConfigOption::add_listener;
    using ConfigOption::remove_listener;
    void add_listener(ButtonListener& listener);
    void remove_listener(ButtonListener& listener);


public:
    bool is_enabled() const;
    void set_enabled(bool enabled);

    std::string text() const;
    void set_text(std::string text);

    int button_height() const;
    int text_size() const;

    virtual void press_button();

//    virtual void load_json(const JsonValue& json) override;
//    virtual JsonValue to_json() const override;

//    virtual void restore_defaults() override;



protected:
    struct Data;
    Pimpl<Data> m_data;
};



class ButtonOption : public ConfigOptionImpl<ButtonOption, ButtonCell>{
public:
    virtual ~ButtonOption();
    ButtonOption(const ButtonOption& x) = delete;
    ButtonOption(
        std::string label,  // the description of the button, shown on the left side of the button
        std::string text,   // the text appearing on the button
        int button_height = 0,
        int text_size = 0
    );
    ButtonOption(
        std::string label,  // the description of the button, shown on the left side of the button
        Enabled state,
        std::string text,   // the text appearing on the button
        int button_height = 0,
        int text_size = 0
    );

    std::string label() const;
    void set_label(std::string label);


private:
    struct Data;
    Pimpl<Data> m_data;
};



//  A button that allows graceful stopping after completing the iteration in a looping program.
//  This button manages its own state and can be queried to determine if a stop has been requested
//  by user.
//
//  Usage:
//    1. Add as a member: DeferredStopButtonOption STOP_BUTTON;
//    2. In constructor: PA_ADD_OPTION(STOP_BUTTON);
//    3. At program start:
//       DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_BUTTON);
//    4. In main loop: if (STOP_BUTTON.should_stop()) break;
//    5. On program exit: ResetOnExit will automatically reset the button via RAII
class DeferredStopButtonOption : public ButtonOption{
public:
    virtual ~DeferredStopButtonOption();
    DeferredStopButtonOption(const DeferredStopButtonOption& x) = delete;

    //  iteration_name: The name of the unit of work (e.g., "Batch", "Round", "Iteration")
    //  Used to construct button text like "Stop after Current Batch"
    //  Can be capitalized or uncapitalized.
    DeferredStopButtonOption(
        const std::string& iteration_name = "Iteration",
        int button_height = 0,
        int text_size = 16
    );

    //  RAII guard to automatically reset button when program exits
    //  Define this guard at beginning of a program running session.
    class ResetOnExit{
    public:
        ResetOnExit(DeferredStopButtonOption& button);
        ~ResetOnExit();
    private:
        DeferredStopButtonOption& m_button;
    };

    //  Called by the program to check if user has pressed the button to
    //  request a stop.
    bool should_stop() const;

    virtual void press_button() override;

private:
    //  Reset the button to be ready to press during a program run.
    //  This is usually called by ResetOnExit.
    void reset_and_ready();

    //  Set button to idle statle after whena program finishes running.
    //  This is usually called by ResetOnExit.
    void set_idle();

    friend ResetOnExit;
    struct Data;
    Pimpl<Data> m_data;
};



}
#endif
