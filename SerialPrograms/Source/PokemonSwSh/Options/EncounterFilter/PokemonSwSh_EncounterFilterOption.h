/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilterOption_H
#define PokemonAutomation_PokemonSwSh_EncounterFilterOption_H

#include "PokemonSwSh_EncounterFilterOverride.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class EncounterFilterOption : public ConfigOption{
public:
    virtual ~EncounterFilterOption();
    EncounterFilterOption(bool rare_stars, bool enable_overrides);

    ShinyFilter shiny_filter() const{ return m_shiny_filter_current; }
    std::vector<EncounterFilterOverride> overrides() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class EncounterFilterWidget;

    QString m_label;

    const bool m_rare_stars;
    const bool m_enable_overrides;

    const ShinyFilter m_shiny_filter_default;
    ShinyFilter m_shiny_filter_current;

    EditableTableOption m_table;
};



}
}
}
#endif
