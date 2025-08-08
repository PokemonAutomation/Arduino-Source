/*  Waterfill Process
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Color.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "WaterfillUtilities.h"

#include <iostream>
//using std::cout;
//using std::endl;


namespace PokemonAutomation{


std::pair<PackedBinaryMatrix, size_t> remove_center_pixels(
    const Kernels::Waterfill::WaterfillObject& object,
    size_t num_pixels_to_remove
){
    PackedBinaryMatrix matrix = object.packed_matrix();
    size_t width = matrix.width();
    size_t height = matrix.height();
//    cout << matrix.dump() << endl;

    //  Sort all pixels by distance from center.
    size_t center_x = (size_t)(object.center_of_gravity_x() - object.min_x);
    size_t center_y = (size_t)(object.center_of_gravity_y() - object.min_y);
    std::map<uint64_t, size_t> distances;
    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            if (matrix.get(c, r)){
                size_t dist_x = c - center_x;
                size_t dist_y = r - center_y;
                uint64_t distance_sqr = (uint64_t)dist_x*dist_x + (uint64_t)dist_y*dist_y;
                distances[distance_sqr]++;
            }
        }
    }

    //  Filter out pixels close to center
    size_t count = 0;
    uint64_t distance_sqr_th = 0;
    for (auto& item : distances){
        count += item.second;
        if (count >= num_pixels_to_remove){
            distance_sqr_th = item.first;
            break;
        }
    }
    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            size_t dist_x = c - center_x;
            size_t dist_y = r - center_y;
            uint64_t distance_sqr = (uint64_t)dist_x*dist_x + (uint64_t)dist_y*dist_y;
            if (distance_sqr < distance_sqr_th){
                matrix.set(c, r, false);
            }
        }
    }

    return std::pair<PackedBinaryMatrix, size_t>(std::move(matrix), distance_sqr_th);
}

bool match_template_by_waterfill(
    const ImageViewRGB32 &image,
    const ImageMatch::WaterfillTemplateMatcher &matcher,
    const std::vector<std::pair<uint32_t, uint32_t>> &filters,
    const std::pair<size_t, size_t> &area_thresholds,
    double rmsd_threshold,
    std::function<bool(Kernels::Waterfill::WaterfillObject& object)> check_matched_object)
{
    if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
        dump_debug_image(
            global_logger_command_line(), 
            "CommonFramework/WaterfillTemplateMatcher", 
            "match_template_by_waterfill_input_image", 
            image
        );
        std::cout << "Match template by waterfill, " << filters.size() << " filter(s), size range ("
                  << area_thresholds.first << ", ";
        if (area_thresholds.second == SIZE_MAX){
            std::cout << "SIZE_MAX";
        }else{
            std::cout << area_thresholds.second;
        }
        std::cout << ")" << std::endl;
    }
    std::vector<PokemonAutomation::PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(image, filters);
//    cout << matrices.size() << endl;

    bool detected = false;
    bool stop_match = false;
    for (size_t i_matrix = 0; i_matrix < matrices.size(); i_matrix++){
        PackedBinaryMatrix& matrix = matrices[i_matrix];
        if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            ImageRGB32 binaryImage = image.copy();
            filter_by_mask(matrix, binaryImage, Color(COLOR_BLACK), true);
            //filter_by_mask(matrix, binaryImage, Color(COLOR_WHITE), true);
            dump_debug_image(
                global_logger_command_line(), 
                "CommonFramework/WaterfillTemplateMatcher", 
                "match_template_by_waterfill_filtered_image_" + std::to_string(i_matrix), 
                binaryImage
            );
        }

        std::unique_ptr<Kernels::Waterfill::WaterfillSession> session = Kernels::Waterfill::make_WaterfillSession();
        Kernels::Waterfill::WaterfillObject object;
        const size_t min_area = area_thresholds.first;
//        cout << "min_area = " << min_area << endl;
        session->set_source(matrix);
        auto finder = session->make_iterator(min_area);
        const bool keep_object_matrix = false;
        while (finder->find_next(object, keep_object_matrix)){
//            cout << "object.area = " << object.area << endl;
            if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
                std::cout << "------------" << std::endl;
                std::cout << "Object area: " << object.area << std::endl;
            }

            if (object.area > area_thresholds.second){
                continue;
            }
            double rmsd = matcher.rmsd_original(image, object);
            if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
                std::cout << "Object rmsd: " << rmsd << std::endl;
            }

            if (rmsd < rmsd_threshold){
                detected = true;
                
                if (check_matched_object(object)){
                    stop_match = true;
                    break;
                }
            }
        }

        if (stop_match){
            break;
        }
    }
    if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
        std::cout << "End match template by waterfill" << std::endl;
    }
    return detected;
}


void draw_matrix_on_image(
    const PackedBinaryMatrix& matrix,
    uint32_t color, ImageRGB32& image, size_t offset_x, size_t offset_y
){
    for (size_t x = 0; x < matrix.width(); x++){
        for (size_t y = 0; y < matrix.height(); y++){
            if (matrix.get(x, y)){
                image.pixel(offset_x + x, offset_y + y) = (uint32_t)color;
            }
        }
    }
}


void draw_object_on_image(
    const Kernels::Waterfill::WaterfillObject& obj,
    const uint32_t& color, ImageRGB32& image, size_t offset_x, size_t offset_y
){
    for (size_t x = 0; x < obj.width(); x++){
        for (size_t y = 0; y < obj.height(); y++){
            if (obj.object->get(obj.min_x + x, obj.min_y + y)){
                image.pixel(offset_x + obj.min_x + x, offset_y + obj.min_y + y) = color;
                // cout << "Set color at " << offset_x + object.min_x + obj.min_x + x << ", " << offset_y + object.min_y + obj.min_y + y << endl;
            }
        }
    }
}




}
