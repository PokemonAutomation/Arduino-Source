/*  Wild Encounter Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_WildEncounterReader_H
#define PokemonAutomation_PokemonFRLG_WildEncounterReader_H

#include <optional>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"


namespace PokemonAutomation{

class Logger;
class ImageViewRGB32;
class VideoOverlaySet;

namespace NintendoSwitch{
namespace PokemonFRLG{

struct PokemonFRLG_WildEncounter{
    std::string name;
    std::optional<unsigned> level;
    // gender
};

class WildEncounterReader {
public:
    WildEncounterReader(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet &items) const;

    // Reads species from the top left of the battle screen
    // by matching to the provided subset of species 
    void read_encounter(
        Logger &logger, Language language,
        const ImageViewRGB32 &frame, 
        std::set<std::string> &subset, 
        PokemonFRLG_WildEncounter &encounter
    );

private:
    Color m_color;
    ImageFloatBox m_box_name;
    // ImageFloatBox m_box_level;
};

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
#endif

