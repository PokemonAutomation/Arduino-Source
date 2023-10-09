/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_GroupOption_H
#define PokemonAutomation_Options_GroupOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "BatchOption.h"

namespace PokemonAutomation{


class GroupOption : public BatchOption{
public:
    ~GroupOption();
    GroupOption(
        std::string label,
        LockMode lock_while_program_is_running,
        bool toggleable = false,
        bool enabled = true
    );

    const std::string label() const;
    bool toggleable() const;
    bool enabled() const;
    void set_enabled(bool enabled);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

public:
    //  Callbacks
    virtual void on_set_enabled(bool enabled);

private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
