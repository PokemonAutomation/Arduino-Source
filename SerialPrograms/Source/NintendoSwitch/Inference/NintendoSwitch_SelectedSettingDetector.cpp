/*  Detect Home
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch_SelectedSettingDetector.h"

//
// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

SelectedSettingWatcher::~SelectedSettingWatcher() = default;

SelectedSettingWatcher::SelectedSettingWatcher(
    ImageFloatBox selected_box,
    ImageFloatBox not_selected_box1,
    ImageFloatBox not_selected_box2,
    ImageFloatBox representative_background
)
    : VisualInferenceCallback("SelectedSettingWatcher")
    , m_selected_box(selected_box)
    , m_not_selected_box1(not_selected_box1)
    , m_not_selected_box2(not_selected_box2)
    , m_representative_background(representative_background)
{}

void SelectedSettingWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_selected_box);
    items.add(COLOR_BLUE, m_not_selected_box1);
    items.add(COLOR_BLUE, m_not_selected_box2);
    items.add(COLOR_GREEN, m_representative_background);
}

bool SelectedSettingWatcher::is_white_theme(const ImageViewRGB32& screen){
    ImageStats stats_window_background = image_stats(extract_box_reference(screen, m_representative_background));
    bool white_theme = stats_window_background.average.sum() > 500;
    return white_theme;
}

bool SelectedSettingWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){

    ImageStats stats_unselected_box1 = image_stats(extract_box_reference(screen, m_not_selected_box1));
    double unselected1_average_sum = stats_unselected_box1.average.sum();
//    cout << "unselected_average_sum1: " << std::to_string(unselected1_average_sum) << endl;

    ImageStats stats_unselected_box2 = image_stats(extract_box_reference(screen, m_not_selected_box2));
    double unselected2_average_sum = stats_unselected_box2.average.sum();
//    cout << "unselected_average_sum2: " << std::to_string(unselected2_average_sum) << endl;

    double average_sum_unselected_diff = std::abs(unselected1_average_sum - unselected2_average_sum);

    ImageStats stats_selected_box = image_stats(extract_box_reference(screen, m_selected_box));
    double selected_average_sum = stats_selected_box.average.sum();
//    cout << "selected_average_sum: " << std::to_string(selected_average_sum) << endl;

    bool is_selected = false;
    if (is_white_theme(screen)){  // light mode
        // unselected should be brighter than selected
        is_selected = selected_average_sum < std::min(unselected1_average_sum, unselected2_average_sum) - average_sum_unselected_diff - 20 ;
    }else{  // dark mode
        // selected should be brighter than unselected
        is_selected = selected_average_sum  > std::max(unselected1_average_sum, unselected2_average_sum) + average_sum_unselected_diff + 20;
    }

    return is_selected;
}






}
}
