/*  Type Symbol Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CancellableScope.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonSwSh/Resources/PokemonSwSh_TypeSprites.h"
#include "PokemonSwSh_TypeSymbolFinder.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;



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

    return dist_x*dist_x + dist_y*dist_y;
}

//bool print = false;

std::pair<double, PokemonType> match_type_symbol(const ImageViewRGB32& image){
    size_t width = image.width();
    size_t height = image.height();
    if (width * height < 100){
        return {1.0, PokemonType::NONE};
    }
    if (width > 2 * height){
        return {1.0, PokemonType::NONE};
    }
    if (height > 2 * width){
        return {1.0, PokemonType::NONE};
    }
    ImageStats stats = image_stats(image);
    if (stats.stddev.sum() < 50){
//        if (print){
//            cout << "stats.stddev.sum() = " << stats.stddev.sum() << endl;
//        }
        return {1.0, PokemonType::NONE};
    }

    double aspect_ratio = (double)width / height;

//    static int c = 0;
//    image.save("test-" + std::to_string(threshold) + "-" + std::to_string(c++) + ".png");

//    std::map<double, PokemonType> rank;
    double best_score = 0.45;
    PokemonType best_type = PokemonType::NONE;
    for (const auto& item : all_type_sprites()){
//        if (threshold != 700 || id != 55){
//            continue;
//        }

        double expected_aspect_ratio = item.second.aspect_ratio();
        double ratio = aspect_ratio / expected_aspect_ratio;
#if 0
        if (print){
            cout << item.second.slug()
                 << " : expected = " << expected_aspect_ratio
                 << ", actual = " << aspect_ratio
                 << ", ratio = " << ratio << endl;
        }
#endif
        if (std::abs(ratio - 1) > 0.2){
            continue;
        }

        double rmsd_alpha = item.second.matcher().diff(image);

//        item.second.matcher().m_image.save("sprite.png");
//        if (print){
//            cout << item.second.slug() << ": " << rmsd_alpha << endl;
//        }

#if 0
        //  Handicap fairy due to white and pink being too similar in color and
        //  false positiving on the background.
        if (item.first == PokemonType::FAIRY){
            rmsd_ratio *= 1.5;
        }

        //  Bonus for dark because or large contrast.
        if (item.first == PokemonType::DARK){
            rmsd_ratio *= 0.8;
        }
#endif

        if (best_score > rmsd_alpha){
            best_score = rmsd_alpha;
            best_type = item.first;
//            cout << item.second.slug() << ": " << stats.stddev << endl;
        }
    }

//    if (best_type != PokemonType::NONE){
//        cout << get_type_slug(best_type) << ": " << best_score << endl;
//    }
    return {best_score, best_type};
}

void find_type_symbol_candidates(
    std::multimap<double, std::pair<PokemonType, ImagePixelBox>>& candidates,
    const ImageViewPlanar32& original_screen,
    const ImageViewRGB32& image,
    PackedBinaryMatrix& matrix, double max_area_ratio
){
    size_t max_area = (size_t)(image.width() * image.height() * max_area_ratio);
    std::vector<WaterfillObject> objects = find_objects_inplace(
        matrix,
        (size_t)(20. * original_screen.total_pixels() / (1920*1080))
    );

//    static int index = 0;

    std::map<size_t, WaterfillObject> objmap;
    for (size_t c = 0; c < objects.size(); c++){
        if (objects[c].area > max_area){
            continue;
        }
        objmap[c] = objects[c];

#if 0
        extract_box_reference(image, ImagePixelBox(objects[c])).save("test-" + std::to_string(index++) + ".png");
#endif
    }

//    cout << "begin = " << objmap.size() << endl;

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
                const WaterfillObject& obj0 = iter0->second;
                const WaterfillObject& obj1 = iter1->second;
                size_t distance = distance_sqr(
                    ImagePixelBox(obj0.min_x, obj0.min_y, obj0.max_x, obj0.max_y),
                    ImagePixelBox(obj1.min_x, obj1.min_y, obj1.max_x, obj1.max_y)
                );
                if (distance < 5*5){
                    iter0->second.merge_assume_no_overlap(iter1->second);
                    iter1 = objmap.erase(iter1);
                    changed = true;
                }else{
                    ++iter1;
                }
            }
        }
    }while (changed);

//    cout << "merged = " << objmap.size() << endl;

    //  Identify objects.
    for (const auto& item : objmap){
        ImageViewRGB32 img = extract_box_reference(image, item.second);

//        print = index == 137;
//        img.save("test-" + std::to_string(index++) + ".png");

        std::pair<double, PokemonType> result = match_type_symbol(img);
//        cout << "result = " << POKEMON_TYPE_SLUGS().get_string(result.second) << ": " << result.first << endl;
        if (result.second != PokemonType::NONE){
            const WaterfillObject& obj = item.second;
            candidates.emplace(
                result.first,
                std::pair<PokemonType, ImagePixelBox>(
                    result.second,
                    ImagePixelBox(obj.min_x, obj.min_y, obj.max_x, obj.max_y)
                )
            );
        }
    }

//    cout << "candidates = " << candidates.size() << endl;
}



std::multimap<double, std::pair<PokemonType, ImagePixelBox>> find_type_symbols(
    const ImageViewPlanar32& original_screen,
    const ImageViewRGB32& image, double max_area_ratio
){
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
            find_type_symbol_candidates(candidates, original_screen,image, matrix, max_area_ratio);
        }
    }

//    cout << "-------------" << endl;

    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> filtered;
    for (const auto& candidate : candidates){
//        cout << POKEMON_TYPE_SLUGS().get_string(candidate.second.first) << ": " << candidate.first << endl;
//        hits.emplace_back(overlay, translate_to_parent(screen, box, candidate.second.second.box), COLOR_GREEN);

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

#if 0
    static int c = 0;
    for (const auto& item : filtered){
//        cout << get_type_slug(item.second.first) << ": " << item.first << " - [" << item.second.second.center_x() << "," << item.second.second.center_y() << "]" << endl;
        const ImagePixelBox& box = item.second.second;
        ImageViewRGB32 img = image.sub_image(
            box.min_x, box.min_y,
            box.width(), box.height()
        );
        img.save("test-" + std::to_string(c++) + ".png");
    }
#endif

    return filtered;
}





void test_find_type_symbols(
    CancellableScope& scope,
    VideoOverlay& overlay,
    const ImageFloatBox& box,
    const ImageViewRGB32& screen, double max_area_ratio
){
    ImageViewRGB32 image = extract_box_reference(screen, box);

    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> candidates = find_type_symbols(screen, image, max_area_ratio);


    std::deque<OverlayBoxScope> hits;
//    hits.clear();
    cout << "---------------" << endl;
    for (const auto& item : candidates){
        cout << POKEMON_TYPE_SLUGS().get_string(item.second.first) << ": " << item.first << endl;
        hits.emplace_back(overlay, translate_to_parent(screen, box, item.second.second), COLOR_GREEN);
    }

    scope.wait_for(std::chrono::seconds(10));
}





}
}
}
