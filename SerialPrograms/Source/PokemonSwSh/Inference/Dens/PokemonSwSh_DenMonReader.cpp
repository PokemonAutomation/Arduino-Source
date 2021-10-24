/*  Den Sprite Identifier
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageMatch/FilterToAlpha.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh_DenMonReader.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{





class DenSpriteMatcher : public ImageMatch::SilhouetteDictionaryMatcher{
public:
    DenSpriteMatcher() = default;
    virtual QRgb crop_image(QImage& image) const override{
        QRect rect = ImageMatch::enclosing_rectangle_with_pixel_filter(
            image,
            [](QRgb pixel){
                return qRed(pixel) + qGreen(pixel) + qBlue(pixel) < 100;
            }
        );
        image = image.copy(rect);
        return 0;
    }
    virtual void set_alpha_channels(QImage& image) const override{
        ImageMatch::set_alpha_black(image);
    }
};


DenSpriteMatcher make_DEN_SPRITE_MATCHER(){
    DenSpriteMatcher matcher;
    for (const auto& item : all_pokemon_sprites()){
        matcher.add(item.first, item.second.silhouette());
    }
    return matcher;
}
const DenSpriteMatcher& DEN_SPRITE_MATCHER(){
    static DenSpriteMatcher matcher = make_DEN_SPRITE_MATCHER();
    return matcher;
}




DenMonReader::DenMonReader(Logger& logger, VideoOverlay& overlay)
    : m_matcher(DEN_SPRITE_MATCHER())
    , m_logger(logger)
    , m_white(overlay, 0.800, 0.200, 0.150, 0.100)
    , m_den_color(overlay, 0.400, 0.050, 0.200, 0.100)
    , m_lair_pink(overlay, 0.575, 0.035, 0.050, 0.100)
    , m_sprite(overlay, 0.098, 0.23, 0.285, 0.41)
{}


DenMonReadResults DenMonReader::read(const QImage& screen) const{
    DenMonReadResults results;
    if (screen.isNull()){
        return results;
    }

    ImageStats white = image_stats(extract_box(screen, m_white));
    if (!is_solid(white, {0.303079, 0.356564, 0.340357})){
        return results;
    }
    do{
        ImageStats den_color = image_stats(extract_box(screen, m_den_color));

        if (is_solid(den_color, {0.593023, 0.204651, 0.202326})){
            results.type = DenMonReadResults::RED_BEAM;
            m_logger.log("Den Type: Red Beam", Qt::blue);
            break;
        }
        if (is_solid(den_color, {0.580866, 0.378132, 0.0410021})){
            results.type = DenMonReadResults::PURPLE_BEAM;
            m_logger.log("Den Type: Purple Beam", Qt::blue);
            break;
        }

        ImageStats lair_pink = image_stats(extract_box(screen, m_lair_pink));
//        cout << lair_pink.average << lair_pink.stddev << endl;

        if (is_solid(lair_pink, {0.448155, 0.177504, 0.374341})){
            results.type = DenMonReadResults::MAX_LAIR;
            m_logger.log("Den Type: Max Lair", Qt::blue);
            break;
        }

        return results;
    }while (false);


    QImage processed = extract_box(screen, m_sprite);
//    processed.save("processed.png");
//    processed = ImageMatch::black_filter_to_alpha(processed);
    results.slugs = m_matcher.match(std::move(processed), ALPHA_SPREAD);
    results.slugs.log(m_logger, MAX_ALPHA);

    results.slugs.clear_beyond_alpha(MAX_ALPHA);

    return results;
}



DenMonSelectData::DenMonSelectData(){
    m_list.emplace_back("(none)", QIcon());
    for (const auto& item : all_pokemon_sprites()){
        m_list.emplace_back(
            QString::fromStdString(item.first),
            item.second.icon()
        );
    }
}
DenMonSelectOption::DenMonSelectOption(QString label)
    : StringSelectOption(std::move(label), cases(), "")
{}
std::string DenMonSelectOption::slug() const{
    size_t index = *this;
    if (index == 0){
        return "";
    }
    return ((const QString&)*this).toStdString();
}


}
}
}
