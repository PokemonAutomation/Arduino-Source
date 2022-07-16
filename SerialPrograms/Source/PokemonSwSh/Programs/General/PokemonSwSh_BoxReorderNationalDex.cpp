/*  Box Reorder National Dex
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <sstream>
#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTypes/RGB32ImageView.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/OCR/OCR_TextMatcher.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Programs/ReleaseHelpers.h"
#include "PokemonSwSh_BoxReorderNationalDex.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;



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
    
    uint16_t move_to_location(Logger& logger, BotBaseContext& context, uint16_t from, uint16_t to){
        auto [from_box, from_row, from_column] = get_location(from);
        auto [to_box, to_row, to_column] = get_location(to);

        std::ostringstream ss;
        ss << "Moving from location index " << from << "(" << from_box << "/" << from_row << "/" << from_column << ")";
        ss << " to location index " << to << "(" << to_box << "/" << to_row << "/" << to_column << ")";
        logger.log(ss.str());

        int difference_box = to_box - from_box;
        int difference_row = to_row - from_row;
        int difference_column = to_column - from_column;
        for (int i = 0; i < difference_box; ++i){
            pbf_press_button(context, BUTTON_R, 10, k_wait_after_move);
        }
        for (int i = 0; i > difference_box; --i){
            pbf_press_button(context, BUTTON_L, 10, k_wait_after_move);
        }

        for (int i = 0; i < difference_row; ++i){
            pbf_press_dpad(context, DPAD_DOWN, 10, k_wait_after_move);
        }
        for (int i = 0; i > difference_row; --i){
            pbf_press_dpad(context, DPAD_UP, 10, k_wait_after_move);
        }

        for (int i = 0; i < difference_column; ++i){
            pbf_press_dpad(context, DPAD_RIGHT, 10, k_wait_after_move);
        }
        for (int i = 0; i > difference_column; --i){
            pbf_press_dpad(context, DPAD_LEFT, 10, k_wait_after_move);
        }
        return to;
    }

    std::string read_selected_pokemon(ConsoleHandle& console, BotBaseContext& context, Language language){
        context.wait_for_all_requests();
        InferenceBoxScope box(console, ImageFloatBox(0.76, 0.08, 0.15, 0.064));
        context.wait_for(k_wait_after_read);

        QImage screen = console.video().snapshot();
        ImageViewRGB32 frame = extract_box_reference(screen, box);

        OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
            console, language, frame,
            OCR::BLACK_TEXT_FILTERS()
        );
        result.log(console, PokemonNameReader::MAX_LOG10P);
//        assert(result.results.size() == 1);
        if (result.results.size() != 1){
            return "";
        }
        return result.results.begin()->second.token;
    }

    std::vector<std::string> read_all_pokemon(Logger& logger, ConsoleHandle& console, BotBaseContext& context, uint16_t pokemon_count, Language language){
        std::vector<std::string> pokemons;
        uint16_t current_location = 0;
        for (uint16_t i = 0; i < pokemon_count; ++i){
            current_location = move_to_location(logger, context, current_location, i);
            pokemons.push_back(read_selected_pokemon(console, context, language));
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
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



BoxReorderNationalDex::BoxReorderNationalDex(const BoxReorderNationalDex_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE(
        "<b>Game Language:</b><br>This needs to be set correctly for " + STRING_POKEMON + " to be identified correctly.",
        PokemonNameReader::instance().languages(), true
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

void BoxReorderNationalDex::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(context, DODGE_SYSTEM_UPDATE_WINDOW);
    }else{
        pbf_press_button(context, BUTTON_LCLICK, 5, 5);
    }

    std::vector<std::string> current_order = read_all_pokemon(env.console, env.console, context, POKEMON_COUNT, LANGUAGE);

    const std::vector<std::string>& slugs = NATIONAL_DEX_SLUGS();
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

        std::ostringstream ss;
        ss << "Swapping " << current_order[unsorted_location] << " at location index " << unsorted_location << " and " << current_order[sorted_location] << " at location index " << sorted_location;
        env.console.log(ss.str());
        current_location = move_to_location(env.console, context, current_location, unsorted_location);
        pbf_press_button(context, BUTTON_A, 10, k_wait_after_move);
        current_location = move_to_location(env.console, context, current_location, sorted_location);
        pbf_press_button(context, BUTTON_A, 10, k_wait_after_move);
        std::swap(current_order[unsorted_location], current_order[sorted_location]);
    }
    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().HOME_TO_GAME_DELAY);
}



}
}
}

