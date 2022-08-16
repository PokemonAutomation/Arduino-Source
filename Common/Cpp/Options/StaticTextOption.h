/*  Static Text Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StaticTextOption_H
#define PokemonAutomation_Options_StaticTextOption_H

#include "Common/Cpp/SpinLock.h"
#include "ConfigOption.h"

namespace PokemonAutomation{



class StaticTextOption : public ConfigOption{
public:
    StaticTextOption(std::string label);
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    std::string text() const;
    void set_text(std::string label);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    mutable SpinLock m_lock;
    std::string m_text;
};



class SectionDividerOption : public ConfigOption{
public:
    SectionDividerOption(std::string label);
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    std::string text() const;
    void set_text(std::string label);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    mutable SpinLock m_lock;
    std::string m_text;
};





}
#endif


