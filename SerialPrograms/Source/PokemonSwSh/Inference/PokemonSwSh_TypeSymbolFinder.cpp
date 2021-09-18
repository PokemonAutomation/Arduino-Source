/*  Type Symbol Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/CommonFilters.h"
#include "PokemonSwSh_TypeSymbolFinder.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



size_t distance_sqr(const ImagePixelBox& a, const ImagePixelBox& b){
    bool overlap_x = a.min_x <= b.max_x && b.min_x <= a.max_x;
    bool overlap_y = a.min_y <= b.max_y && b.min_y <= a.max_y;
    if (overlap_x && overlap_y){
        return 0;
    }

    pxint_t dist_x = 0;
    if (!overlap_x){
        dist_x = a.max_x < b.min_x
            ? b.min_x - a.max_x
            : a.min_x - b.max_x;
    }

    pxint_t dist_y = 0;
    if (!overlap_y){
        dist_y = a.max_y < b.min_y
            ? b.min_y - a.max_y
            : a.min_y - b.max_y;
    }

    return (size_t)dist_x*dist_x + (size_t)dist_y*dist_y;
}


std::pair<double, PokemonType> match_type_symbol(const QImage& image){
    int width = image.width();
    int height = image.height();
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
        return {1.0, PokemonType::NONE};
    }

//    image.save("test-" + QString::number(id) + ".png");

//    std::map<double, PokemonType> rank;
    double best_score = 0.20;
    PokemonType best_type = PokemonType::NONE;
    for (const auto& item : all_type_sprites()){
        double rmsd_ratio = item.second.matcher().rmsd_ratio_with(image);
//        cout << item.second.slug() << ": " << rmsd_ratio << endl;

#if 1
        //  Handicap fairy due to white and pink being too similar in color and
        //  false positiving on the background.
        if (item.first == PokemonType::FAIRY){
            rmsd_ratio *= 1.5;
        }
#endif

        if (best_score > rmsd_ratio){
            best_score = rmsd_ratio;
            best_type = item.first;
//            cout << item.second.slug() << ": " << stats.stddev << endl;
        }
    }

//    if (best_type != PokemonType::NONE){
//        cout << get_type_slug(best_type) << ": " << best_score << endl;
//    }
    return {best_score, best_type};
}



void find_symbol_candidates(
    std::multimap<double, std::pair<PokemonType, ImagePixelBox>>& candidates,
    const QImage& image, int min_rgb_brightness, double max_area_ratio
){
    size_t max_area = (size_t)(image.width() * image.height() * max_area_ratio);

    CellMatrix matrix(image);
    BrightFilter filter(min_rgb_brightness);
    matrix.apply_filter(image, filter);

    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, false, 20);

    std::map<CellMatrix::ObjectID, FillGeometry> objmap;
    for (const FillGeometry& item : objects){
        if (item.box.area() > max_area){
            continue;
        }
        objmap[item.id] = item;
    }

    //  Merge nearby objects.
    bool changed;
    do{
        changed = false;
        for (auto& item0 : objmap){
            for (auto& item1 : objmap){
                if (item0.first >= item1.first){
                    continue;
                }
                size_t distance = distance_sqr(item0.second.box, item1.second.box);
                if (distance < 5*5){
                    item0.second.merge_assume_no_overlap(item1.second);
                    objmap.erase(item1.second.id);
                    changed = true;
                }
            }
        }
    }while (changed);

    //  Identify objects.
    for (const auto& item : objmap){
        QImage img = image.copy(
            item.second.box.min_x, item.second.box.min_y,
            item.second.box.width(), item.second.box.height()
        );
        std::pair<double, PokemonType> result = match_type_symbol(img);
        if (result.second != PokemonType::NONE){
            candidates.emplace(
                result.first,
                std::pair<PokemonType, ImagePixelBox>(result.second, item.second.box)
            );
        }
    }
}



std::multimap<double, std::pair<PokemonType, ImagePixelBox>> find_symbols(
    const QImage& image, double max_area_ratio
){
    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> candidates;

    find_symbol_candidates(candidates, image, 450, max_area_ratio);
    find_symbol_candidates(candidates, image, 500, max_area_ratio);
    find_symbol_candidates(candidates, image, 550, max_area_ratio);
    find_symbol_candidates(candidates, image, 600, max_area_ratio);
    find_symbol_candidates(candidates, image, 650, max_area_ratio);
    find_symbol_candidates(candidates, image, 700, max_area_ratio);

    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> filtered;
    for (const auto& candidate : candidates){
//        cout << get_type_slug(candidate.second.first) << ": " << candidate.first << endl;
//        hits.emplace_back(overlay, translate_to_parent(screen, box, candidate.second.second.box), Qt::green);

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
    for (const auto& item : filtered){
//        cout << get_type_slug(item.second.first) << ": " << item.first << " - [" << item.second.second.center_x() << "," << item.second.second.center_y() << "]" << endl;
        const ImagePixelBox& box = item.second.second;
        QImage img = image.copy(
            box.min_x, box.min_y,
            box.width(), box.height()
        );
//        img.save("test-" + QString::number(c++) + ".png");
    }
#endif

    return filtered;
}







}
}
}
