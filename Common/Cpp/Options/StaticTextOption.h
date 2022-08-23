/*  Static Text Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StaticTextOption_H
#define PokemonAutomation_Options_StaticTextOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{



class StaticTextOption : public ConfigOption{
public:
    ~StaticTextOption();
    StaticTextOption(std::string label, bool text_wrapping = true);

    bool text_wrapping() const;
    std::string text() const;
    void set_text(std::string label);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override{}

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};



class SectionDividerOption : public ConfigOption{
public:
    ~SectionDividerOption();
    SectionDividerOption(std::string label, bool text_wrapping = true);

    bool text_wrapping() const;
    std::string text() const;
    void set_text(std::string label);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override{}

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};





}
#endif


