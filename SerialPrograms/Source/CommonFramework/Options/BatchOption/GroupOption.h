/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      "GroupOption::enabled" is thread-safe.
 *
 */

#ifndef PokemonAutomation_GroupOption_H
#define PokemonAutomation_GroupOption_H

#include <QString>
#include "BatchOption.h"

namespace PokemonAutomation{


class GroupOption : public BatchOption{
public:
    GroupOption(
        QString label,
        bool toggleable = false,
        bool enabled = true
    );

    const QString label() const{ return m_label; }
    bool enabled() const;

    virtual void load_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

public:
    //  Callbacks
    virtual void on_set_enabled(bool enabled);

private:
    friend class GroupWidget;
    const QString m_label;
    const bool m_toggleable;
    const bool m_default_enabled;
    std::atomic<bool> m_enabled;
};




}
#endif
