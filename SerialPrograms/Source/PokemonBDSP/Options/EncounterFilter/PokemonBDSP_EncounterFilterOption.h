/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterFilterOption_H
#define PokemonAutomation_PokemonBDSP_EncounterFilterOption_H

#include <atomic>
#include "PokemonBDSP_EncounterFilterOverride.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class EncounterFilterOption : public ConfigOption{
public:
    EncounterFilterOption(bool enable_overrides, bool allow_autocatch);

    ShinyFilter shiny_filter() const{ return m_shiny_filter_current.load(std::memory_order_acquire); }
    std::vector<EncounterFilterOverride> overrides() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class EncounterFilterWidget;

    const bool m_enable_overrides;
    const bool m_allow_autocatch;

    const ShinyFilter m_shiny_filter_default;
    std::atomic<ShinyFilter> m_shiny_filter_current;

    EncounterFilterOptionFactory m_factory;
    EditableTableOption m_table;
};



}
}
}
#endif
