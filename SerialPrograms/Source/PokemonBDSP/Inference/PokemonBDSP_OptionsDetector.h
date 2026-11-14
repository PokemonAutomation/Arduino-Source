/*  Options Menu Detection and OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_OptionsDetector_H
#define PokemonAutomation_PokemonBDSP_OptionsDetector_H

#include <string>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
class ImageViewRGB32;
class Logger;
namespace NintendoSwitch{
namespace PokemonBDSP{

class TutorialOptionsDetector{
public:
    bool detect(const ImageViewRGB32& screen) const;
};

class TutorialOptionPositionDetector{
public:
    // Return the selected screen row's center, or -1 if its outline is absent.
    double selected_row(const ImageViewRGB32& screen) const;
    bool selected_value(const ImageViewRGB32& screen, double y) const;

    // Empty row reads a setting label. Otherwise read only that row's selected
    // value (On/Off uses the selection highlight). Return a Settings.json token,
    // or empty on unknown text or an ambiguous selection.
    std::string read_text(Logger& logger, Language language,
        const ImageViewRGB32& screen, double y, const std::string& row = "") const;
};

}
}
}
#endif
