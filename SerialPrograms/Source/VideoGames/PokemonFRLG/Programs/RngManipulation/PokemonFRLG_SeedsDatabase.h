/*  Seeds Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_SeedsDatabase_H
#define PokemonAutomation_PokemonFRLG_SeedsDatabase_H

#include <cstdint>
#include <string>
#include <vector>
#include "CommonFramework/Language.h"
#include "PokemonFRLG_BlindNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


enum class SoundSetting{
    Mono,
    Stereo,
};

static const uint16_t BLACKOUT_SEED_OFFSET = 0x0024;

// Only English and Japanese lists exist; other languages fall back to English.
std::string seeds_json_path(bool firered, Language language);


struct SeedMatch{
    bool found = false;
    std::vector<uint16_t> seed_values;
    int16_t seed_position = -1;
    uint64_t seed_delay = 0;
    SeedButton seed_button = SeedButton::A;
    BlackoutButton extra_button = BlackoutButton::None;
    SoundSetting sound = SoundSetting::Mono;
    std::string button_mode;
    std::string column_name;
};


class SeedsDatabase{
public:
    SeedsDatabase(const std::string& json_path);
    SeedMatch find_seed(uint16_t target_seed, SoundSetting sound, int radius) const;

private:
    struct SeedColumn{
        std::string name;
        SoundSetting sound;
        std::string button_mode;
        SeedButton seed_button;
        std::vector<uint16_t> seeds;
    };

    std::vector<uint64_t> m_delays;
    std::vector<SeedColumn> m_columns;
};



}
}
}
#endif
