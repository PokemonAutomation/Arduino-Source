/*  Den Sprite Identifier
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/ImageMatch/FilterToAlpha.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh_DenMonReader.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DenSpriteMatcher : public ImageMatch::CroppedImageMatcher{
public:
    DenSpriteMatcher()
        : CroppedImageMatcher(false)
    {}
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
    , m_box(overlay, 0.098, 0.23, 0.285, 0.41)
{}


DenMonReadResults DenMonReader::read(const QImage& image, double max_RMSD_ratio) const{
    DenMonReadResults results;
    if (image.isNull()){
        return results;
    }

    QImage processed = extract_box(image, m_box);
//    processed.save("processed.png");
//    processed = ImageMatch::black_filter_to_alpha(processed);
    results.slugs = m_matcher.match(std::move(processed));
    results.slugs.log(m_logger, max_RMSD_ratio);

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
