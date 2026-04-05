/*  Type Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "PokemonHome_TypeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

using namespace Kernels;
using namespace Kernels::Waterfill;
using namespace Pokemon;



class TypeSprite{
public:
    TypeSprite(const std::string& slug)
        : m_slug(slug)
    {
        ImageRGB32 sprite(RESOURCE_PATH() + "PokemonHome/Types/" + slug + ".png");

        //  Clear non-255 alpha pixels.
        size_t words = sprite.bytes_per_row() / sizeof(uint32_t);
        uint32_t* ptr = sprite.data();
        for (size_t r = 0; r < sprite.height(); r++){
            for (size_t c = 0; c < sprite.width(); c++){
                uint32_t pixel = ptr[c];
                if ((pixel >> 24) != 0xff){
                    ptr[c] = 0;
                }
            }
            ptr += words;
        }

        PackedBinaryMatrix matrix = compress_rgb32_to_binary_min(sprite, 224, 224, 224);
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 10);

        WaterfillObject object;
        for (const WaterfillObject& item : objects){
            object.merge_assume_no_overlap(item);
        }

        m_aspect_ratio = object.aspect_ratio();
        m_matcher = std::make_unique<ImageMatch::WeightedExactImageMatcher>(
            sprite.sub_image(object.min_x, object.min_y, object.width(), object.height()).copy(),
            ImageMatch::WeightedExactImageMatcher::InverseStddevWeight{1, 64}
        );
    }

    const std::string& slug() const{ return m_slug; }
    double aspect_ratio() const{ return m_aspect_ratio; }
    const ImageMatch::WeightedExactImageMatcher& matcher() const{ return *m_matcher; }

private:
    std::string m_slug;
    double m_aspect_ratio;
    std::unique_ptr<ImageMatch::WeightedExactImageMatcher> m_matcher;
};


struct TypeSpriteDatabase{
    std::map<PokemonType, TypeSprite> m_type_map;

    static TypeSpriteDatabase& instance(){
        static TypeSpriteDatabase data;
        return data;
    }

    TypeSpriteDatabase(){
        for (const auto& item : POKEMON_TYPE_SLUGS()){
            if (item.first == PokemonType::NONE){
                continue;
            }
            m_type_map.emplace(item.first, TypeSprite(item.second));
        }
    }
};



static size_t box_distance_sqr(const ImagePixelBox& a, const ImagePixelBox& b){
    size_t dist_x = a.distance_x(b);
    size_t dist_y = a.distance_y(b);
    return dist_x*dist_x + dist_y*dist_y;
}


static std::pair<double, PokemonType> match_type_blob(const ImageViewRGB32& image){
    size_t width = image.width();
    size_t height = image.height();
    if (width * height < 100){
        return {1.0, PokemonType::NONE};
    }
    if (width > 2 * height || height > 2 * width){
        return {1.0, PokemonType::NONE};
    }

    ImageStats stats = image_stats(image);
    if (stats.stddev.sum() < 50){
        return {1.0, PokemonType::NONE};
    }

    double aspect_ratio = (double)width / height;
    double best_score = 0.45;
    PokemonType best_type = PokemonType::NONE;

    for (const auto& item : TypeSpriteDatabase::instance().m_type_map){
        double expected_ratio = item.second.aspect_ratio();
        double ratio = aspect_ratio / expected_ratio;
        if (std::abs(ratio - 1) > 0.2){
            continue;
        }
        double score = item.second.matcher().diff(image);
        if (score < best_score){
            best_score = score;
            best_type = item.first;
        }
    }

    return {best_score, best_type};
}


static void find_type_candidates(
    std::multimap<double, std::pair<PokemonType, ImagePixelBox>>& candidates,
    const ImageViewRGB32& image,
    PackedBinaryMatrix& matrix,
    double max_area_ratio
){
    size_t max_area = (size_t)(image.width() * image.height() * max_area_ratio);
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);

    std::map<size_t, WaterfillObject> objmap;
    for (size_t c = 0; c < objects.size(); c++){
        if (objects[c].area > max_area){
            continue;
        }
        objmap[c] = objects[c];
    }

    //  Merge nearby objects.
    bool changed;
    do{
        changed = false;
        for (auto iter0 = objmap.begin(); iter0 != objmap.end(); ++iter0){
            for (auto iter1 = objmap.begin(); iter1 != objmap.end();){
                if (iter0->first >= iter1->first){
                    ++iter1;
                    continue;
                }
                if (box_distance_sqr(ImagePixelBox(iter0->second), ImagePixelBox(iter1->second)) < 5*5){
                    iter0->second.merge_assume_no_overlap(iter1->second);
                    iter1 = objmap.erase(iter1);
                    changed = true;
                }else{
                    ++iter1;
                }
            }
        }
    }while (changed);

    for (const auto& item : objmap){
        ImageViewRGB32 img = extract_box_reference(image, item.second);
        std::pair<double, PokemonType> result = match_type_blob(img);
        if (result.second != PokemonType::NONE){
            candidates.emplace(
                result.first,
                std::pair<PokemonType, ImagePixelBox>(result.second, ImagePixelBox(item.second))
            );
        }
    }
}



TypeReader::TypeReader(VideoStream& stream, const ImageFloatBox& box)
    : m_box(stream.overlay(), box)
{}

std::pair<PokemonType, PokemonType> TypeReader::read_types(const ImageViewRGB32& screen) const{
    ImageViewRGB32 image = extract_box_reference(screen, m_box);

    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> candidates;
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
            find_type_candidates(candidates, image, matrix, 0.20);
        }
    }

    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> filtered;
    for (const auto& candidate : candidates){
        bool is_dupe = false;
        for (const auto& item : filtered){
            if (box_distance_sqr(candidate.second.second, item.second.second) == 0){
                is_dupe = true;
                break;
            }
        }
        if (!is_dupe){
            filtered.emplace(candidate);
        }
    }

    //  Re-sort by screen position (left-to-right, top-to-bottom) so that type1/type2
    //  assignment is stable regardless of which type was detected with higher confidence.
    std::vector<std::pair<PokemonType, ImagePixelBox>> sorted;
    sorted.reserve(filtered.size());
    for (const auto& item : filtered){
        sorted.emplace_back(item.second);
    }
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b){
        size_t height = a.second.max_y - a.second.min_y;
        size_t diff_y = a.second.min_y > b.second.min_y
            ? a.second.min_y - b.second.min_y
            : b.second.min_y - a.second.min_y;
        if (diff_y > height){
            return a.second.min_y < b.second.min_y;
        }
        return a.second.min_x < b.second.min_x;
    });

    std::pair<PokemonType, PokemonType> result{PokemonType::NONE, PokemonType::NONE};
    for (const auto& item : sorted){
        PokemonType type = item.first;
        if (result.first == PokemonType::NONE){
            result.first = type;
        }else if (type != result.first){
            result.second = type;
            break;
        }
    }
    return result;
}



}
}
}
