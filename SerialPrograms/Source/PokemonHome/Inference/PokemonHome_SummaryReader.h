/*  Pokemon Home Summary Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_SummaryReader_H
#define PokemonAutomation_PokemonHome_SummaryReader_H

#include <string>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Common/Cpp/Logging/AbstractLogger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

class SummaryReader{
public:
    SummaryReader(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    int read_national_dex(Logger& logger, const ImageViewRGB32& screen) const;
    int read_original_trainer_id(Logger& logger, const ImageViewRGB32& screen) const;
    std::string read_original_trainer_name(Language language, const ImageViewRGB32& screen) const;
    std::string read_nature(Language language, const ImageViewRGB32& screen) const;
    std::string read_ability(Language language, const ImageViewRGB32& screen) const;
    std::string read_language_of_origin(const ImageViewRGB32& screen) const;
    int read_level(Logger& logger, const ImageViewRGB32& screen) const;

private:
    Color m_color;
    ImageFloatBox m_national_dex_number_box;
    ImageFloatBox m_level_box;
    ImageFloatBox m_original_trainer_id_box;
    ImageFloatBox m_original_trainer_name_box;
    ImageFloatBox m_nature_box;
    ImageFloatBox m_ability_box;
    ImageFloatBox m_language_of_origin_box;
};

void add_tests_SummaryReader(UnitTestDatabase& database);

}
}
}
#endif
