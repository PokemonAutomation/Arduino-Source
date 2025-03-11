/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_GroupOption_H
#define PokemonAutomation_Options_GroupOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "BatchOption.h"

namespace PokemonAutomation{


class GroupOption : public BatchOption{
public:
    enum class EnableMode{
        ALWAYS_ENABLED,
        DEFAULT_DISABLED,
        DEFAULT_ENABLED,
    };

    ~GroupOption();
    GroupOption(
        std::string label,
        LockMode lock_while_program_is_running,
        EnableMode enable_mode = EnableMode::ALWAYS_ENABLED,
        bool show_restore_defaults_button = false
    );

    const std::string label() const;

    bool toggleable() const;
    bool enabled() const;
    void set_enabled(bool enabled);

    bool restore_defaults_button_enabled() const;

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
