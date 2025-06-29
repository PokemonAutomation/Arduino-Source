/*  Image Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "ImageGradient.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


size_t count_horizontal_translucent_border_pixels(const ImageViewRGB32& image, const Color& threshold, bool dark_top){
    // Gradient on each color channel at each pixel in one row
    std::vector<int16_t> gradients(image.width() * 3, 0);
    // Whether a color channel is zero for each pixel in one row
    std::vector<bool> is_zero(image.width() * 3, false);

    if (image.height() == 0 || image.width() == 0){
        return 0;
    }

    size_t num_rows = image.height() - 1;
    for(size_t row_index = 0; row_index < num_rows; row_index++){
        for(size_t x = 0; x < image.width(); x++){
            uint32_t p_above = image.pixel(x, row_index);
            uint32_t p_below = image.pixel(x, row_index+1);

            // If both two pixels are black, then we skip this case
            if (p_above == 0 && p_below == 0){
                continue;
            }

            // c1 may be darker, while c2 is not yet covered by the dark translucent region
            Color c1(dark_top ? p_above : p_below);
            Color c2(dark_top ? p_below : p_above);
            if (c1.red() == 0){
                is_zero[3*x] = true;
            }
            if (c1.green() == 0){
                is_zero[3*x+1] = true;
            }
            if (c1.blue() == 0){
                is_zero[3*x+2] = true;
            }
            gradients[3*x] = std::max(gradients[3*x], int16_t((int)c2.red() - (int) c1.red()));
            gradients[3*x+1] = std::max(gradients[3*x+1], int16_t((int)c2.green() - (int) c1.green()));
            gradients[3*x+2] = std::max(gradients[3*x+2], int16_t((int)c2.blue() - (int) c1.blue()));


        }
    }

    int16_t thres_c[3] = {threshold.red(), threshold.green(), threshold.blue()};

    size_t num_border_pixels = 0;
    for(size_t x = 0; x < image.width(); x++){
        bool has_non_border_channel = false;
        for(int c = 0; c < 3; c++){
            if (is_zero[3*x+c] == false && gradients[3*x+c] < thres_c[c]){
                has_non_border_channel = true;
                break;
            }
        }

        num_border_pixels += !has_non_border_channel;
    }
    
    // int16_t min_color[3] = {255, 255, 255}; 
    // for(size_t i = 0; i < image.width(); i++){
    //     if (is_zero[3*i] == false || is_zero[3*i+1] == false || is_zero[3*i+2] == false){
    //         cout << i << ": (" << gradients[3*i] << (is_zero[3*i] ? "*" : "");
    //         cout << ", " << gradients[3*i+1] << (is_zero[3*i+1] ? "*" : "");
    //         cout << ", " << gradients[3*i+2] << (is_zero[3*i+2] ? "*" : "") << ")\n";
    //         for(int j = 0; j < 3; j++){
    //             if (is_zero[3*i+j] == false){
    //                 min_color[j] = std::min(min_color[j], gradients[3*i + j]);
    //             }
    //         }
    //     }
    // }
    // cout << "min non-0 color " << min_color[0] << " " << min_color[1] << " " << min_color[2] << endl;
    // cout << "num border pixels proportion: " << num_border_pixels / (float)image.width() << endl;

    return num_border_pixels;
}


size_t count_vertical_translucent_border_pixels(const ImageViewRGB32& image, const Color& threshold, bool dark_left){
    if (image.height() == 0 || image.width() == 0){
        return 0;
    }

    size_t num_border_pixels = 0;
    int16_t thres_c[3] = {threshold.red(), threshold.green(), threshold.blue()};

    size_t num_cols = image.width() - 1;
    // Go through each row in the image
    for(size_t y = 0; y < image.height(); y++){
        // Record the border gradient of this row:
        bool is_zero[3] = {false};
        int16_t gradients[3] = {0};
        for(size_t col_index = 0; col_index < num_cols; col_index++){
            uint32_t p_left = image.pixel(col_index, y);
            uint32_t p_right = image.pixel(col_index+1, y);

            // If both two pixels are black, then we skip this case
            if (p_left == 0 && p_right == 0){
                continue;
            }

            // c1 may be darker, while c2 is not yet covered by the dark translucent region
            Color c1(dark_left ? p_left : p_right);
            Color c2(dark_left ? p_right : p_left);

            if (c1.red() == 0){
                is_zero[0] = true;
            }
            if (c1.green() == 0){
                is_zero[1] = true;
            }
            if (c1.blue() == 0){
                is_zero[2] = true;
            }

            gradients[0] = std::max(gradients[0], int16_t((int)c2.red() - (int) c1.red()));
            gradients[1] = std::max(gradients[1], int16_t((int)c2.green() - (int) c1.green()));
            gradients[2] = std::max(gradients[2], int16_t((int)c2.blue() - (int) c1.blue()));
        }

        bool has_non_border_channel = false;
        for(int c = 0; c < 3; c++){
            if (is_zero[c] == false && gradients[c] < thres_c[c]){
                has_non_border_channel = true;
                break;
            }
        }
        num_border_pixels += !has_non_border_channel;

        // cout << y << ": (" << gradients[0] << (is_zero[0] ? "*" : "");
        // cout << ", " << gradients[1] << (is_zero[1] ? "*" : "");
        // cout << ", " << gradients[2] << (is_zero[2] ? "*" : "") << ")\n";
        // cout << "has_non_border_channel " << has_non_border_channel << endl;
    }
    
    // cout << "num border pixels proportion: " << num_border_pixels / (float)image.height() << endl;

    return num_border_pixels;
}





}
