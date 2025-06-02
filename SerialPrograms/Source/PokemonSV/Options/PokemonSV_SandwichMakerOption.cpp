/*  Sandwich Maker Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
//#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV_SandwichMakerOption.h"
//#include "PokemonSV_SandwichIngredientsOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//  Map for standard+type recipes taking Base + Type to find SandwichRecipe
const std::map<std::pair<BaseRecipe, PokemonType>, SandwichRecipe>& PREMADE_SANDWICH_TYPE(){
    static const std::map<std::pair<BaseRecipe, PokemonType>, SandwichRecipe> map{
        {{BaseRecipe::non_shiny, PokemonType::normal},  SandwichRecipe::non_shiny_normal},
        {{BaseRecipe::shiny, PokemonType::normal},  SandwichRecipe::shiny_normal},
        {{BaseRecipe::shiny, PokemonType::fire},    SandwichRecipe::shiny_fire},
        {{BaseRecipe::shiny, PokemonType::water},   SandwichRecipe::shiny_water},
        {{BaseRecipe::shiny, PokemonType::electric},SandwichRecipe::shiny_electric},
        {{BaseRecipe::shiny, PokemonType::grass},   SandwichRecipe::shiny_grass},
        {{BaseRecipe::shiny, PokemonType::ice},     SandwichRecipe::shiny_ice},
        {{BaseRecipe::shiny, PokemonType::fighting},SandwichRecipe::shiny_fighting},
        {{BaseRecipe::shiny, PokemonType::poison},  SandwichRecipe::shiny_poison},
        {{BaseRecipe::shiny, PokemonType::ground},  SandwichRecipe::shiny_ground},
        {{BaseRecipe::shiny, PokemonType::flying},  SandwichRecipe::shiny_flying},
        {{BaseRecipe::shiny, PokemonType::psychic}, SandwichRecipe::shiny_psychic},
        {{BaseRecipe::shiny, PokemonType::bug},     SandwichRecipe::shiny_bug},
        {{BaseRecipe::shiny, PokemonType::rock},    SandwichRecipe::shiny_rock},
        {{BaseRecipe::shiny, PokemonType::ghost},   SandwichRecipe::shiny_ghost},
        {{BaseRecipe::shiny, PokemonType::dragon},  SandwichRecipe::shiny_dragon},
        {{BaseRecipe::shiny, PokemonType::dark},    SandwichRecipe::shiny_dark},
        {{BaseRecipe::shiny, PokemonType::steel},   SandwichRecipe::shiny_steel},
        {{BaseRecipe::shiny, PokemonType::fairy},   SandwichRecipe::shiny_fairy},
        {{BaseRecipe::huge,  PokemonType::normal},  SandwichRecipe::huge_normal},
        {{BaseRecipe::huge,  PokemonType::fire},    SandwichRecipe::huge_fire},
        {{BaseRecipe::huge,  PokemonType::water},   SandwichRecipe::huge_water},
        {{BaseRecipe::huge,  PokemonType::electric},SandwichRecipe::huge_electric},
        {{BaseRecipe::huge,  PokemonType::grass},   SandwichRecipe::huge_grass},
        {{BaseRecipe::huge,  PokemonType::ice},     SandwichRecipe::huge_ice},
        {{BaseRecipe::huge,  PokemonType::fighting},SandwichRecipe::huge_fighting},
        {{BaseRecipe::huge,  PokemonType::poison},  SandwichRecipe::huge_poison},
        {{BaseRecipe::huge,  PokemonType::ground},  SandwichRecipe::huge_ground},
        {{BaseRecipe::huge,  PokemonType::flying},  SandwichRecipe::huge_flying},
        {{BaseRecipe::huge,  PokemonType::psychic}, SandwichRecipe::huge_psychic},
        {{BaseRecipe::huge,  PokemonType::bug},     SandwichRecipe::huge_bug},
        {{BaseRecipe::huge,  PokemonType::rock},    SandwichRecipe::huge_rock},
        {{BaseRecipe::huge,  PokemonType::ghost},   SandwichRecipe::huge_ghost},
        {{BaseRecipe::huge,  PokemonType::dragon},  SandwichRecipe::huge_dragon},
        {{BaseRecipe::huge,  PokemonType::dark},    SandwichRecipe::huge_dark},
        {{BaseRecipe::huge,  PokemonType::steel},   SandwichRecipe::huge_steel},
        {{BaseRecipe::huge,  PokemonType::fairy},   SandwichRecipe::huge_fairy},
        {{BaseRecipe::tiny,  PokemonType::normal},  SandwichRecipe::tiny_normal},
        {{BaseRecipe::tiny,  PokemonType::fire},    SandwichRecipe::tiny_fire},
        {{BaseRecipe::tiny,  PokemonType::water},   SandwichRecipe::tiny_water},
        {{BaseRecipe::tiny,  PokemonType::electric},SandwichRecipe::tiny_electric},
        {{BaseRecipe::tiny,  PokemonType::grass},   SandwichRecipe::tiny_grass},
        {{BaseRecipe::tiny,  PokemonType::ice},     SandwichRecipe::tiny_ice},
        {{BaseRecipe::tiny,  PokemonType::fighting},SandwichRecipe::tiny_fighting},
        {{BaseRecipe::tiny,  PokemonType::poison},  SandwichRecipe::tiny_poison},
        {{BaseRecipe::tiny,  PokemonType::ground},  SandwichRecipe::tiny_ground},
        {{BaseRecipe::tiny,  PokemonType::flying},  SandwichRecipe::tiny_flying},
        {{BaseRecipe::tiny,  PokemonType::psychic}, SandwichRecipe::tiny_psychic},
        {{BaseRecipe::tiny,  PokemonType::bug},     SandwichRecipe::tiny_bug},
        {{BaseRecipe::tiny,  PokemonType::rock},    SandwichRecipe::tiny_rock},
        {{BaseRecipe::tiny,  PokemonType::ghost},   SandwichRecipe::tiny_ghost},
        {{BaseRecipe::tiny,  PokemonType::dragon},  SandwichRecipe::tiny_dragon},
        {{BaseRecipe::tiny,  PokemonType::dark},    SandwichRecipe::tiny_dark},
        {{BaseRecipe::tiny,  PokemonType::steel},   SandwichRecipe::tiny_steel},
        {{BaseRecipe::tiny,  PokemonType::fairy},   SandwichRecipe::tiny_fairy},
    };
    return map;
}

//  Map for Other recipes
const std::map<ParadoxRecipe, SandwichRecipe>& PREMADE_SANDWICH_OTHER(){
    static const std::map<ParadoxRecipe, SandwichRecipe> map{
        {ParadoxRecipe::humungo3_greattusk_1,     SandwichRecipe::humungo3_greattusk_1},
        {ParadoxRecipe::humungo2_screamtail_1,    SandwichRecipe::humungo2_screamtail_1},
        {ParadoxRecipe::humungo2_screamtail_2,    SandwichRecipe::humungo2_screamtail_2},
        {ParadoxRecipe::humungo2_brutebonnet_1,   SandwichRecipe::humungo2_brutebonnet_1},
        {ParadoxRecipe::humungo2_brutebonnet_2,   SandwichRecipe::humungo2_brutebonnet_2},
        {ParadoxRecipe::humungo3_fluttermane_1,   SandwichRecipe::humungo3_fluttermane_1},
        {ParadoxRecipe::humungo3_fluttermane_2,   SandwichRecipe::humungo3_fluttermane_2},
        {ParadoxRecipe::humungo2_slitherwing_1,   SandwichRecipe::humungo2_slitherwing_1},
        {ParadoxRecipe::humungo2_slitherwing_2,   SandwichRecipe::humungo2_slitherwing_2},
        {ParadoxRecipe::humungo3_sandyshocks_1,   SandwichRecipe::humungo3_sandyshocks_1},
        {ParadoxRecipe::humungo3_sandyshocks_2,   SandwichRecipe::humungo3_sandyshocks_2},
        {ParadoxRecipe::humungo3_roaringmoon_1,   SandwichRecipe::humungo3_roaringmoon_1},
        {ParadoxRecipe::humungo3_roaringmoon_2,   SandwichRecipe::humungo3_roaringmoon_2},
        {ParadoxRecipe::humungo2_irontreads_1,    SandwichRecipe::humungo2_irontreads_1},
        {ParadoxRecipe::humungo2_irontreads_2,    SandwichRecipe::humungo2_irontreads_2},
        {ParadoxRecipe::humungo2_ironbundle_1,    SandwichRecipe::humungo2_ironbundle_1},
        {ParadoxRecipe::humungo2_ironbundle_2,    SandwichRecipe::humungo2_ironbundle_2},
        {ParadoxRecipe::humungo2_ironhands_1,     SandwichRecipe::humungo2_ironhands_1},
        {ParadoxRecipe::humungo2_ironhands_2,     SandwichRecipe::humungo2_ironhands_2},
        {ParadoxRecipe::humungo2_ironjugulis_1,   SandwichRecipe::humungo2_ironjugulis_1},
        {ParadoxRecipe::humungo2_ironjugulis_2,   SandwichRecipe::humungo2_ironjugulis_2},
        {ParadoxRecipe::humungo3_ironmoth_1,      SandwichRecipe::humungo3_ironmoth_1},
        {ParadoxRecipe::humungo3_ironmoth_2,      SandwichRecipe::humungo3_ironmoth_2},
        {ParadoxRecipe::humungo3_ironthorns_1,    SandwichRecipe::humungo3_ironthorns_1},
        {ParadoxRecipe::humungo3_ironthorns_2,    SandwichRecipe::humungo3_ironthorns_2},
        {ParadoxRecipe::humungo2_ironvaliant_1,   SandwichRecipe::humungo2_ironvaliant_1},
        {ParadoxRecipe::humungo2_ironvaliant_2,   SandwichRecipe::humungo2_ironvaliant_2},
        {ParadoxRecipe::teensy3_greattusk_1,      SandwichRecipe::teensy3_greattusk_1},
        {ParadoxRecipe::teensy2_screamtail_1,     SandwichRecipe::teensy2_screamtail_1},
        {ParadoxRecipe::teensy2_screamtail_2,     SandwichRecipe::teensy2_screamtail_2},
        {ParadoxRecipe::teensy2_brutebonnet_1,    SandwichRecipe::teensy2_brutebonnet_1},
        {ParadoxRecipe::teensy3_fluttermane_1,    SandwichRecipe::teensy3_fluttermane_1},
        {ParadoxRecipe::teensy2_sliterwing_1,     SandwichRecipe::teensy2_sliterwing_1},
        {ParadoxRecipe::teensy2_sliterwing_2,     SandwichRecipe::teensy2_sliterwing_2},
        {ParadoxRecipe::teensy3_sandyshocks_1,    SandwichRecipe::teensy3_sandyshocks_1},
        {ParadoxRecipe::teensy3_sandyshocks_2,    SandwichRecipe::teensy3_sandyshocks_2},
        {ParadoxRecipe::teensy3_roaringmoon_1,    SandwichRecipe::teensy3_roaringmoon_1},
        {ParadoxRecipe::teensy2_irontreads_1,     SandwichRecipe::teensy2_irontreads_1},
        {ParadoxRecipe::teensy2_irontreads_2,     SandwichRecipe::teensy2_irontreads_2},
        {ParadoxRecipe::teensy2_ironbundle_1,     SandwichRecipe::teensy2_ironbundle_1},
        {ParadoxRecipe::teensy2_ironbundle_2,     SandwichRecipe::teensy2_ironbundle_2},
        {ParadoxRecipe::teensy2_ironhands_1,      SandwichRecipe::teensy2_ironhands_1},
        {ParadoxRecipe::teensy2_ironjugulis_1,    SandwichRecipe::teensy2_ironjugulis_1},
        {ParadoxRecipe::teensy3_ironmoth_1,       SandwichRecipe::teensy3_ironmoth_1},
        {ParadoxRecipe::teensy3_ironthorns_1,     SandwichRecipe::teensy3_ironthorns_1},
        {ParadoxRecipe::teensy2_ironvaliant_1,    SandwichRecipe::teensy2_ironvaliant_1},
        {ParadoxRecipe::teensy2_ironvaliant_2,    SandwichRecipe::teensy2_ironvaliant_2},
    };
    return map;
}

//  Leave the herba out, the user selects them as HERBA_ONE and HERBA_TWO.
//  Remember the ingredient limits: Six fillings, four condiments. Two condiment slots will be taken by the Herba.
const std::map<SandwichRecipe, std::vector<std::string>>& PREMADE_SANDWICH_INGREDIENTS(){
    static const std::map<SandwichRecipe, std::vector<std::string>> map{
        {SandwichRecipe::non_shiny_normal,      {"chorizo", "chorizo", "chorizo", "chorizo", "banana", "banana", "mayonnaise", "mayonnaise", "mayonnaise", "whipped-cream"}},
        {SandwichRecipe::shiny_normal,          {"cucumber", "pickle", "tofu", "tofu", "tofu", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_fire,            {"cucumber", "pickle", "red-bell-pepper", "red-bell-pepper","curry-powder","curry-powder", "red-bell-pepper"}},
        {SandwichRecipe::shiny_water,           {"cucumber", "pickle", "cucumber", "cucumber", "cucumber","curry-powder","curry-powder"}},
        {SandwichRecipe::shiny_electric,        {"cucumber", "pickle", "yellow-bell-pepper", "yellow-bell-pepper", "yellow-bell-pepper", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_grass,           {"cucumber", "pickle", "lettuce", "lettuce", "lettuce", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_ice,             {"cucumber", "pickle", "klawf-stick", "klawf-stick", "klawf-stick", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_fighting,        {"cucumber", "pickle", "pickle", "pickle", "pickle", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_poison,          {"cucumber", "pickle", "green-bell-pepper", "green-bell-pepper", "green-bell-pepper", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_ground,          {"cucumber", "pickle", "ham", "ham", "ham", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_flying,          {"cucumber", "pickle", "prosciutto", "prosciutto", "prosciutto", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_psychic,         {"cucumber", "pickle", "onion", "onion", "onion", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_bug,             {"cucumber", "pickle", "cherry-tomatoes", "cherry-tomatoes", "cherry-tomatoes", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_rock,            {"cucumber", "pickle", "bacon", "bacon", "bacon", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_ghost,           {"cucumber", "pickle", "red-onion", "red-onion", "red-onion", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_dragon,          {"cucumber", "pickle", "avocado", "avocado", "avocado", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_dark,            {"cucumber", "pickle", "smoked-fillet", "smoked-fillet", "smoked-fillet", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_steel,           {"cucumber", "pickle", "hamburger", "hamburger", "hamburger", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_fairy,           {"cucumber", "pickle", "tomato", "tomato", "tomato", "curry-powder", "curry-powder"}},
        {SandwichRecipe::huge_normal,           {"tofu", "mustard", "mustard"}},
        {SandwichRecipe::huge_fire,             {"red-bell-pepper", "mustard", "mustard"}},
        {SandwichRecipe::huge_water,            {"cucumber", "mustard", "mustard"}},
        {SandwichRecipe::huge_electric,         {"yellow-bell-pepper", "mustard", "mustard"}},
        {SandwichRecipe::huge_grass,            {"lettuce", "mustard", "mustard"}},
        {SandwichRecipe::huge_ice,              {"klawf-stick", "mustard", "mustard"}},
        {SandwichRecipe::huge_fighting,         {"pickle", "mustard", "mustard"}},
        {SandwichRecipe::huge_poison,           {"green-bell-pepper", "mustard", "mustard"}},
        {SandwichRecipe::huge_ground,           {"ham", "mustard", "mustard"}},
        {SandwichRecipe::huge_flying,           {"prosciutto", "mustard", "mustard"}},
        {SandwichRecipe::huge_psychic,          {"onion", "mustard", "mustard"}},
        {SandwichRecipe::huge_bug,              {"cherry-tomatoes", "mustard", "mustard"}},
        {SandwichRecipe::huge_rock,             {"bacon", "mustard", "mustard"}},
        {SandwichRecipe::huge_ghost,            {"red-onion", "mustard", "mustard"}},
        {SandwichRecipe::huge_dragon,           {"avocado", "mustard", "mustard"}},
        {SandwichRecipe::huge_dark,             {"smoked-fillet", "mustard", "mustard"}},
        {SandwichRecipe::huge_steel,            {"hamburger", "mustard", "mustard"}},
        {SandwichRecipe::huge_fairy,            {"tomato", "mustard", "mustard"}},
        {SandwichRecipe::tiny_normal,           {"tofu", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_fire,             {"red-bell-pepper", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_water,            {"cucumber", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_electric,         {"yellow-bell-pepper", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_grass,            {"lettuce", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_ice,              {"klawf-stick", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_fighting,         {"pickle", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_poison,           {"green-bell-pepper", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_ground,           {"ham", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_flying,           {"prosciutto", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_psychic,          {"onion", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_bug,              {"cherry-tomatoes", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_rock,             {"bacon", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_ghost,            {"red-onion", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_dragon,           {"avocado", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_dark,             {"smoked-fillet", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_steel,            {"hamburger", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_fairy,            {"tomato", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::humungo3_greattusk_1,   {"herbed-sausage", "herbed-sausage", "herbed-sausage", "herbed-sausage", "ham", "horseradish", "jam", "wasabi", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo2_screamtail_1,  {"noodles", "tomato", "tomato", "wasabi", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo2_screamtail_2,  {"noodles", "potato-salad", "vinegar", "salty-herba-mystica"}},
        {SandwichRecipe::humungo2_brutebonnet_1, {"potato-salad", "lettuce", "lettuce", "wasabi", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo2_brutebonnet_2, {"potato-salad", "rice", "jam", "chili-sauce", "salty-herba-mystica"}},
        {SandwichRecipe::humungo3_fluttermane_1, {"potato-salad", "potato-salad", "potato-salad", "potato-salad", "potato-salad", "tomato", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo3_fluttermane_2, {"potato-salad", "potato-salad", "potato-salad", "potato-salad", "potato-salad", "olive-oil", "yogurt", "yogurt", "salty-herba-mystica"}},
        {SandwichRecipe::humungo2_slitherwing_1, {"cherry-tomatoes", "cherry-tomatoes", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo2_slitherwing_2, {"pickle", "potato-salad", "rice", "chili-sauce", "cream-cheese", "salty-herba-mystica"}},
        {SandwichRecipe::humungo3_sandyshocks_1, {"noodles", "noodles", "noodles", "noodles", "noodles", "mustard", "wasabi", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo3_sandyshocks_2, {"noodles", "noodles", "noodles", "noodles", "noodles", "mustard", "jam", "salty-herba-mystica"}},
        {SandwichRecipe::humungo3_roaringmoon_1, {"potato-salad", "potato-salad", "potato-salad", "potato-salad", "potato-salad", "wasabi", "yogurt", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo3_roaringmoon_2, {"potato-salad", "potato-salad", "potato-salad", "potato-salad", "potato-salad", "wasabi", "vinegar", "salty-herba-mystica"}},
        {SandwichRecipe::humungo2_irontreads_1,  {"ham", "ham", "egg", "mustard", "curry-powder", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo2_irontreads_2,  {"noodles", "potato-salad", "peanut-butter", "salty-herba-mystica"}},
        {SandwichRecipe::humungo2_ironbundle_1,  {"cucumber", "cucumber", "noodles", "jam", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo2_ironbundle_2,  {"noodles", "rice", "wasabi", "curry-powder", "pepper", "salty-herba-mystica"}},
        {SandwichRecipe::humungo2_ironhands_1,   {"yellow-bell-pepper", "yellow-bell-pepper", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo2_ironhands_2,   {"noodles", "pickle", "rice", "jam", "pepper", "salty-herba-mystica"}},
        {SandwichRecipe::humungo2_ironjugulis_1, {"egg", "smoked-fillet", "smoked-fillet", "yogurt", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo2_ironjugulis_2, {"potato-salad", "rice", "ketchup", "wasabi", "salty-herba-mystica"}},
        {SandwichRecipe::humungo3_ironmoth_1,    {"chorizo", "chorizo", "chorizo", "noodles", "red-bell-pepper", "olive-oil", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo3_ironmoth_2,    {"chorizo", "chorizo", "noodles", "noodles", "rice", "rice", "curry-powder", "salty-herba-mystica"}},
        {SandwichRecipe::humungo3_ironthorns_1,  {"noodles", "noodles", "noodles", "noodles", "noodles", "marmalade", "mustard", "jam", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo3_ironthorns_2,  {"noodles", "noodles", "noodles", "noodles", "noodles", "yellow-bell-pepper", "mustard", "mustard", "salty-herba-mystica"}},
        {SandwichRecipe::humungo2_ironvaliant_1, {"herbed-sausage", "tomato", "tomato", "marmalade", "wasabi", "yogurt", "spicy-herba-mystica"}},
        {SandwichRecipe::humungo2_ironvaliant_2, {"potato-salad", "rice", "tomato", "salty-herba-mystica"}},
        {SandwichRecipe::teensy3_greattusk_1,    {"herbed-sausage", "herbed-sausage", "herbed-sausage", "noodles", "pickle", "horseradish", "horseradish", "sour-herba-mystica"}},
        {SandwichRecipe::teensy2_screamtail_1,   {"egg", "onion", "onion", "chili-sauce", "wasabi", "yogurt", "sour-herba-mystica"}},
        {SandwichRecipe::teensy2_screamtail_2,   {"herbed-sausage", "tomato", "tomato", "wasabi", "wasabi", "salty-herba-mystica"}},
        {SandwichRecipe::teensy2_brutebonnet_1,  {"egg", "smoked-fillet", "smoked-fillet", "chili-sauce", "curry-powder", "jam", "sour-herba-mystica"}},
        {SandwichRecipe::teensy3_fluttermane_1,  {"cheese", "potato-salad", "potato-salad", "potato-salad", "potato-salad", "tomato", "curry-powder", "curry-powder", "sour-herba-mystica"}},
        {SandwichRecipe::teensy2_sliterwing_1,   {"chorizo", "pickle", "pickle", "butter", "sour-herba-mystica"}},
        {SandwichRecipe::teensy2_sliterwing_2,   {"chorizo", "pickle", "pickle", "curry-powder", "marmalade", "wasabi", "salty-herba-mystica"}},
        {SandwichRecipe::teensy3_sandyshocks_1,  {"noodles", "noodles", "noodles", "noodles", "yellow-bell-pepper", "herbed-sausage", "horseradish", "sour-herba-mystica"}},
        {SandwichRecipe::teensy3_sandyshocks_2,  {"fried-fillet", "fried-fillet", "noodles", "noodles", "noodles", "banana", "wasabi", "wasabi", "wasabi", "salty-herba-mystica"}},
        {SandwichRecipe::teensy3_roaringmoon_1,  {"potato-salad", "potato-salad", "potato-salad", "potato-salad", "potato-salad", "herbed-sausage", "wasabi", "wasabi", "wasabi", "sour-herba-mystica"}},
        {SandwichRecipe::teensy2_irontreads_1,   {"hamburger", "hamburger", "herbed-sausage", "curry-powder", "whipped-cream", "sour-herba-mystica"}},
        {SandwichRecipe::teensy2_irontreads_2,   {"hamburger", "hamburger", "herbed-sausage", "curry-powder", "horseradish", "peanut-butter", "salty-herba-mystica"}},
        {SandwichRecipe::teensy2_ironbundle_1,   {"herbed-sausage", "klawf-stick", "klawf-stick", "cream-cheese", "wasabi", "sour-herba-mystica"}},
        {SandwichRecipe::teensy2_ironbundle_2,   {"herbed-sausage", "klawf-stick", "klawf-stick", "curry-powder", "pepper", "wasabi", "salty-herba-mystica"}},
        {SandwichRecipe::teensy2_ironhands_1,    {"yellow-bell-pepper", "yellow-bell-pepper", "sour-herba-mystica"}},
        {SandwichRecipe::teensy2_ironjugulis_1,  {"egg", "smoked-fillet", "smoked-fillet", "yogurt", "salt", "sour-herba-mystica"}},
        {SandwichRecipe::teensy3_ironmoth_1,     {"chorizo", "chorizo", "chorizo", "noodles", "red-bell-pepper", "olive-oil", "sour-herba-mystica"}},
        {SandwichRecipe::teensy3_ironthorns_1,   {"noodles", "noodles", "noodles", "noodles", "egg", "yellow-bell-pepper", "horseradish", "sour-herba-mystica"}},
        {SandwichRecipe::teensy2_ironvaliant_1,  {"pickle", "pickle", "egg", "salt", "yogurt", "marmalade", "sour-herba-mystica"}},
        {SandwichRecipe::teensy2_ironvaliant_2,  {"pickle", "pickle", "egg", "wasabi", "wasabi", "yogurt", "salty-herba-mystica"}},
    };
    return map;
}





std::string SandwichMakerOption::herba_to_string(HerbaSelection value){
    switch (value){
    case HerbaSelection::bitter_herba_mystica:
        return "bitter-herba-mystica";
    case HerbaSelection::salty_herba_mystica:
        return "salty-herba-mystica";
    case HerbaSelection::sour_herba_mystica:
        return "sour-herba-mystica";
    case HerbaSelection::spicy_herba_mystica:
        return "spicy-herba-mystica";
    case HerbaSelection::sweet_herba_mystica:
        return "sweet-herba-mystica";
    }
    return "ERROR";
}

std::vector<std::string> SandwichMakerOption::get_premade_ingredients(SandwichRecipe value){
    auto iter = PREMADE_SANDWICH_INGREDIENTS().find(value);
    return iter->second;
}

bool SandwichMakerOption::two_herba_required(BaseRecipe value){
    if (value == BaseRecipe::shiny || value == BaseRecipe::huge || value == BaseRecipe::tiny){
        return true;
    }
    return false;
}

SandwichRecipe SandwichMakerOption::get_premade_sandwich_recipe(BaseRecipe base, PokemonType type, ParadoxRecipe paradox){
    if (base == BaseRecipe::paradox)
    {
        auto iter = PREMADE_SANDWICH_OTHER().find(paradox);
        return iter->second;
    }else{
        auto iter = PREMADE_SANDWICH_TYPE().find(std::make_pair(base, type));
        return iter->second;
    }
}

SandwichMakerOption::~SandwichMakerOption(){
    HERBA_TWO.remove_listener(*this);
    HERBA_ONE.remove_listener(*this);
    TYPE.remove_listener(*this);
    BASE_RECIPE.remove_listener(*this);
}

SandwichMakerOption::SandwichMakerOption(
    std::string label,
    OCR::LanguageOCROption* language_option,
    BaseRecipe base_recipe,
    bool show_save_option,
    GroupOption::EnableMode enable_mode
)
    : GroupOption(
        std::move(label),
        LockMode::UNLOCK_WHILE_RUNNING,
        enable_mode
    )
    , m_language_owner(language_option == nullptr
        ? new OCR::LanguageOCROption(
            "<b>Game Language:</b><br>Required to read ingredients.",
            IV_READER().languages(),
            LockMode::LOCK_WHILE_RUNNING,
            true
        )
        : nullptr
    )
    , LANGUAGE(language_option == nullptr ? *m_language_owner : *language_option)
    , SAVE_GAME_BEFORE_SANDWICH(
        "<b>Save game before making sandwich:</b><br>Sandwich making can be unreliable. "
        "Save the game before each sandwich to prevent loss of progress.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , BASE_RECIPE(
        "<b>Sandwich Recipe:</b><br>Select a recipe to make a sandwich with preset ingredients, or select Custom Sandwich to make a sandwich using the table below. "
        "Refer to the documentation for recipe ingredients and valid Herba Mystica combinations.",
        {
            {BaseRecipe::non_shiny, "non-shiny", "Normal Encounter (non-shiny)"},
            {BaseRecipe::shiny,     "shiny",     "Sparkling + Title + Encounter"},
            {BaseRecipe::huge,      "huge",      "Sparkling + Title + Humungo"},
            {BaseRecipe::tiny,      "tiny",      "Sparkling + Title + Teensy"},
            {BaseRecipe::paradox,   "paradox",   "Title + Encounter + Humungo/Teensy: Paradox-specific"},
            {BaseRecipe::custom,    "custom",    "Custom Sandwich"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        base_recipe
        )
    , TYPE(
        "",
        {
            {PokemonType::normal,   "normal",   "Normal"},
            {PokemonType::fire,     "fire",     "Fire"},
            {PokemonType::water,    "water",    "Water"},
            {PokemonType::electric, "electric", "Electric"},
            {PokemonType::grass,    "grass",    "Grass"},
            {PokemonType::ice,      "ice",      "Ice"},
            {PokemonType::fighting, "fighting", "Fighting"},
            {PokemonType::poison,   "poison",   "Poison"},
            {PokemonType::ground,   "ground",   "Ground"},
            {PokemonType::flying,   "flying",   "Flying"},
            {PokemonType::psychic,  "psychic",  "Psychic"},
            {PokemonType::bug,      "bug",      "Bug"},
            {PokemonType::rock,     "rock",     "Rock"},
            {PokemonType::ghost,    "ghost",    "Ghost"},
            {PokemonType::dragon,   "dragon",   "Dragon"},
            {PokemonType::dark,     "dark",     "Dark"},
            {PokemonType::steel,    "steel",    "Steel"},
            {PokemonType::fairy,    "fairy",    "Fairy"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        PokemonType::normal
    )
    , PARADOX(
        "",
        {
            {ParadoxRecipe::humungo3_greattusk_1,     "humungo3_greattusk_1",     "Great Tusk - Humungo #1"},
            {ParadoxRecipe::humungo2_screamtail_1,    "humungo2_screamtail_1",    "Scream Tail - Humungo #1"},
            {ParadoxRecipe::humungo2_screamtail_2,    "humungo2_screamtail_2",    "Scream Tail - Humungo #2"},
            {ParadoxRecipe::humungo2_brutebonnet_1,   "humungo2_brutebonnet_1",   "Brute Bonnet - Humungo #1"},
            {ParadoxRecipe::humungo2_brutebonnet_2,   "humungo2_brutebonnet_2",   "Brute Bonnet - Humungo #2"},
            {ParadoxRecipe::humungo3_fluttermane_1,   "humungo3_fluttermane_1",   "Flutter Mane - Humungo #1"},
            {ParadoxRecipe::humungo3_fluttermane_2,   "humungo3_fluttermane_2",   "Flutter Mane - Humungo #2"},
            {ParadoxRecipe::humungo2_slitherwing_1,   "humungo2_slitherwing_1",   "Slither Wing - Humungo #1"},
            {ParadoxRecipe::humungo2_slitherwing_2,   "humungo2_slitherwing_2",   "Slither Wing - Humungo #2"},
            {ParadoxRecipe::humungo3_sandyshocks_1,   "humungo3_sandyshocks_1",   "Sandy Shocks - Humungo #1"},
            {ParadoxRecipe::humungo3_sandyshocks_2,   "humungo3_sandyshocks_2",   "Sandy Shocks - Humungo #2"},
            {ParadoxRecipe::humungo3_roaringmoon_1,   "humungo3_roaringmoon_1",   "Roaring Moon - Humungo #1"},
            {ParadoxRecipe::humungo3_roaringmoon_2,   "humungo3_roaringmoon_2",   "Roaring Moon - Humungo #2"},
            {ParadoxRecipe::humungo2_irontreads_1,    "humungo2_irontreads_1",    "Iron Treads - Humungo #1"},
            {ParadoxRecipe::humungo2_irontreads_2,    "humungo2_irontreads_2",    "Iron Treads - Humungo #2"},
            {ParadoxRecipe::humungo2_ironbundle_1,    "humungo2_ironbundle_1",    "Iron Bundle - Humungo #1"},
            {ParadoxRecipe::humungo2_ironbundle_2,    "humungo2_ironbundle_2",    "Iron Bundle - Humungo #2"},
            {ParadoxRecipe::humungo2_ironhands_1,     "humungo2_ironhands_1",     "Iron Hands - Humungo #1"},
            {ParadoxRecipe::humungo2_ironhands_2,     "humungo2_ironhands_2",     "Iron Hands - Humungo #2"},
            {ParadoxRecipe::humungo2_ironjugulis_1,   "humungo2_ironjugulis_1",   "Iron Jugulis - Humungo #1"},
            {ParadoxRecipe::humungo2_ironjugulis_2,   "humungo2_ironjugulis_2",   "Iron Jugulis - Humungo #2"},
            {ParadoxRecipe::humungo3_ironmoth_1,      "humungo3_ironmoth_1",      "Iron Moth - Humungo #1"},
            {ParadoxRecipe::humungo3_ironmoth_2,      "humungo3_ironmoth_2",      "Iron Moth - Humungo #2"},
            {ParadoxRecipe::humungo3_ironthorns_1,    "humungo3_ironthorns_1",    "Iron Thorns - Humungo #1"},
            {ParadoxRecipe::humungo3_ironthorns_2,    "humungo3_ironthorns_2",    "Iron Thorns - Humungo #2"},
            {ParadoxRecipe::humungo2_ironvaliant_1,   "humungo2_ironvaliant_1",   "Iron Valiant - Humungo #1"},
            {ParadoxRecipe::humungo2_ironvaliant_2,   "humungo2_ironvaliant_2",   "Iron Valiant - Humungo #2"},
            {ParadoxRecipe::teensy3_greattusk_1,      "teensy3_greattusk_1",      "Great Tusk - Teensy #1"},
            {ParadoxRecipe::teensy2_screamtail_1,     "teensy2_screamtail_1",     "Scream Tail - Teensy #1"},
            {ParadoxRecipe::teensy2_screamtail_2,     "teensy2_screamtail_2",     "Scream Tail - Teensy #2"},
            {ParadoxRecipe::teensy2_brutebonnet_1,    "teensy2_brutebonnet_1",    "Brute Bonnet - Teensy #1"},
            {ParadoxRecipe::teensy3_fluttermane_1,    "teensy3_fluttermane_1",    "Flutter Mane - Teensy #1"},
            {ParadoxRecipe::teensy2_sliterwing_1,     "teensy2_sliterwing_1",     "Slither Wing - Teensy #1"},
            {ParadoxRecipe::teensy2_sliterwing_2,     "teensy2_sliterwing_2",     "Slither Wing - Teensy #2"},
            {ParadoxRecipe::teensy3_sandyshocks_1,    "teensy3_sandyshocks_1",    "Sandy Shocks - Teensy #1"},
            {ParadoxRecipe::teensy3_sandyshocks_2,    "teensy3_sandyshocks_2",    "Sandy Shocks - Teensy #2"},
            {ParadoxRecipe::teensy3_roaringmoon_1,    "teensy3_roaringmoon_1",    "Roaring Moon - Teensy #1"},
            {ParadoxRecipe::teensy2_irontreads_1,     "teensy2_irontreads_1",     "Iron Treads - Teensy #1"},
            {ParadoxRecipe::teensy2_irontreads_2,     "teensy2_irontreads_2",     "Iron Treads - Teensy #2"},
            {ParadoxRecipe::teensy2_ironbundle_1,     "teensy2_ironbundle_1",     "Iron Bundle - Teensy #1"},
            {ParadoxRecipe::teensy2_ironbundle_2,     "teensy2_ironbundle_2",     "Iron Bundle - Teensy #2"},
            {ParadoxRecipe::teensy2_ironhands_1,      "teensy2_ironhands_1",      "Iron Hands - Teensy #1"},
            {ParadoxRecipe::teensy2_ironjugulis_1,    "teensy2_ironjugulis_1",    "Iron Jugulis - Teensy #1"},
            {ParadoxRecipe::teensy3_ironmoth_1,       "teensy3_ironmoth_1",       "Iron Moth - Teensy #1"},
            {ParadoxRecipe::teensy3_ironthorns_1,     "teensy3_ironthorns_1",     "Iron Thorns - Teensy #1"},
            {ParadoxRecipe::teensy2_ironvaliant_1,    "teensy2_ironvaliant_1",    "Iron Valiant - Teensy #1"},
            {ParadoxRecipe::teensy2_ironvaliant_2,    "teensy2_ironvaliant_2",    "Iron Vailant - Teensy #2"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        ParadoxRecipe::humungo3_greattusk_1
    )
    , HERBA_ONE(
        "<b>Herba Mystica:</b><br>Select the Herba Mystica to use in the preset recipe. Keep in mind which herb combinations are valid for the selected recipe.",
        {
            {HerbaSelection::sweet_herba_mystica,   "sweet-herba-mystica",  "Sweet Herba Mystica"},
            {HerbaSelection::salty_herba_mystica,   "salty-herba-mystica",  "Salty Herba Mystica"},
            {HerbaSelection::sour_herba_mystica,    "sour-herba-mystica",   "Sour Herba Mystica"},
            {HerbaSelection::bitter_herba_mystica,  "bitter-herba-mystica", "Bitter Herba Mystica"},
            {HerbaSelection::spicy_herba_mystica,   "spicy-herba-mystica",  "Spicy Herba Mystica"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        HerbaSelection::salty_herba_mystica
    )
    , HERBA_TWO(
        "",
        {
            {HerbaSelection::sweet_herba_mystica,   "sweet-herba-mystica",  "Sweet Herba Mystica"},
            {HerbaSelection::salty_herba_mystica,   "salty-herba-mystica",  "Salty Herba Mystica"},
            {HerbaSelection::sour_herba_mystica,    "sour-herba-mystica",   "Sour Herba Mystica"},
            {HerbaSelection::bitter_herba_mystica,  "bitter-herba-mystica", "Bitter Herba Mystica"},
            {HerbaSelection::spicy_herba_mystica,   "spicy-herba-mystica",  "Spicy Herba Mystica"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        HerbaSelection::salty_herba_mystica
    )
    , HERB_INCOMPATIBILITY_WARNING("")
    , SANDWICH_INGREDIENTS("<b>Custom Sandwich:</b><br>Make a sandwich from the selected ingredients.<br>You must have at least one ingredient and one condiment, and no more than six ingredients and four condiments.")
{
    if (m_language_owner){
        PA_ADD_OPTION(LANGUAGE);
    }
    if (show_save_option){
        PA_ADD_OPTION(SAVE_GAME_BEFORE_SANDWICH);
    }
    PA_ADD_OPTION(BASE_RECIPE);
    PA_ADD_OPTION(TYPE);
    PA_ADD_OPTION(PARADOX);
    PA_ADD_OPTION(HERBA_ONE);
    PA_ADD_OPTION(HERBA_TWO);
    PA_ADD_OPTION(HERB_INCOMPATIBILITY_WARNING);
    PA_ADD_OPTION(SANDWICH_INGREDIENTS);

    HERB_INCOMPATIBILITY_WARNING.set_visibility(ConfigOptionState::HIDDEN);

    SandwichMakerOption::on_config_value_changed(this);
    BASE_RECIPE.add_listener(*this);
    TYPE.add_listener(*this);
    HERBA_ONE.add_listener(*this);
    HERBA_TWO.add_listener(*this);
}

void SandwichMakerOption::on_config_value_changed(void* object){
    if (BASE_RECIPE == BaseRecipe::custom){
        HERBA_ONE.set_visibility(ConfigOptionState::HIDDEN);
        HERBA_TWO.set_visibility(ConfigOptionState::HIDDEN);
        HERB_INCOMPATIBILITY_WARNING.set_visibility(ConfigOptionState::HIDDEN);
        TYPE.set_visibility(ConfigOptionState::HIDDEN); //to prevent the options moving around
        PARADOX.set_visibility(ConfigOptionState::HIDDEN);
        SANDWICH_INGREDIENTS.set_visibility(ConfigOptionState::ENABLED);
    }else if (BASE_RECIPE == BaseRecipe::non_shiny){
        HERBA_ONE.set_visibility(ConfigOptionState::HIDDEN);
        HERBA_TWO.set_visibility(ConfigOptionState::HIDDEN);
        HERB_INCOMPATIBILITY_WARNING.set_visibility(ConfigOptionState::HIDDEN);
        TYPE.set_visibility(ConfigOptionState::HIDDEN); //to prevent the options moving around
        PARADOX.set_visibility(ConfigOptionState::HIDDEN);
        SANDWICH_INGREDIENTS.set_visibility(ConfigOptionState::HIDDEN);
    }else if (two_herba_required(BASE_RECIPE)){ //shiny, huge, tiny
        HERBA_ONE.set_visibility(ConfigOptionState::ENABLED);
        HERBA_TWO.set_visibility(ConfigOptionState::ENABLED);
        SANDWICH_INGREDIENTS.set_visibility(ConfigOptionState::HIDDEN);

        std::string herb_error;
        do{
            if (BASE_RECIPE != BaseRecipe::shiny){
                break;
            }
            herb_error = check_herb_compatibility(HERBA_ONE, HERBA_TWO, TYPE);
        }while (false);

        if (herb_error.empty()){
            HERB_INCOMPATIBILITY_WARNING.set_visibility(ConfigOptionState::HIDDEN);
        }else{
            HERB_INCOMPATIBILITY_WARNING.set_text("<font color=\"red\">" + herb_error + "</font>");
            HERB_INCOMPATIBILITY_WARNING.set_visibility(ConfigOptionState::ENABLED);
        }
        TYPE.set_visibility(ConfigOptionState::ENABLED);
        PARADOX.set_visibility(ConfigOptionState::HIDDEN);
    }else{ //other
        HERBA_ONE.set_visibility(ConfigOptionState::HIDDEN);
        HERBA_TWO.set_visibility(ConfigOptionState::HIDDEN);
        TYPE.set_visibility(ConfigOptionState::HIDDEN);
        HERB_INCOMPATIBILITY_WARNING.set_visibility(ConfigOptionState::HIDDEN);
        PARADOX.set_visibility(ConfigOptionState::ENABLED);
        SANDWICH_INGREDIENTS.set_visibility(ConfigOptionState::HIDDEN);
    }
}
std::string SandwichMakerOption::check_validity() const{
    if (!two_herba_required(BASE_RECIPE)){
        return "";
    }
    if (BASE_RECIPE != BaseRecipe::shiny){
        return "";
    }
    return check_herb_compatibility(HERBA_ONE, HERBA_TWO, TYPE);
}

std::string SandwichMakerOption::check_herb_compatibility(HerbaSelection herb1, HerbaSelection herb2, PokemonType type) const{
    if ((herb1 == HerbaSelection::sweet_herba_mystica && herb2 == HerbaSelection::sour_herba_mystica) ||
        (herb1 == HerbaSelection::sour_herba_mystica && herb2 == HerbaSelection::sweet_herba_mystica)
    ){
        return "1x Sweet and 1x Sour herb are incompatible for all types.";
    }
    if (herb1 == HerbaSelection::sour_herba_mystica && herb2 == HerbaSelection::sour_herba_mystica){
        return "2x Sour herbs are incompatible for all types.";
    }
    if (herb1 == HerbaSelection::sweet_herba_mystica && herb2 == HerbaSelection::sweet_herba_mystica &&
        type == PokemonType::bug
    ){
        return "2x Sweet herbs are incompatible with bug type.";
    }
    if ((herb1 == HerbaSelection::salty_herba_mystica && herb2 == HerbaSelection::sour_herba_mystica) ||
        (herb1 == HerbaSelection::sour_herba_mystica && herb2 == HerbaSelection::salty_herba_mystica)
    ){
        static const std::set<PokemonType> TYPES{
            PokemonType::flying,
            PokemonType::ground,
            PokemonType::rock,
            PokemonType::ice,
        };
        if (!TYPES.contains(type)){
            return "1x Salty and 1x Sour herb are only compatible with flying, ground, rock, and ice.";
        }
    }
    if ((herb1 == HerbaSelection::bitter_herba_mystica && herb2 == HerbaSelection::sour_herba_mystica) ||
        (herb1 == HerbaSelection::sour_herba_mystica && herb2 == HerbaSelection::bitter_herba_mystica)
    ){
        static const std::set<PokemonType> TYPES{
            PokemonType::fighting,
            PokemonType::bug,
            PokemonType::fairy,
        };
        if (TYPES.contains(type)){
            return "1x Sour and 1x Bitter herb are incompatible with fighting, bug, and fairy.";
        }
    }

    return "";
}



}
}
}
