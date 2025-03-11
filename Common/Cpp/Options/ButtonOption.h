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


class ButtonCell : public ConfigOption{
public:
    enum Enabled : bool{
        DISABLED,
        ENABLED,
    };

public:
    ~ButtonCell();
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

    void press_button();

//    virtual void load_json(const JsonValue& json) override;
//    virtual JsonValue to_json() const override;

//    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;



protected:
    struct Data;
    Pimpl<Data> m_data;
};



class ButtonOption : public ButtonCell{
public:
    ~ButtonOption();
    ButtonOption(const ButtonOption& x) = delete;
    ButtonOption(
        std::string label,
        std::string text,
        int button_height = 0,
        int text_size = 0
    );
    ButtonOption(
        std::string label,
        Enabled state,
        std::string text,
        int button_height = 0,
        int text_size = 0
    );

    std::string label() const;
    void set_label(std::string label);

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;


private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
