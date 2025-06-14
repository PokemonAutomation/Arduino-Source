/*  Pokemon Forms
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonForms_H
#define PokemonAutomation_Pokemon_PokemonForms_H

#include <string>
#include <vector>

namespace PokemonAutomation{
namespace Pokemon{


// slug and display names related to a pokemon form
class PokemonForm{
public:
    const std::string& slug() const{ return m_slug; }
    const std::string& display_name() const{ return m_display_name; }
    bool is_shiny() const;
    // slug of the base species
    const std::string& species() const { return m_species; }
    // const PokemonForm& shiny_form() const;
    // const PokemonForm& non_shiny_form() const;

private:
    PokemonForm(std::string slug, std::string display_name, std::string species)
    : m_slug(std::move(slug)), m_display_name(std::move(display_name)), m_species(std::move(species)) {}
    friend struct PokemonFormDatabase;

    std::string m_slug;
    std::string m_display_name;
    std::string m_species;
};


// Given a slug, find the form related data
// You can find all form slugs with unique appearance from the map keys of:
// Pokemon_PokemonHomeSprites.h:ALL_POKEMON_HOME_SPRITES().get()
// If no such slug is found, return nullptr
const PokemonForm* get_pokemon_form(const std::string& form_slug);

// Get a vector of form slugs. Each slug is a form with unique appearance
// that has a unique sprite loaded in Pokemon_PokemonHomeSprites.h:ALL_POKEMON_HOME_SPRITES()
const std::vector<std::string>& ALL_POKEMON_FORMS();

}
}
#endif
