/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_GroupOption_H
#define PokemonAutomation_Options_GroupOption_H

#include <atomic>
#include "BatchOption.h"

namespace PokemonAutomation{


class GroupOption : public BatchOption{
public:
    GroupOption(
        std::string label,
        bool toggleable = false,
        bool enabled = true
    );

    const std::string label() const{ return m_label; }
    bool enabled() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

public:
    //  Callbacks
    virtual void on_set_enabled(bool enabled);

private:
    friend class GroupWidget;
    const std::string m_label;
    const bool m_toggleable;
    const bool m_default_enabled;
    std::atomic<bool> m_enabled;
};




}
#endif
