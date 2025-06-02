/*  Sandwich Makter Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichMakerOption_H
#define PokemonAutomation_PokemonSV_SandwichMakerOption_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "PokemonSV_SandwichIngredientsTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



enum class HerbaSelection{
    sweet_herba_mystica,
    salty_herba_mystica,
    sour_herba_mystica,
    bitter_herba_mystica,
    spicy_herba_mystica,
};

enum class BaseRecipe{
    non_shiny,
    shiny,
    huge,
    tiny,
    paradox,
    custom,
};

enum class PokemonType{
    normal,
    fire,
    water,
    electric,
    grass,
    ice,
    fighting,
    poison,
    ground,
    flying,
    psychic,
    bug,
    rock,
    ghost,
    dragon,
    dark,
    steel,
    fairy,
};

enum class ParadoxRecipe{
    humungo3_greattusk_1,
    humungo2_screamtail_1,
    humungo2_screamtail_2,
    humungo2_brutebonnet_1,
    humungo2_brutebonnet_2,
    humungo3_fluttermane_1,
    humungo3_fluttermane_2,
    humungo2_slitherwing_1,
    humungo2_slitherwing_2,
    humungo3_sandyshocks_1,
    humungo3_sandyshocks_2,
    humungo3_roaringmoon_1,
    humungo3_roaringmoon_2,
    humungo2_irontreads_1,
    humungo2_irontreads_2,
    humungo2_ironbundle_1,
    humungo2_ironbundle_2,
    humungo2_ironhands_1,
    humungo2_ironhands_2,
    humungo2_ironjugulis_1,
    humungo2_ironjugulis_2,
    humungo3_ironmoth_1,
    humungo3_ironmoth_2,
    humungo3_ironthorns_1,
    humungo3_ironthorns_2,
    humungo2_ironvaliant_1,
    humungo2_ironvaliant_2,
    teensy3_greattusk_1,
    teensy2_screamtail_1,
    teensy2_screamtail_2,
    teensy2_brutebonnet_1,
    teensy3_fluttermane_1,
    teensy2_sliterwing_1,
    teensy2_sliterwing_2,
    teensy3_sandyshocks_1,
    teensy3_sandyshocks_2,
    teensy3_roaringmoon_1,
    teensy2_irontreads_1,
    teensy2_irontreads_2,
    teensy2_ironbundle_1,
    teensy2_ironbundle_2,
    teensy2_ironhands_1,
    teensy2_ironjugulis_1,
    teensy3_ironmoth_1,
    teensy3_ironthorns_1,
    teensy2_ironvaliant_1,
    teensy2_ironvaliant_2,
};

enum class SandwichRecipe{
    non_shiny_normal,
    shiny_normal,
    shiny_fire,
    shiny_water,
    shiny_electric,
    shiny_grass,
    shiny_ice,
    shiny_fighting,
    shiny_poison,
    shiny_ground,
    shiny_flying,
    shiny_psychic,
    shiny_bug,
    shiny_rock,
    shiny_ghost,
    shiny_dragon,
    shiny_dark,
    shiny_steel,
    shiny_fairy,
    huge_normal,
    huge_fire,
    huge_water,
    huge_electric,
    huge_grass,
    huge_ice,
    huge_fighting,
    huge_poison,
    huge_ground,
    huge_flying,
    huge_psychic,
    huge_bug,
    huge_rock,
    huge_ghost,
    huge_dragon,
    huge_dark,
    huge_steel,
    huge_fairy,
    tiny_normal,
    tiny_fire,
    tiny_water,
    tiny_electric,
    tiny_grass,
    tiny_ice,
    tiny_fighting,
    tiny_poison,
    tiny_ground,
    tiny_flying,
    tiny_psychic,
    tiny_bug,
    tiny_rock,
    tiny_ghost,
    tiny_dragon,
    tiny_dark,
    tiny_steel,
    tiny_fairy,
    humungo3_greattusk_1,
    humungo2_screamtail_1,
    humungo2_screamtail_2,
    humungo2_brutebonnet_1,
    humungo2_brutebonnet_2,
    humungo3_fluttermane_1,
    humungo3_fluttermane_2,
    humungo2_slitherwing_1,
    humungo2_slitherwing_2,
    humungo3_sandyshocks_1,
    humungo3_sandyshocks_2,
    humungo3_roaringmoon_1,
    humungo3_roaringmoon_2,
    humungo2_irontreads_1,
    humungo2_irontreads_2,
    humungo2_ironbundle_1,
    humungo2_ironbundle_2,
    humungo2_ironhands_1,
    humungo2_ironhands_2,
    humungo2_ironjugulis_1,
    humungo2_ironjugulis_2,
    humungo3_ironmoth_1,
    humungo3_ironmoth_2,
    humungo3_ironthorns_1,
    humungo3_ironthorns_2,
    humungo2_ironvaliant_1,
    humungo2_ironvaliant_2,
    teensy3_greattusk_1,
    teensy2_screamtail_1,
    teensy2_screamtail_2,
    teensy2_brutebonnet_1,
    teensy3_fluttermane_1,
    teensy2_sliterwing_1,
    teensy2_sliterwing_2,
    teensy3_sandyshocks_1,
    teensy3_sandyshocks_2,
    teensy3_roaringmoon_1,
    teensy2_irontreads_1,
    teensy2_irontreads_2,
    teensy2_ironbundle_1,
    teensy2_ironbundle_2,
    teensy2_ironhands_1,
    teensy2_ironjugulis_1,
    teensy3_ironmoth_1,
    teensy3_ironthorns_1,
    teensy2_ironvaliant_1,
    teensy2_ironvaliant_2,
};




class SandwichMakerOption : public GroupOption, private ConfigOption::Listener{

public:
    ~SandwichMakerOption();

    SandwichMakerOption(
        std::string label,
        OCR::LanguageOCROption* language_option,
        BaseRecipe base_recipe,
        bool show_save_option,
        GroupOption::EnableMode enable_mode
    );

public:
    std::string herba_to_string(HerbaSelection value);
    std::vector<std::string> get_premade_ingredients(SandwichRecipe value);
    static bool two_herba_required(BaseRecipe value);
    SandwichRecipe get_premade_sandwich_recipe(BaseRecipe base, PokemonType type, ParadoxRecipe other);

private:
    std::unique_ptr<OCR::LanguageOCROption> m_language_owner;
public:
    OCR::LanguageOCROption& LANGUAGE;

    BooleanCheckBoxOption SAVE_GAME_BEFORE_SANDWICH;

    EnumDropdownOption<BaseRecipe> BASE_RECIPE;
    EnumDropdownOption<PokemonType> TYPE;
    EnumDropdownOption<ParadoxRecipe> PARADOX;
    EnumDropdownOption<HerbaSelection> HERBA_ONE;
    EnumDropdownOption<HerbaSelection> HERBA_TWO;
    StaticTextOption HERB_INCOMPATIBILITY_WARNING;
    SandwichIngredientsTable SANDWICH_INGREDIENTS;

private:
    virtual void on_config_value_changed(void* object) override;
    virtual std::string check_validity() const override;

    std::string check_herb_compatibility(HerbaSelection herb1, HerbaSelection herb2, PokemonType type) const;
};

}
}
}
#endif
