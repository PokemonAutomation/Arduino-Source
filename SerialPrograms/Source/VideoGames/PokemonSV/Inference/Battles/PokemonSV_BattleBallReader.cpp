/*  Battle Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "PokemonSV_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


BattleBallReader::BattleBallReader(VideoStream& stream, Language language, Color color)
    : m_name_reader(PokeballNameReader::instance())
    , m_language(language)
    , m_logger(stream.logger())
    , m_name(stream.overlay(), {0.380, 0.687, 0.240, 0.050}, color)
    , m_sprite(stream.overlay(), {0.480, 0.764, 0.040, 0.060}, color)
    , m_quantity(stream.overlay(), {0.480, 0.830, 0.040, 0.035}, color)
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
            m_logger, m_language, cropped,
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
#if 0
    ImageRGB32 image = to_blackwhite_rgb32_range(
        extract_box_reference(screen, m_quantity),
        0xff000000, 0xff7f7f7f, true
    );
    int qty = OCR::read_number(m_logger, image);
#else
    int qty = OCR::read_number_waterfill(
        m_logger,
        extract_box_reference(screen, m_quantity),
        0xff000000, 0xff7f7f7f
    );
    #endif
    return (uint16_t)std::max(qty, 0);
}



}
}
}
