/*  Home Box Sorter Living Dex Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "PokemonHome_BoxNavigation.h"
#include "PokemonHome_BoxSorterLivingDex.h"
#include "PokemonHome_BoxSorterLivingDex_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class Test_LivingDexTemplate : public UnitTest{
public:
    Test_LivingDexTemplate(std::string name, std::string input, std::string error = "", bool resource = false)
        : UnitTest("PokemonHome::LivingDexTemplate - " + name)
        , m_input(std::move(input))
        , m_error(std::move(error))
        , m_resource(resource)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const std::string path = m_resource ? RESOURCE_PATH() + "PokemonHome/DexTemplates/" + m_input : "custom-dex.json";
        try{
            JsonValue json = m_resource ? load_json_file(path) : parse_json(m_input);
            validate_living_dex_template(json, path);
        }catch (const ParseException& e){
            if (m_error.empty()){
                return UnitTestResult("Unexpected validation error: " + e.message());
            }
            if (e.message().find(m_error) == std::string::npos || e.message().find(path) == std::string::npos){
                return UnitTestResult("Validation error lacks expected context '" + m_error + "': " + e.message());
            }
            return true;
        }
        return m_error.empty() ? UnitTestResult(true) : UnitTestResult("Expected validation to fail: " + m_error);
    }

private:
    std::string m_input;
    std::string m_error;
    bool m_resource;
};

class Test_LivingDexCustomOption : public UnitTest{
public:
    Test_LivingDexCustomOption()
        : UnitTest("PokemonHome::LivingDexTemplate - Custom file visibility and settings")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        BoxSorterLivingDex sorter;
        PathOption* file = nullptr;
        for (ConfigOption* option : dynamic_cast<BatchOption&>(sorter.options()).options()){
            if (auto* path = dynamic_cast<PathOption*>(option)){
                file = path;
            }
        }
        if (file == nullptr || file->visibility() != ConfigOptionState::HIDDEN){
            return UnitTestResult("Custom file selector must be hidden for the default style.");
        }
        JsonObject settings;
        settings["LIVING_DEX_STYLE"] = "custom";
        settings["CUSTOM_DEX_FILE"] = "custom-dex.json";
        sorter.load_json(settings.clone());
        if (file->visibility() != ConfigOptionState::ENABLED || std::string(*file) != "custom-dex.json"){
            return UnitTestResult("Selecting Custom must show and load the file selector.");
        }
        JsonValue saved = sorter.to_json();
        sorter.restore_defaults();
        if (file->visibility() != ConfigOptionState::HIDDEN){
            return UnitTestResult("Restoring the default style must hide the file selector.");
        }
        sorter.load_json(saved);
        if (file->visibility() != ConfigOptionState::ENABLED || std::string(*file) != "custom-dex.json"){
            return UnitTestResult("Custom style and file must survive a settings round trip.");
        }
        settings["LIVING_DEX_STYLE"] = "sorted_by_forms";
        sorter.load_json(settings.clone());
        return file->visibility() == ConfigOptionState::HIDDEN;
    }
};

void add_tests_LivingDexTemplate(UnitTestDatabase& database){
    for (const char* filename : {
        "living_dex_by_forms.json", "living_dex_by_species.json",
        "ajp_final_form.json", "ajp_final_form_form.json", "ajp_living.json",
        "ajp_living_form.json", "ajp_living_form_lite.json",
        "pokepc_fully_sorted.json", "pokepc_fully_sorted_minimal.json",
        "pokepc_grouped_balanced.json", "pokepc_grouped_region.json",
        "pokepc_sorted_species.json", "pokepc_sorted_species_minimal.json", "pokepc_species_first.json"
    }){
        database.add<Test_LivingDexTemplate>(filename, filename, "", true);
    }

    const JsonValue valid = parse_json(R"([{
        "species_slug":"bulbasaur", "form_slug":"none", "natID":1,
        "primaryType":"grass", "secondaryType":"poison", "slug":"bulbasaur",
        "has_gender_difference":false
    }])");
    database.add<Test_LivingDexTemplate>("Valid Pokemon", valid.dump());
    database.add<Test_LivingDexTemplate>("Malformed JSON", "[{", "JSON array");
    database.add<Test_LivingDexTemplate>("Wrong root", "{}", "JSON array");
    database.add<Test_LivingDexTemplate>("Null root", "null", "JSON array");
    database.add<Test_LivingDexTemplate>("Empty template", "[]", "at least one entry");
    database.add<Test_LivingDexTemplate>("Non-object entry", "[7]", "entry 1");

    // Missing and incorrectly typed required fields must identify the field.
    const JsonObject& original = valid.to_array_throw()[0].to_object_throw();
    for (const char* field : {"species_slug", "form_slug", "natID", "primaryType", "slug", "has_gender_difference"}){
        JsonObject missing;
        for (const auto& item : original){
            if (item.first != field){
                missing[item.first] = item.second.clone();
            }
        }
        JsonArray entries;
        entries.push_back(std::move(missing));
        database.add<Test_LivingDexTemplate>("Missing " + std::string(field), entries.dump(), field);
        JsonValue wrong_type = valid.clone();
        wrong_type.to_array_throw()[0].to_object_throw()[field] = JsonValue();
        database.add<Test_LivingDexTemplate>("Wrong type " + std::string(field), wrong_type.dump(), field);
    }

    auto changed = [&](const std::string& name, const char* field, JsonValue value, const std::string& error){
        JsonValue json = valid.clone();
        json.to_array_throw()[0].to_object_throw()[field] = std::move(value);
        database.add<Test_LivingDexTemplate>(name, json.dump(), error);
    };
    changed("Negative dex number", "natID", -1, "natID");
    changed("Overflowing dex number", "natID", 65536, "natID");
    changed("Fractional dex number", "natID", 1.5, "natID");
    changed("Invalid empty slot", "natID", 0, "empty slot");
    changed("Blank species", "species_slug", "", "species_slug");
    changed("Blank form", "form_slug", " ", "form_slug");
    changed("Blank slug", "slug", "", "slug");
    changed("Unknown primary type", "primaryType", "Grass", "primaryType");
    changed("Missing primary type", "primaryType", "none", "primaryType");
    changed("Unknown secondary type", "secondaryType", "stellar", "secondaryType");
    changed("Wrong secondary type", "secondaryType", false, "secondaryType");
    changed("Null secondary type", "secondaryType", JsonValue(), "");
    changed("None secondary type", "secondaryType", "none", "");
    changed("Missing gender suffix", "has_gender_difference", true, "slug");

    database.add<Test_LivingDexTemplate>("Optional secondary type", R"([{
        "species_slug":"charmander", "form_slug":"none", "natID":4,
        "primaryType":"fire", "slug":"charmander", "has_gender_difference":false
    }])");
    database.add<Test_LivingDexTemplate>("Empty slot", R"([{
        "species_slug":"empty", "form_slug":"", "natID":0,
        "primaryType":"", "secondaryType":null, "slug":"", "has_gender_difference":false
    }])");
    JsonValue gender = valid.clone();
    gender.to_array_throw()[0].to_object_throw()["has_gender_difference"] = true;
    gender.to_array_throw()[0].to_object_throw()["slug"] = "bulbasaur-female";
    database.add<Test_LivingDexTemplate>("Gender suffix", gender.dump());

    JsonValue second_entry = valid.clone();
    second_entry.to_array_throw().push_back(JsonObject());
    database.add<Test_LivingDexTemplate>("Error entry index", second_entry.dump(), "entry 2");
    JsonArray oversized;
    for (size_t i = 0; i <= MAX_HOME_BOXES * 30; i++){
        oversized.push_back(JsonValue());
    }
    database.add<Test_LivingDexTemplate>("Too many slots", oversized.dump(), "capacity");
    database.add<Test_LivingDexCustomOption>();
}


}
}
}
