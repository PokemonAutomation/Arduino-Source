/*  Item Compatibility Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cfloat>
#include <cmath>
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonLA_MapMarkerLocator.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


double get_orientation_on_map(const ImageViewRGB32& screen, bool avoid_lava_area){
    // The map region of the entire screen
    const ImageFloatBox box(0.271, 0.059, 0.458, 0.833);

    const ImageViewRGB32 region = extract_box_reference(screen, box);

    // Find the red pixels that form the red arrow
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_multirange(region,
    {
        {combine_rgb(200, 0, 0), combine_rgb(255, 180, 180)},
        {combine_rgb(180, 0, 0), combine_rgb(255, 160, 160)},
        {combine_rgb(160, 0, 0), combine_rgb(255, 140, 140)},
        {combine_rgb(140, 0, 0), combine_rgb(255, 120, 120)},
        {combine_rgb(120, 0, 0), combine_rgb(255, 100, 100)},
    });

    ImageRGB32 output = region.copy();

    std::unique_ptr<Kernels::Waterfill::WaterfillSession> session = Kernels::Waterfill::make_WaterfillSession();
    session->set_source(matrix);

    const size_t min_area = 100;
    auto finder = session->make_iterator(min_area);
    Kernels::Waterfill::WaterfillObject red_marker_obj;
    bool found_red_marker = false;
    // Since the lava area in coastlands is also red, to avoid confusion, we avoid the lava area:
    const size_t lava_min_x = (size_t)(screen.width() * 0.636);
    const size_t lava_max_x = (size_t)(screen.width() * 0.7 + 0.5);
    const size_t lava_min_y = (size_t)(screen.height() * 0.124);
    const size_t lava_max_y = (size_t)(screen.height() * 0.243 + 0.5);
    while (finder->find_next(red_marker_obj, true)){
        const size_t center_x = (size_t)red_marker_obj.center_of_gravity_x();
        const size_t center_y = (size_t)red_marker_obj.center_of_gravity_y();
        if (center_x >= lava_min_x && center_x <= lava_max_x && center_y >= lava_min_y && center_y <= lava_max_y){
            // Skip lava area
            continue;
        }
        // cout << red_marker_obj.area << endl;
        found_red_marker = true;
        break;
    }

    if (!found_red_marker){
        cout << "Error: cannot find red marker on the map" << endl;
        return FLT_MAX;
    }

    const size_t stop = (size_t)(0.85 * red_marker_obj.area);
    // matrix2 stores pixels that are at the end points of the red marker.
    PackedBinaryMatrix matrix2 = remove_center_pixels(red_marker_obj, stop).first;

    draw_matrix_on_image(matrix2, combine_rgb(0, 0, 255), output, red_marker_obj.min_x, red_marker_obj.min_y);

    // Get those end point regions from matrix2 into marker_ends.
    // The object in marker_ends are in the local coordinate system of the red marker
    const size_t local_marker_center_x = (size_t)red_marker_obj.center_of_gravity_x() - red_marker_obj.min_x;
    const size_t local_marker_center_y = (size_t)red_marker_obj.center_of_gravity_y() - red_marker_obj.min_y;
    session->set_source(matrix2);
    finder = session->make_iterator(1);
    std::vector<Kernels::Waterfill::WaterfillObject> marker_ends;
    for (Kernels::Waterfill::WaterfillObject marker_end; finder->find_next(marker_end, false);){
        marker_ends.emplace_back(std::move(marker_end));
    }

    if (marker_ends.size() != 3){
        cout << "Error: detected red marker has wrong shape. Found wrong red area on map?" << endl;
        return FLT_MAX;
    }

    // The angle of each marker end relative to the marker center
    double end_angles[3] = {0.0, 0.0, 0.0};
    for(int i = 0; i < 3; i++){
        ptrdiff_t x = (ptrdiff_t)marker_ends[i].center_of_gravity_x() - (ptrdiff_t)local_marker_center_x;
        ptrdiff_t y = (ptrdiff_t)marker_ends[i].center_of_gravity_y() - (ptrdiff_t)local_marker_center_y;
        double angle = std::atan2(y, x) * 57.29577951308232;
        if (angle < 0){
            angle += 360;
        }
        end_angles[i] = angle;
    }

    // end_angle_distance[i], the angular distance between end_angles[i] and end_angles[(i+1)%3]
    double end_angle_distances[3] = {0.0, 0.0, 0.0};
    // Find out which two ends form the smallest angular distance between them
    int min_angle_distance_index = 0;
    for(int i = 0; i < 3; i++){
        end_angle_distances[i] = std::fabs(end_angles[i] - end_angles[(i+1)%3]);
        if (end_angle_distances[i] > 180){
            end_angle_distances[i] = 360 - end_angle_distances[i];
        }
        
        if (end_angle_distances[i] < end_angle_distances[min_angle_distance_index]){
            min_angle_distance_index = i;
        }
    }

    // so now the marker end with the index: min_angle_distance_index and (min_angle_distance_index+1)%3 form
    // the smallest anglular distance between two ends. The other one, min_angle_distance_index+2)%3 must be
    // the end where the red marker points to!
    double red_marker_direction = end_angles[(min_angle_distance_index+2)%3];
    cout << "Found red marker direction " << red_marker_direction << endl;

    output.pixel((size_t)red_marker_obj.center_of_gravity_x(), (size_t)red_marker_obj.center_of_gravity_y()) = (uint32_t)Color(255, 0, 0);
    output.save("./test_map_location.png");

    return red_marker_direction;
}



}
}
}
