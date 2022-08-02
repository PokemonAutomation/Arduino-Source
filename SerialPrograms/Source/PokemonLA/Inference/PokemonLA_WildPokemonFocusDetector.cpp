/*  Wild Pokemon Focus Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLA_WildPokemonFocusDetector.h"

#include <array>
#include <sstream>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

size_t count_horizontal_translucent_border_pixels(const ImageViewRGB32& image, const Color& threshold, bool dark_top);

size_t count_vertical_translucent_border_pixels(const ImageViewRGB32& image, const Color& threshold, bool dark_left){
    if (image.height() == 0 || image.width() == 0){
        return 0;
    }

    size_t num_border_pixels = 0;
    int16_t thres_c[3] = {threshold.r(), threshold.g(), threshold.b()};

    size_t num_cols = image.width() - 1;
    // Go through each row in the image
    for(size_t y = 0; y < image.height(); y++){
        // Record the border gradient of this row:
        bool is_black[3] = {false};
        int16_t gradients[3] = {0};
        for(size_t col_index = 0; col_index < num_cols; col_index++){
            uint32_t p_left = image.pixel(col_index, y);
            uint32_t p_right = image.pixel(col_index+1, y);

            // c1 may be darker, while c2 is not yet covered by the dark translucent region
            Color c1(dark_left ? p_left : p_right);
            Color c2(dark_left ? p_right : p_left);

            if (c1.r() == 0 && c2.r() > 0){
                is_black[0] = true;
            }
            if (c1.g() == 0 && c2.g() > 0){
                is_black[1] = true;
            }
            if (c1.b() == 0 && c2.b() > 0){
                is_black[2] = true;
            }

            gradients[0] = std::max(gradients[0], int16_t((int)c2.r() - (int) c1.r()));
            gradients[1] = std::max(gradients[1], int16_t((int)c2.g() - (int) c1.g()));
            gradients[2] = std::max(gradients[2], int16_t((int)c2.b() - (int) c1.b()));
        }

        bool has_non_border_channel = false;
        for(int c = 0; c < 3; c++){
            if (is_black[c] == false && gradients[c] < thres_c[c]){
                has_non_border_channel = true;
                break;
            }
        }
        num_border_pixels += !has_non_border_channel;

        // cout << y << ": (" << gradients[0] << (is_black[0] ? "*" : "");
        // cout << ", " << gradients[1] << (is_black[1] ? "*" : "");
        // cout << ", " << gradients[2] << (is_black[2] ? "*" : "") << ")\n";
        // cout << "has_non_border_channel " << has_non_border_channel << endl;
    }
    
    cout << "num border pixels proportion: " << num_border_pixels / (float)image.height() << endl;

    return num_border_pixels;
}

WildPokemonFocusDetector::WildPokemonFocusDetector(LoggerQt& logger, VideoOverlay& overlay)
    : VisualInferenceCallback("WildPokemonFocusDetector")
    , m_pokemon_tab_upper_bound(0.109, 0.853, 0.24, 0.012)
    , m_pokemon_tab_lower_bound(0.109, 0.952, 0.24, 0.012)
    , m_pokemon_tab_left_bound(0.100, 0.875, 0.007, 0.045)
    , m_pokemon_tab_right_bound(0.3495, 0.873, 0.007, 0.073)
{}

void WildPokemonFocusDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_BLUE, m_pokemon_tab_upper_bound);
    items.add(COLOR_BLUE, m_pokemon_tab_lower_bound);
    items.add(COLOR_BLUE, m_pokemon_tab_left_bound);
    items.add(COLOR_BLUE, m_pokemon_tab_right_bound);
}

//  Return true if the inference session should stop.
bool WildPokemonFocusDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    const Color threshold(20, 20, 20);
    const ImageViewRGB32& upper_border = extract_box_reference(frame, m_pokemon_tab_upper_bound);
    const size_t upper_border_length = count_horizontal_translucent_border_pixels(
        upper_border, threshold, false);
    
    if (upper_border_length / (float)upper_border.width() <= 0.9){
        return false;
    }

    const ImageViewRGB32& lower_border = extract_box_reference(frame, m_pokemon_tab_lower_bound);
    const size_t lower_border_length = count_horizontal_translucent_border_pixels(
        lower_border, threshold, true);
    
    if (lower_border_length / (float)lower_border.width() <= 0.9){
        return false;
    }

    const ImageViewRGB32& left_border = extract_box_reference(frame, m_pokemon_tab_left_bound);
    const size_t left_border_length = count_vertical_translucent_border_pixels(
        left_border, threshold, false);
    if (left_border_length / (float)left_border.height() <= 0.9){
        return false;
    }

    const ImageViewRGB32& right_border = extract_box_reference(frame, m_pokemon_tab_right_bound);
    const size_t right_border_length = count_vertical_translucent_border_pixels(
        right_border, threshold, true);
    if (right_border_length / (float)right_border.height() <= 0.9){
        return false;
    }

    return true;
}


// PokemonDetails read_focused_wild_pokemon_info(
//     LoggerQt& logger, VideoOverlay& overlay,
//     const ImageViewRGB32& frame,
//     Language language
// ){

// }



}
}
}
