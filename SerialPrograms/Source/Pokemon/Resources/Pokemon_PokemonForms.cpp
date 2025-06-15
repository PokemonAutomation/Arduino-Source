/*  Pokemon Forms
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <set>
#include <fstream>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/Globals.h"
#include "Pokemon_PokemonNames.h"
#include "Pokemon_PokemonForms.h"
#include "Pokemon_PokemonSlugs.h"

namespace PokemonAutomation{
namespace Pokemon{


struct PokemonFormDatabase{
    PokemonFormDatabase();
    static const PokemonFormDatabase& instance(){
        static PokemonFormDatabase database;
        return database;
    }

    std::map<std::string, PokemonForm> m_slug_to_form;
    std::vector<std::string> m_slugs;
};

bool is_form_shiny(const std::string& form_slug){
    const std::string& suffix = "-shiny";
    if (form_slug.length() < suffix.length()) {
           return false;
       }
    return form_slug.compare(form_slug.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool PokemonForm::is_shiny() const {
    return is_form_shiny(m_slug);
}

PokemonFormDatabase::PokemonFormDatabase(){
    // Load form map JSON
    const std::string form_display_map_path = RESOURCE_PATH() + "Pokemon/AllFormDisplayMap.json";
    JsonValue json = load_json_file(form_display_map_path);
    // a map from form slug to list of tuples, where each tuple contains a form slug and a display name
    JsonObject& form_displays = json.to_object_throw(form_display_map_path);

    const std::string no_shiny_pokemon_path = RESOURCE_PATH() + "Pokemon/SpecialPokemonWithNoShinyForm.txt";
    std::ifstream fin(no_shiny_pokemon_path);
    if (!fin.is_open()) {
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "canot open resource file: " + no_shiny_pokemon_path);
    }
    std::string line;
    std::set<std::string> no_shiny_forms;
    while (std::getline(fin, line)) {
        fin >> std::ws;
        no_shiny_forms.emplace(std::move(line));
    }

    for(const auto& species_slug: NATIONAL_DEX_SLUGS()){
        const std::string& species_display_name = get_pokemon_name(species_slug).display_name();
        const JsonArray* form_array = form_displays.get_array(species_slug);
        if (form_array == nullptr){
            // there is a single form for this species
            m_slug_to_form.emplace(species_slug, PokemonForm(species_slug, species_display_name, species_slug));
            std::string shiny_slug = species_slug + "-shiny";
            m_slug_to_form.emplace(shiny_slug, PokemonForm(shiny_slug, "Shiny " + species_display_name, species_slug));
            m_slugs.push_back(species_slug);
            m_slugs.emplace_back(std::move(shiny_slug));
            continue;
        }

        bool has_shiny_form = false;
        std::vector<std::map<std::string, PokemonForm>::const_iterator> forms;
        for (const auto& form : *form_array){
            // form is a list of two elements, first the form slug name, second the display name
            const JsonArray* form_ptr = form.to_array();
            if (form_ptr == nullptr){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No [form_slug, display_name] format in form display map: " + species_slug);
            }
            const JsonArray& form_slug_and_display = *form_ptr;
            if (form_slug_and_display.size() != 2){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Wrong [form_slug, display_name] length in form display map: " + species_slug);
            }
            const std::string& form_slug = form_slug_and_display[0].to_string_throw();
            const std::string& display_name = form_slug_and_display[1].to_string_throw();
            if (form_slug.empty() || display_name.empty()){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Empty string in [form_slug, display_name] in form display map: " + species_slug);
            }
            if (is_form_shiny(form_slug)){
                has_shiny_form = true;
            }
            const auto it = m_slug_to_form.emplace(form_slug, PokemonForm(form_slug, display_name, species_slug)).first;
            m_slugs.push_back(form_slug);
            forms.push_back(it);
        }
        if (!has_shiny_form){ // if no explict shiny form loaded, we build it our own
            for (const auto& it: forms){
                const auto& form = it->second;   
                if (no_shiny_forms.find(form.m_slug) != no_shiny_forms.end()){
                    continue; // this form has no shiny
                }
                std::string shiny_slug = form.m_slug + "-shiny";
                m_slug_to_form.emplace(shiny_slug, PokemonForm(
                    shiny_slug, "Shiny " + form.m_display_name, form.m_species
                ));
                m_slugs.emplace_back(std::move(shiny_slug));
            }
        }
    }
    std::cout << "Loaded " << m_slug_to_form.size() << " form slugs" << std::endl;
}

const PokemonForm* get_pokemon_form(const std::string& slug){
    const auto& slug_to_form = PokemonFormDatabase::instance().m_slug_to_form;
    const auto it = slug_to_form.find(slug);
    if (it == slug_to_form.end()){
        return nullptr;
    }
    return &it->second;
}

const std::vector<std::string>& ALL_POKEMON_FORMS(){
    return PokemonFormDatabase::instance().m_slugs;
}


}
}