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
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonFRLG_RngCalibration.h"
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

class RngTargetDisplay : public StringOption{
public:
    RngTargetDisplay()
        : StringOption(
            false,
            "<b>Target Details:</b>",
            LockMode::READ_ONLY,
            "-", ""
        )
    {}

    void set_target(const AdvPokemonResult& pokemon, const int16_t& gender_threshold);
    void set_target(const AdvWildPokemonResult& pokemon, const int16_t& gender_threshold);

private:
    std::string result_to_string(const AdvPokemonResult& pokemon, const int16_t& gender_threshold);
    std::string result_to_string(const AdvWildPokemonResult& pokemon, const int16_t& gender_threshold);
};

class RngFilterDisplay : public GroupOption{
public:
    RngFilterDisplay();

    void set(const AdvRngFilters& filter);
    void reset();

private:
    static std::string get_range_string(const IvRange& range);

public:
    StringOption species;
    StringOption gender;
    StringOption nature;
    StringOption level;
    StringOption hp;
    StringOption atk;
    StringOption def;
    StringOption spatk;
    StringOption spdef;
    StringOption speed;
};


class RngCalibrationDisplay : public GroupOption{
public:
    RngCalibrationDisplay();

    void set_calibrations(const RngCalibrations& calibrations);
    void set_hits(const std::vector<AdvRngState>& rng_states);
    void reset_hits();

private:
    static std::string get_hits_string(const std::vector<AdvRngState>& rng_states);
public:
    SimpleIntegerOption<int64_t> seed_calibration;
    FloatingPointOption csf_calibration;
    FloatingPointOption advances_calibration;
    StringOption hits;
};

}
}
}
#endif
