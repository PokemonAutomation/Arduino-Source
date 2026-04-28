/*  Tera Type Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "PokemonHome_TeraTypeReader.h"

namespace PokemonAutomation {
namespace Pokemon {

using namespace Kernels;
using namespace Kernels::Waterfill;

namespace {

class TypeSprite {
public:
    TypeSprite(const std::string& slug, const std::string& resource_location)
        : m_slug(slug)
    {
        ImageRGB32 sprite(RESOURCE_PATH() + resource_location);

        sprite = filter_rgb32_range(
            sprite,
            0xff000000, 0xffffffff,
            Color(0), false
        );

        PackedBinaryMatrix matrix = compress_rgb32_to_binary_min(sprite, 224, 224, 224);
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 10);

        WaterfillObject object;
        for (const WaterfillObject& item : objects){
            object.merge_assume_no_overlap(item);
        }

        m_aspect_ratio = object.aspect_ratio();
        m_matcher = std::make_unique<ImageMatch::WeightedExactImageMatcher>(
            sprite.sub_image(object.min_x, object.min_y, object.width(), object.height()).copy(),
            ImageMatch::WeightedExactImageMatcher::InverseStddevWeight{ 1, 64 }
        );
    }

    const std::string& slug() const { return m_slug; }
    double aspect_ratio() const { return m_aspect_ratio; }
    const ImageMatch::WeightedExactImageMatcher& matcher() const { return *m_matcher; }

private:
    std::string m_slug;
    double m_aspect_ratio;
    std::unique_ptr<ImageMatch::WeightedExactImageMatcher> m_matcher;
};

struct TeraTypeSpriteDatabase {
    std::map<PokemonTeraType, TypeSprite> m_type_map;

    static TeraTypeSpriteDatabase& instance(){
        static TeraTypeSpriteDatabase data;
        return data;
    }

    TeraTypeSpriteDatabase(){
        for (const auto& item : POKEMON_TERA_TYPE_SLUGS()){
            if (item.first == PokemonTeraType::NONE){
                continue;
            }
            m_type_map.emplace(item.first, TypeSprite(item.second, "PokemonHome/TeraTypes/" + item.second + ".png"));
        }
    }
};

size_t distance_sqr(const ImagePixelBox& a, const ImagePixelBox& b){
    bool overlap_x = a.min_x <= b.max_x && b.min_x <= a.max_x;
    bool overlap_y = a.min_y <= b.max_y && b.min_y <= a.max_y;
    if (overlap_x && overlap_y){
        return 0;
    }

    size_t dist_x = 0;
    if (!overlap_x){
        dist_x = a.max_x < b.min_x
            ? b.min_x - a.max_x
            : a.min_x - b.max_x;
    }

    size_t dist_y = 0;
    if (!overlap_y){
        dist_y = a.max_y < b.min_y
            ? b.min_y - a.max_y
            : a.min_y - b.max_y;
    }

    return dist_x * dist_x + dist_y * dist_y;
}

bool image_validation(const ImageViewRGB32& image){
    size_t width = image.width();
    size_t height = image.height();
    if (width * height < 100){
        return false;
    }
    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }
    ImageStats stats = image_stats(image);
    if (stats.stddev.sum() < 50){
        return false;
    }

    return true;
}

std::pair<double, PokemonTeraType> match_tera_type_symbol(const ImageViewRGB32& image){
    if (!image_validation(image)){
        return { 1.0, PokemonTeraType::NONE };
    }

    size_t width = image.width();
    size_t height = image.height();

    double aspect_ratio = (double)width / height;

    const std::map<PokemonTeraType, TypeSprite>* type_sprite_map = &TeraTypeSpriteDatabase::instance().m_type_map;

    double best_score = 0.45;
    PokemonTeraType best_type = PokemonTeraType::NONE;
    for (const auto& item : *type_sprite_map){
        double expected_aspect_ratio = item.second.aspect_ratio();
        double ratio = aspect_ratio / expected_aspect_ratio;

        if (std::abs(ratio - 1) > 0.2){
            continue;
        }

        double rmsd_alpha = item.second.matcher().diff(image);

        if (best_score > rmsd_alpha){
            best_score = rmsd_alpha;
            best_type = item.first;
        }
    }

    return { best_score, best_type };
}

void find_tera_type_symbol_candidates(
    std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>>& candidates,
    const ImageViewPlanar32& original_screen,
    const ImageViewRGB32& image,
    PackedBinaryMatrix& matrix,
    double max_area_ratio
){
    size_t max_area = (size_t)(image.width() * image.height() * max_area_ratio);
    std::vector<WaterfillObject> objects = find_objects_inplace(
        matrix,
        (size_t)(20. * original_screen.total_pixels() / (1920 * 1080))
    );

    std::map<size_t, WaterfillObject> objmap;
    for (size_t c = 0; c < objects.size(); c++){
        if (objects[c].area > max_area){
            continue;
        }
        objmap[c] = objects[c];
    }

    bool changed;
    do{
        changed = false;
        for (auto iter0 = objmap.begin(); iter0 != objmap.end(); ++iter0){
            for (auto iter1 = objmap.begin(); iter1 != objmap.end();){
                if (iter0->first >= iter1->first){
                    ++iter1;
                    continue;
                }
                const WaterfillObject& obj0 = iter0->second;
                const WaterfillObject& obj1 = iter1->second;
                size_t distance = distance_sqr(
                    ImagePixelBox(obj0.min_x, obj0.min_y, obj0.max_x, obj0.max_y),
                    ImagePixelBox(obj1.min_x, obj1.min_y, obj1.max_x, obj1.max_y)
                );
                if (distance < 5 * 5){
                    iter0->second.merge_assume_no_overlap(iter1->second);
                    iter1 = objmap.erase(iter1);
                    changed = true;
                } else{
                    ++iter1;
                }
            }
        }
    }
    while (changed);

    for (const auto& item : objmap){
        ImageViewRGB32 img = extract_box_reference(image, item.second);

        std::pair<double, PokemonTeraType> result = match_tera_type_symbol(img);
        if (result.second != PokemonTeraType::NONE){
            const WaterfillObject& obj = item.second;
            candidates.emplace(
                result.first,
                std::pair<PokemonTeraType, ImagePixelBox>(
                    result.second,
                    ImagePixelBox(obj.min_x, obj.min_y, obj.max_x, obj.max_y)
                )
            );
        }
    }
}

}

std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>> find_tera_type_symbols(
    const ImageViewPlanar32& original_screen,
    const ImageViewRGB32& image,
    double max_area_ratio
){
    std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>> candidates;

    {
        std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
            image,
            {
                {0xff808060, 0xffffffff},
                {0xffa0a060, 0xffffffff},
                {0xff606060, 0xffffffff},
                {0xff707070, 0xffffffff},
                {0xff808080, 0xffffffff},
                {0xff909090, 0xffffffff},
                {0xffa0a0a0, 0xffffffff},
                {0xffb0b0b0, 0xffffffff},
                {0xffc0c0c0, 0xffffffff},
                {0xffd0d0d0, 0xffffffff},
                {0xffe0e0e0, 0xffffffff},
            }
        );
        for (PackedBinaryMatrix& matrix : matrices){
            find_tera_type_symbol_candidates(candidates, original_screen, image, matrix, max_area_ratio);
        }
    }

    std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>> filtered;
    for (const auto& candidate : candidates){
        bool is_dupe = false;
        for (const auto& item : filtered){
            if (distance_sqr(candidate.second.second, item.second.second) == 0){
                is_dupe = true;
                break;
            }
        }
        if (!is_dupe){
            filtered.emplace(candidate);
        }
    }

    return filtered;
}

PokemonTeraType read_pokemon_tera_type(
    const ImageViewRGB32& original_screen,
    const ImageFloatBox& box
){
    ImageViewRGB32 image = extract_box_reference(original_screen, box);

    std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>> filtered = find_tera_type_symbols(
        original_screen,
        image,
        0.20
    );

    if (filtered.empty()){
        return PokemonTeraType::NONE;
    }

    return filtered.begin()->second.first;
}

}
}
