/*  RNG Displays
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngDisplays_H
#define PokemonAutomation_PokemonFRLG_RngDisplays_H

#include <vector>
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_AdvRng.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;

class RngFilterDisplay : public GroupOption{
public:
    RngFilterDisplay();

    void set(const AdvRngFilters& filter);

private:
    static std::string get_range_string(const IvRange& range);
    static std::string get_gender_string(const AdvGender& gender);
    static std::string get_nature_string(const AdvNature& nature);

public:
    StringOption hp;
    StringOption atk;
    StringOption def;
    StringOption spatk;
    StringOption spdef;
    StringOption speed;
    StringOption gender;
    StringOption nature;
};


class PossibleHitsDisplay : public GroupOption{
public:
    PossibleHitsDisplay();

    void set(const std::vector<AdvRngState>& rng_states);
    void set(std::map<AdvRngState, AdvPokemonResult>& hits_map);

private:
    static std::vector<AdvRngState> get_rng_states_from_map(std::map<AdvRngState, AdvPokemonResult>& hits_map);
    static std::string get_seeds_string(const std::vector<AdvRngState>& rng_states);
    static std::string get_seeds_string(std::map<AdvRngState, AdvPokemonResult>& hits_map);
    static std::string get_advances_string(const std::vector<AdvRngState>& rng_states); 
    static std::string get_advances_string(std::map<AdvRngState, AdvPokemonResult>& hits_map);
public:
    StringOption seeds;
    StringOption advances;
};

}
}
}
#endif
