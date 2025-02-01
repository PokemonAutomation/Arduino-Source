/*  Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.h"
#include "PokemonBDSP/Inference/PokemonBDSP_PokeballSpriteMatcher.h"
#include "PokemonHome_BallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


const double BallReader::MAX_ALPHA = 0.40;
const double BallReader::ALPHA_SPREAD = 0.02;


const PokemonBDSP::PokeballSpriteMatcher& BALL_SPRITE_MATCHER(){
    static PokemonBDSP::PokeballSpriteMatcher matcher;
    return matcher;
}



BallReader::BallReader(VideoStream& stream)
    : m_matcher(BALL_SPRITE_MATCHER())
    , m_stream(stream)
    , m_box_sprite(stream.overlay(), {0.228, 0.095, 0.030, 0.049})
{}



std::string BallReader::read_ball(const ImageViewRGB32& screen) const{
    if (!screen){
        return "";
    }

    ImageMatch::ImageMatchResult sprite_result;
    {
        ImageViewRGB32 image = extract_box_reference(screen, m_box_sprite);
        sprite_result = m_matcher.match(image, ALPHA_SPREAD);
        sprite_result.log(m_stream.logger(), 0.50);
        if (!sprite_result.results.empty() && sprite_result.results.begin()->first > MAX_ALPHA){
            sprite_result.results.clear();
        }
    }

    if (sprite_result.results.size() != 1){
        dump_image(m_stream.logger(), ProgramInfo(), "BallReader", screen);
    }
    if (sprite_result.results.empty()){
        return "";
    }

    return sprite_result.results.begin()->second;
}



}
}
}
