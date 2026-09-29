/*  Pokemon Scarlet/Violet Ingredients
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_Ingredients_H
#define PokemonAutomation_PokemonSV_Ingredients_H

#include <vector>
#include <map>
#include "CommonFramework/Language.h"
#include "CommonTools/Resources/SpriteDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


const std::vector<std::string>& ALL_SANDWICH_FILLINGS_SLUGS();
const std::vector<std::string>& ALL_SANDWICH_CONDIMENTS_SLUGS();

class SandwichIngredientNames{
public:
    const std::string& display_name() const{ return m_display_name; }

private:
    friend struct IngredientNameDatabase;

    std::string m_display_name;
    std::map<Language, std::string> m_display_names;
};

const SandwichIngredientNames& get_ingredient_name(const std::string& slug);
const std::string& parse_ingredient_name(const std::string& display_name);



const SpriteDatabase& SANDWICH_FILLINGS_DATABASE();
const SpriteDatabase& SANDWICH_CONDIMENTS_DATABASE();


}
}
}
#endif
