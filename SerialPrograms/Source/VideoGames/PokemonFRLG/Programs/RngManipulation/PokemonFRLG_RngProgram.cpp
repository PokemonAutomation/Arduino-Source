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
#include "PokemonFRLG_RngProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

class ConsolePartyCheckActions : public PartyCheckActions{
public:
    ConsolePartyCheckActions(ConsoleHandle& console, ProControllerContext& context, StartMenuContext menu_context)
        : m_console(console), m_context(context), m_menu_context(menu_context)
    {}

    virtual bool open_party_menu() override{
        try{
            open_party_menu_from_overworld(m_console, m_context, m_menu_context);
        }catch (OperationFailedException&){
            return false;
        }
        m_context.wait_for_all_requests();
        return true;
    }
    virtual PartySlot read_last_occupied_slot() override{
        return detect_last_occupied_party_slot(m_console);
    }

private:
    ConsoleHandle& m_console;
    ProControllerContext& m_context;
    StartMenuContext m_menu_context;
};

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


void check_party_free_slots(
    Logger& logger,
    size_t free_slots_needed,
    PartyCheckActions& actions
){
    if (free_slots_needed == 0){
        return;
    }
    logger.log("Checking for " + std::to_string(free_slots_needed) + " free party slot(s)...");
    if (!actions.open_party_menu()){
        logger.log("Unable to open the party menu. Skipping the free party slot check.", COLOR_RED);
        return;
    }
    constexpr size_t MAX_PARTY_SIZE = 6;
    const size_t party_size = static_cast<size_t>(actions.read_last_occupied_slot()) + 1;
    if (MAX_PARTY_SIZE - party_size < free_slots_needed){
        throw UserSetupError(logger, not_enough_free_slots_message(party_size, free_slots_needed));
    }
    logger.log("Party has enough free slots.");
}


void RngProgramInstance::start_program_game_checks(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    ConsolePartyCheckActions actions(env.console, context, start_menu_context());
    check_party_free_slots(env.console.logger(), free_party_slots_needed(), actions);
}

}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
