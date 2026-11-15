/*  FRLG Prechecks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include <memory>
#include <string>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG_Navigation.h"
#include "VideoGames/PokemonFRLG/Inference/Menus/PokemonFRLG_PartySlot.h"
#include "PokemonFRLG_Prechecks.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

namespace{

constexpr size_t MAX_PARTY_SIZE = 6;


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


class BlackBorderPrecheck : public Precheck{
public:
    BlackBorderPrecheck()
        : Precheck(
            "black_border",
            "Screen size at 100% with no black border, and the controller connected.",
            PrecheckPolicy::ALWAYS_RUNS
        )
    {}
    virtual PrecheckOutcome run(SingleSwitchProgramEnvironment& env, ProControllerContext& context) const override{
        home_black_border_check(env.console, context);
        return {PrecheckStatus::PASSED, ""};
    }
};


class FreePartySlotsPrecheck : public Precheck{
public:
    explicit FreePartySlotsPrecheck(size_t free_slots_needed)
        : Precheck(
            "free_party_slots",
            std::format("At least {} free party slot(s), on the overworld where the game was saved.", free_slots_needed),
            PrecheckPolicy::SKIPPABLE
        )
        , m_free_slots_needed(free_slots_needed)
    {}
    virtual PrecheckOutcome run(SingleSwitchProgramEnvironment& env, ProControllerContext& context) const override{
        try{
            open_party_menu_from_overworld(env.console, context);
        }catch (OperationFailedException&){
            return {PrecheckStatus::SKIPPED, "Unable to open the party menu."};
        }
        context.wait_for_all_requests();
        const size_t party_size = static_cast<size_t>(detect_last_occupied_party_slot(env.console)) + 1;
        return free_party_slots_outcome(party_size, m_free_slots_needed);
    }

private:
    size_t m_free_slots_needed;
};


}  // namespace


std::unique_ptr<const Precheck> make_black_border_precheck(){
    return std::make_unique<BlackBorderPrecheck>();
}

std::unique_ptr<const Precheck> make_free_party_slots_precheck(size_t free_slots_needed){
    return std::make_unique<FreePartySlotsPrecheck>(free_slots_needed);
}

PrecheckOutcome free_party_slots_outcome(size_t party_size, size_t free_slots_needed){
    if (party_size + free_slots_needed > MAX_PARTY_SIZE){
        return {PrecheckStatus::FAILED, not_enough_free_slots_message(party_size, free_slots_needed)};
    }
    return {PrecheckStatus::PASSED, "Party has enough free slots."};
}


}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
