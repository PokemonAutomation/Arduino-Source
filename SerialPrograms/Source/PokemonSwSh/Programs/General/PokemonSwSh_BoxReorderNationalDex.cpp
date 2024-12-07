/*  Box Reorder National Dex
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <sstream>
#include <map>
#include "CommonFramework/Language.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/OCR/OCR_TextMatcher.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_BoxReorderNationalDex.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


namespace{

constexpr uint16_t k_wait_after_move = TICKS_PER_SECOND / 1.5;
constexpr std::chrono::milliseconds k_wait_after_read = std::chrono::milliseconds(200);

// A location can be represented as a uint16_t, meaning the order of the location starting at the first box.
// This function decodes this location into box ID and in-box 2D location.
std::tuple<uint16_t, uint16_t, uint16_t> get_location(size_t index){
    uint16_t box = (uint16_t)(index / 30);
    index = index % 30;

    uint16_t row = (uint16_t)(index / 6);
    index = index % 6;

    uint16_t column = (uint16_t)(index);
    return { box, row, column };
}

// Move cursor from one location to another
// The location is represented as a uint16_t, meaning the order of the location starting at the first box.
// Decode this location into box ID and in-box 2D location by `get_location()`
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

    // TODO: can make this more efficient by moving past grid boundary to appear at the other side

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

// Read the current displayed pokemon name. Return the pokemon slug.
//
// A slug is a unique name used in a program, different than the name that is displayed to user on UI.
// In most cases, a pokemon slug is the lower-case version of the Pokemon name, but there are some cases
// like the slug of the Pokemon Mr. Mime is "mr-mime".
std::string read_selected_pokemon(ConsoleHandle& console, BotBaseContext& context, Language language){
    context.wait_for_all_requests();
    OverlayBoxScope box(console, ImageFloatBox(0.76, 0.08, 0.15, 0.064));
    context.wait_for(k_wait_after_read);

    VideoSnapshot screen = console.video().snapshot();
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

// Move through some pokemon according to the box location order and read their names.
// Return a list of pokemon slugs.
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
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:BoxReorderNationalDex",
        STRING_POKEMON + " SwSh", "Box Reorder National Dex",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/BoxReorderNationalDex.md",
        "Order boxes of " + STRING_POKEMON + ".",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



BoxReorderNationalDex::BoxReorderNationalDex()
    : LANGUAGE(
        "<b>Game Language:</b><br>This needs to be set correctly for " + STRING_POKEMON + " to be identified correctly.",
        PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , POKEMON_COUNT(
        "<b>Number of " + STRING_POKEMON + " to order:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        30
    )
    , DODGE_SYSTEM_UPDATE_WINDOW(
        "<b>Dodge System Update Window:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(POKEMON_COUNT);
    PA_ADD_OPTION(DODGE_SYSTEM_UPDATE_WINDOW);
}

void BoxReorderNationalDex::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, DODGE_SYSTEM_UPDATE_WINDOW);
    }else{
        pbf_press_button(context, BUTTON_LCLICK, 5, 5);
    }

    // Read all the pokemon and return a list of their slugs in the current box order.
    std::vector<std::string> current_order = read_all_pokemon(env.console, env.console, context, POKEMON_COUNT, LANGUAGE);

    // The list of pokemon slugs in national order.
    const std::vector<std::string>& dex_slugs = NATIONAL_DEX_SLUGS();
    // Build a map of slug -> national dex ID for fast lookup
    std::map<std::string, size_t> dex_slug_order;
    for(size_t i = 0; i < dex_slugs.size(); ++i){
        dex_slug_order.emplace(dex_slugs[i], i);
    }

    // check if we have any pokemon name read failure.
    for(size_t i = 0; i < current_order.size(); ++i){
        const auto it = dex_slug_order.find(current_order[i]);
        // If OCR gives a name that is not in the nation dex, throw an error:
        if (it == dex_slug_order.end()){
            const auto [box, row, col] = get_location(i);
            std::stringstream os;
            os << "Failed to read pokemon name at box " << box << " row " << row << " col " << col;
            OperationFailedException::fire(
                env.console, ErrorReport::SEND_ERROR_REPORT,
                os.str()
            );
        }
    }
    
    // Sort the read pokemon by the dex order.
    std::vector<std::string> sorted_order = current_order;

    std::sort(sorted_order.begin(), sorted_order.end(), [&](const std::string& str1, const std::string& str2){
        const size_t id1 = dex_slug_order.find(str1)->second;
        const size_t id2 = dex_slug_order.find(str2)->second;
        return id1 < id2;
    });

    // Now the cursor is at the last pokemon:
    uint16_t current_location = POKEMON_COUNT - 1;
    for (uint16_t index = 0; index < current_order.size(); ++index){
        if (current_order[index] == sorted_order[index]){
            continue;
        }

        const auto it = std::find(current_order.begin() + index, current_order.end(), sorted_order[index]);
        // Where the pokemon should be moved from
        const uint16_t unsorted_location = it - current_order.begin();
        // Where the pokemon should be moved to
        const uint16_t sorted_location = index;

        std::ostringstream ss;
        ss << "Swapping " << current_order[unsorted_location] << " at location index " << unsorted_location << " and " << current_order[sorted_location] << " at location index " << sorted_location;
        env.console.log(ss.str());
        // Move the cursor to the unsorted_location
        current_location = move_to_location(env.console, context, current_location, unsorted_location);
        // Press A to grab the pokemon
        pbf_press_button(context, BUTTON_A, 10, k_wait_after_move);
        // Move the cursor to the sorted_location
        current_location = move_to_location(env.console, context, current_location, sorted_location);
        // Press A to finish swapping the pokemon
        pbf_press_button(context, BUTTON_A, 10, k_wait_after_move);
        // Now the order in the box is changed, update `current_order` to reflect this change.
        std::swap(current_order[unsorted_location], current_order[sorted_location]);
    }

    // Go to Switch home menu
    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().HOME_TO_GAME_DELAY);
}



}
}
}

