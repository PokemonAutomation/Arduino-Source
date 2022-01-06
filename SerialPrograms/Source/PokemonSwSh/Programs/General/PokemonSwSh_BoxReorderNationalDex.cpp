/*  Box Reorder National Dex
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <sstream>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/OCR/OCR_TextMatcher.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Programs/ReleaseHelpers.h"
#include "PokemonSwSh_BoxReorderNationalDex.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

namespace{
    constexpr uint16_t k_wait_after_move = TICKS_PER_SECOND / 1.5;
    constexpr std::chrono::milliseconds k_wait_after_read = std::chrono::milliseconds(200);

    std::tuple<uint16_t, uint16_t, uint16_t> get_location(uint16_t index){
        uint16_t box = index / 30;
        index = index % 30;

        uint16_t row = index / 6;
        index = index % 6;

        uint16_t column = index;
        return { box, row, column };
    }
    
    uint16_t move_to_location(SingleSwitchProgramEnvironment& env, uint16_t from, uint16_t to){
        auto [from_box, from_row, from_column] = get_location(from);
        auto [to_box, to_row, to_column] = get_location(to);

        std::stringstream ss;
        ss << "Moving from location index " << from << "(" << from_box << "/" << from_row << "/" << from_column << ")";
        ss << " to location index " << to << "(" << to_box << "/" << to_row << "/" << to_column << ")";
        env.log(ss.str());

        int difference_box = to_box - from_box;
        int difference_row = to_row - from_row;
        int difference_column = to_column - from_column;
        for (int i = 0; i < difference_box; ++i){
            pbf_press_button(env.console, BUTTON_R, 10, k_wait_after_move);
        }
        for (int i = 0; i > difference_box; --i){
            pbf_press_button(env.console, BUTTON_L, 10, k_wait_after_move);
        }

        for (int i = 0; i < difference_row; ++i){
            pbf_press_dpad(env.console, DPAD_DOWN, 10, k_wait_after_move);
        }
        for (int i = 0; i > difference_row; --i){
            pbf_press_dpad(env.console, DPAD_UP, 10, k_wait_after_move);
        }

        for (int i = 0; i < difference_column; ++i){
            pbf_press_dpad(env.console, DPAD_RIGHT, 10, k_wait_after_move);
        }
        for (int i = 0; i > difference_column; --i){
            pbf_press_dpad(env.console, DPAD_LEFT, 10, k_wait_after_move);
        }
        return to;
    }

    std::string read_selected_pokemon(ProgramEnvironment& env, ConsoleHandle& console, Language language){
        console.botbase().wait_for_all_requests();
        InferenceBoxScope box(console, ImageFloatBox(0.76, 0.08, 0.15, 0.064));
        env.wait_for(k_wait_after_read);

        QImage screen = console.video().snapshot();
        QImage frame = extract_box(screen, box);

        OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(console, language, frame);
        static constexpr double MAX_LOG10P = -1.40;
        result.log(console, MAX_LOG10P);
        assert(result.results.size() == 1);
        return result.results.begin()->second.token;
    }

    std::vector<std::string> read_all_pokemon(SingleSwitchProgramEnvironment& env, uint16_t pokemon_count, Language language){
        std::vector<std::string> pokemons;
        uint16_t current_location = 0;
        for (uint16_t i = 0; i < pokemon_count; ++i){
            current_location = move_to_location(env, current_location, i);
            pokemons.push_back(read_selected_pokemon(env, env.console, language));
        }
        return pokemons;
    }
}

BoxReorderNationalDex_Descriptor::BoxReorderNationalDex_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:BoxReorderNationalDex",
        STRING_POKEMON + " SwSh", "Box Reorder National Dex",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/BoxReorderNationalDex.md",
        "Order boxes of " + STRING_POKEMON + ".",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



BoxReorderNationalDex::BoxReorderNationalDex(const BoxReorderNationalDex_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE(
        "<b>Game Language:</b><br>This needs to be set correctly for " + STRING_POKEMON + " to be identified correctly.",
        Pokemon::PokemonNameReader::instance().languages(), true
    )
    , POKEMON_COUNT(
        "<b>Number of " + STRING_POKEMON + " to order:</b>",
        30
    )
    , DODGE_SYSTEM_UPDATE_WINDOW(
        "<b>Dodge System Update Window:</b>",
        false
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(POKEMON_COUNT);
    PA_ADD_OPTION(DODGE_SYSTEM_UPDATE_WINDOW);
}

void BoxReorderNationalDex::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_no_interact(env.console, DODGE_SYSTEM_UPDATE_WINDOW);
    }else{
        pbf_press_button(env.console, BUTTON_LCLICK, 5, 5);
    }

    std::vector<std::string> current_order = read_all_pokemon(env, POKEMON_COUNT, LANGUAGE);

    QJsonArray array = read_json_file(
        RESOURCE_PATH() + "Pokemon/Pokedex/Pokedex-National.json"
    ).array();

    std::vector<std::string> slugs;
    for (const auto& item : array) {
        QString slug = item.toString();
        if (slug.size() <= 0) {
            PA_THROW_StringException("Expected non-empty string for Pokemon slug.");
        }
        slugs.emplace_back(slug.toUtf8().data());
    }
    std::vector<std::string> sorted_order = current_order;
    std::sort(sorted_order.begin(), sorted_order.end(), [&](const std::string& str1, const std::string& str2){
        auto it1 = std::find(slugs.cbegin(), slugs.cend(), str1);
        auto it2 = std::find(slugs.cbegin(), slugs.cend(), str2);
        return it1 < it2;
    });

    uint16_t current_location = POKEMON_COUNT - 1;
    for (uint16_t index = 0; index < current_order.size(); ++index){
        if (current_order[index] == sorted_order[index]){
            continue;
        }

        auto it = std::find(current_order.begin() + index, current_order.end(), sorted_order[index]);
        uint16_t unsorted_location = it - current_order.begin();
        uint16_t sorted_location = index;

        std::stringstream ss;
        ss << "Swapping " << current_order[unsorted_location] << " at location index " << unsorted_location << " and " << current_order[sorted_location] << " at location index " << sorted_location;
        env.log(ss.str());
        current_location = move_to_location(env, current_location, unsorted_location);
        pbf_press_button(env.console, BUTTON_A, 10, k_wait_after_move);
        current_location = move_to_location(env, current_location, sorted_location);
        pbf_press_button(env.console, BUTTON_A, 10, k_wait_after_move);
        std::swap(current_order[unsorted_location], current_order[sorted_location]);
    }
    pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().HOME_TO_GAME_DELAY);
}



}
}
}

