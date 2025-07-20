/*  Key Binding Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_KeyBindingOption_H
#define PokemonAutomation_Options_KeyBindingOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{



class KeyBindingCell : public ConfigOption{
public:
    ~KeyBindingCell();
    KeyBindingCell(LockMode lock_while_program_is_running);

    operator uint32_t() const;
    operator std::string() const;
    void set(uint32_t key);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;


public:
    //  UI Functions
    void set(std::string text);


private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
