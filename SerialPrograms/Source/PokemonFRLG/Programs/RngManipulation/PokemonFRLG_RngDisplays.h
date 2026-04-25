/*  RNG Displays
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngDisplays_H
#define PokemonAutomation_PokemonFRLG_RngDisplays_H

#include <vector>
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_AdvRng.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;

class SidHelperDisplay: public GroupOption{
public:
    SidHelperDisplay();

    void set(uint16_t trainerId, const std::vector<std::pair<std::string, std::string>>& sid_messages);

private:
    static std::string get_sids_string(const std::vector<std::pair<std::string, std::string>>& sid_messages);

public:
    StringOption tid;
    TextEditOption sids;
};

class RngFilterDisplay : public GroupOption{
public:
    RngFilterDisplay();

    void set(const AdvRngFilters& filter);
    void reset();

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
    void set(const std::map<AdvRngState, AdvPokemonResult>& hits_map);
    void reset();

private:
    static std::vector<AdvRngState> get_rng_states_from_map(const std::map<AdvRngState, AdvPokemonResult>& hits_map);
    static std::string get_hits_string(const std::vector<AdvRngState>& rng_states);
    static std::string get_hits_string(const std::map<AdvRngState, AdvPokemonResult>& hits_map);
public:
    StringOption hits;
};

}
}
}
#endif
