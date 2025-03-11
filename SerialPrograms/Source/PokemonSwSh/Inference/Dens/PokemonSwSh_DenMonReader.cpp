/*  Den Sprite Identifier
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "CommonTools/ImageMatch/FilterToAlpha.h"
#include "CommonTools/ImageMatch/SilhouetteDictionaryMatcher.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh_DenMonReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ImageMatch::SilhouetteDictionaryMatcher make_DEN_SPRITE_MATCHER(){
    ImageMatch::SilhouetteDictionaryMatcher matcher;
    for (const auto& item : ALL_POKEMON_SILHOUETTES()){
        matcher.add(item.first, item.second.sprite);
    }
    return matcher;
}
const ImageMatch::SilhouetteDictionaryMatcher& DEN_SPRITE_MATCHER(){
    static ImageMatch::SilhouetteDictionaryMatcher matcher = make_DEN_SPRITE_MATCHER();
    return matcher;
}




DenMonReader::DenMonReader(Logger& logger, VideoOverlay& overlay)
    : m_matcher(DEN_SPRITE_MATCHER())
    , m_logger(logger)
    , m_white(overlay, {0.800, 0.200, 0.150, 0.100})
    , m_den_color(overlay, {0.400, 0.050, 0.200, 0.100})
    , m_lair_pink(overlay, {0.575, 0.035, 0.050, 0.100})
    , m_sprite(overlay, {0.098, 0.23, 0.285, 0.41})
{}


DenMonReadResults DenMonReader::read(const ImageViewRGB32& screen) const{
    DenMonReadResults results;
    if (!screen){
        return results;
    }

    ImageStats white = image_stats(extract_box_reference(screen, m_white));
    if (!is_solid(white, {0.303079, 0.356564, 0.340357})){
        return results;
    }
    do{
        ImageStats den_color = image_stats(extract_box_reference(screen, m_den_color));

        if (is_solid(den_color, {0.593023, 0.204651, 0.202326})){
            results.type = DenMonReadResults::RED_BEAM;
            m_logger.log("Den Type: Red Beam", COLOR_BLUE);
            break;
        }
        if (is_solid(den_color, {0.580866, 0.378132, 0.0410021})){
            results.type = DenMonReadResults::PURPLE_BEAM;
            m_logger.log("Den Type: Purple Beam", COLOR_BLUE);
            break;
        }

        ImageStats lair_pink = image_stats(extract_box_reference(screen, m_lair_pink));
//        cout << lair_pink.average << lair_pink.stddev << endl;

        if (is_solid(lair_pink, {0.448155, 0.177504, 0.374341})){
            results.type = DenMonReadResults::MAX_LAIR;
            m_logger.log("Den Type: Max Lair", COLOR_BLUE);
            break;
        }

        return results;
    }while (false);


    ImageViewRGB32 cropped = extract_box_reference(screen, m_sprite);
//    cropped.save("processed.png");
//    cropped = ImageMatch::black_filter_to_alpha(cropped);

    // Find the bounding box of the silhouette.
    const ImagePixelBox box = ImageMatch::enclosing_rectangle_with_pixel_filter(
        cropped,
        // The filter is a lambda function that returns true on silhouette pixels.
        // We treat dark pixels as silhouette pixels.
        [](Color pixel){
            return (uint32_t)pixel.red() + pixel.green() + pixel.blue() < 100;
        }
    );
    ImageRGB32 processed = extract_box_reference(cropped, box).copy();
    // Set black pixels to have 255 alpha while other pixels 0 alpha.
    ImageMatch::set_alpha_black(processed);

    results.slugs = m_matcher.match(processed, ALPHA_SPREAD);
    results.slugs.log(m_logger, MAX_ALPHA);

    results.slugs.clear_beyond_alpha(MAX_ALPHA);

    return results;
}



DenMonSelectData::DenMonSelectData(){
    m_database.add_entry(StringSelectEntry("", "(none)"));
    for (const auto& item : ALL_POKEMON_SPRITES()){
        m_database.add_entry(StringSelectEntry(item.first, item.first, item.second.icon));
    }
}
DenMonSelectOption::DenMonSelectOption(std::string label)
    : StringSelectOption(
        std::move(label),
        DenMonSelectData::m_database,
        LockMode::LOCK_WHILE_RUNNING,
        ""
    )
{}


}
}
}
