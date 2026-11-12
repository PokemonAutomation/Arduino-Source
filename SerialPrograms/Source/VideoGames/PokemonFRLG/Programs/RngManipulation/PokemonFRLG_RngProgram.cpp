/*  RNG Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include <string>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonFRLG/Inference/Menus/PokemonFRLG_PartySlot.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_RngProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

std::string not_enough_free_slots_message(size_t party_size, size_t free_slots_needed){
    if (free_slots_needed == 1){
        return std::format(
            "Your party is full. Leave a party slot free so the program can check the {} it catches or receives.",
            Pokemon::STRING_POKEMON
        );
    }
    return std::format(
        "Your party has {} {}. Leave at least {} party slots free for the {} the program catches or receives.",
        party_size, Pokemon::STRING_POKEMON, free_slots_needed, Pokemon::STRING_POKEMON
    );
}

}  // namespace


void RngProgramInstance::check_free_party_slots(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    const size_t free_slots_needed = free_party_slots_needed();
    if (free_slots_needed == 0){
        return;
    }
    Logger& logger = env.console.logger();
    logger.log("Checking for " + std::to_string(free_slots_needed) + " free party slot(s)...");
    try{
        open_party_menu_from_overworld(env.console, context);
    }catch (OperationFailedException&){
        logger.log("Unable to open the party menu. Skipping the free party slot check.", COLOR_RED);
        return;
    }
    context.wait_for_all_requests();
    constexpr size_t MAX_PARTY_SIZE = 6;
    const size_t party_size = static_cast<size_t>(detect_last_occupied_party_slot(env.console)) + 1;
    if (MAX_PARTY_SIZE - party_size < free_slots_needed){
        throw UserSetupError(logger, not_enough_free_slots_message(party_size, free_slots_needed));
    }
    logger.log("Party has enough free slots.");
}

void RngProgramInstance::run_precheck(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    home_black_border_check(env.console, context);
    check_free_party_slots(env, context);
}

}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
