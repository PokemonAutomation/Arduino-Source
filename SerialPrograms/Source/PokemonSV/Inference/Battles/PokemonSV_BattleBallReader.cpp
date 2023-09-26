/*  Battle Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "PokemonSV_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


BattleBallReader::BattleBallReader(ConsoleHandle& console, Language language, Color color)
    : m_name_reader(PokeballNameReader::instance())
    , m_language(language)
    , m_console(console)
    , m_name(console.overlay(), {0.380, 0.687, 0.240, 0.050}, color)
    , m_sprite(console.overlay(), {0.480, 0.764, 0.040, 0.060}, color)
    , m_quantity(console.overlay(), {0.480, 0.830, 0.040, 0.035}, color)
{}


std::string BattleBallReader::read_ball(const ImageViewRGB32& screen) const{
    if (!screen){
        return "";
    }

    //  TODO: Use the sprite as well.

    OCR::StringMatchResult name_result;
    {
        ImageViewRGB32 cropped = extract_box_reference(screen, m_name);
        name_result = m_name_reader.read_substring(
            m_console, m_language, cropped,
            {
                {0xffe0e0e0, 0xffffffff},
                {0xffc0c0c0, 0xffffffff},
                {0xffa0a0a0, 0xffffffff},
                {0xff808080, 0xffffffff},
            }
        );
    }
    if (name_result.results.size() != 1){
//        dump_image(m_console, ProgramInfo(), "BattleBallReader-Name", screen);
        return "";
    }

    return name_result.results.begin()->second.token;
}

uint16_t BattleBallReader::read_quantity(const ImageViewRGB32& screen) const{
    ImageRGB32 image = to_blackwhite_rgb32_range(
        extract_box_reference(screen, m_quantity),
        0xff000000, 0xff7f7f7f, true
    );
    int qty = OCR::read_number(m_console, image);
    return (uint16_t)std::max(qty, 0);
}



}
}
}
