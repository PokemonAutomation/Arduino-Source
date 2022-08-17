/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilterOption_H
#define PokemonAutomation_PokemonSwSh_EncounterFilterOption_H

#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_NameSelectOption.h"
#include "PokemonSwSh_EncounterFilterOverride.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class EncounterFilterOption : public ConfigOption{
public:
    EncounterFilterOption(bool rare_stars, bool enable_overrides);

    ShinyFilter shiny_filter() const{ return m_shiny_filter_current.load(std::memory_order_relaxed); }
    std::vector<std::unique_ptr<EncounterFilterOverride>> copy_snapshot() const{
        return m_table.copy_snapshot();
    }

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class EncounterFilterWidget;

    const bool m_rare_stars;
    const bool m_enable_overrides;

    const ShinyFilter m_shiny_filter_default;
    std::atomic<ShinyFilter> m_shiny_filter_current;

    EncounterFilterTable m_table;
};







}
}
}
#endif
