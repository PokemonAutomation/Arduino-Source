/*  BDSP RNG Displays
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_RngDisplays_H
#define PokemonAutomation_PokemonBDSP_RngDisplays_H

#include <stdint.h>
#include <string>
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Pokemon/Pokemon_BdspRng.h"
#include "Pokemon/Pokemon_Xorshift128.h"
#include "PokemonBDSP_BlinkModel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class RngStateDisplay : public GroupOption{
public:
    RngStateDisplay();

    void set_state(const Pokemon::Xorshift128State& state, uint64_t advances);
    void set_advances(uint64_t advances);
    void set_confidence_unique();

    void reset();

public:
    StringOption state;
    StringOption pokefinder_seeds;
    StringOption advances;
    StringOption confidence;
};


//  Progress through a blink capture.
class BlinkCollectionDisplay : public GroupOption{
public:
    explicit BlinkCollectionDisplay(bool blinks_only = false);

    void set_progress(size_t collected, size_t wanted);
    void set_note(const std::string& text);

    void reset();

public:
    StringOption progress;
};


class RngTargetDisplay : public GroupOption{
public:
    RngTargetDisplay();

    void set_target(const Pokemon::BdspPokemonResult& pokemon, uint64_t advances);
    void set_note(const std::string& reason);
    void set_correction(int64_t advances);

    void reset();

public:
    StringOption target_advance;
    StringOption details;
    StringOption correction;
};

}
}
}
#endif
