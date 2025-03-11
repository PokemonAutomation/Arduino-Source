/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cmath>
#include <cfloat>
#include <functional>
#include <sstream>
#include <array>
#include <string>
#include <vector>
#include <map>
#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageHSV32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonTools/Resources/SpriteDatabase.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonLA_PokemonMapSpriteReader.h"
#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

namespace{

using FeatureType = double;
using FeatureVector = std::vector<FeatureType>;

using ImageMatch::ExactImageDictionaryMatcher;

// We set each sprite resolution to be 50 x 50 during image matching.
const size_t IMAGE_TEMPLATE_SIZE = 50;
// The amount of pixel offset allowed in color matching
const size_t IMAGE_COLOR_MATCH_EXTRA_SIDE_EXT = 2;

const size_t EXTENDED_IMAGE_SIZE = IMAGE_TEMPLATE_SIZE + IMAGE_COLOR_MATCH_EXTRA_SIDE_EXT * 2;

// Defined locally stored data for matching MMO sprites:
// Store data belonging to one sprite
struct PerSpriteMatchingData{
    
    FeatureVector feature;

    ImageStats rgb_stats;
    
    ImageHSV32 hsv_image;
    
    ImageRGB32 gradient_image;
};

using MMOSpriteMatchingMap = std::map<std::string, PerSpriteMatchingData>;

inline bool is_transparent(uint32_t g){
    return (g >> 24) < 128;
}


FeatureType feature_distance(const FeatureVector& a, const FeatureVector& b){
    if (a.size() != b.size()){
        cout << "Error, feature size mismatch " << a.size() << " " << b.size() << endl;
        throw std::runtime_error("feature size mismatch");
    }
    FeatureType sum = 0.0f;
    for(size_t i = 0; i < a.size(); i++){
        FeatureType d = a[i] - b[i];
        sum += d*d;
    }

    return sum;
}

std::string feature_to_str(const FeatureVector& a){
    std::ostringstream os;
    os << "[";
    for(size_t i = 0; i < a.size(); i++){
        if (i != 0){
            os << ", ";
        }
        os << a[i];
    }
    os << "]";
    return os.str();
}

void run_Sobel_gradient_filter(const ImageViewRGB32& image, std::function<void(size_t x, size_t y, int sum_x[3], int sum_y[3])> process_gradient){
    const size_t width = image.width();
    const size_t height = image.height();
    // Kernel for computing gradient along x axis
    const int kx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1},
    };
    // kernel for gradient along y axis
    const int ky[3][3] = {
        { 1,  2,  1},
        { 0,  0,  0},
        {-1, -2, -1},
    };
    const int ksz = 3; // kernel size
    if (width <= ksz || height <= ksz){
        return;
    }
    const size_t x_end = width - ksz + 1;
    const size_t y_end = height - ksz + 1;

    for(size_t y = 0; y < y_end; y++){
        for(size_t x = 0; x < x_end; x++){
            int sum_x[3] = {0, 0, 0};
            int sum_y[3] = {0, 0, 0};
            bool has_alpha_pixel = false;
            for(size_t sy = 0; sy < 3; sy++){
                for(size_t sx = 0; sx < 3; sx++){
                    uint32_t p = image.pixel(x + sx, y + sy);
                    int alpha = p >> 24;
                    if (alpha < 128){
                        // We don't compute gradient when there is a pixel in the kernel
                        // scope that is transparent
                        has_alpha_pixel = true;
                        break;
                    }
                    for(int ch = 0; ch < 3; ch++){ // rgb channel
                        int shift = ch * 8;
                        int c = (uint32_t(0xff) & p >> shift);

                        sum_x[ch] += c * kx[sy][sx];
                        sum_y[ch] += c * ky[sy][sx];
                    }
                }
                if (has_alpha_pixel){
                    break;
                }
            } // end of kernel operation
            if (has_alpha_pixel){
                continue;
            }

            process_gradient(x+1, y+1, sum_x, sum_y);
        }
    }
}

ImageRGB32 smooth_image(const ImageViewRGB32& image){
    // static int count = 0;
    // {
    //     image.save("./test_smooth_before_" + std::to_string(count) + ".png");
    // }

    ImageRGB32 result(image.width(), image.height());
    result.fill(0);

    const float filter[5] = {0.062f, 0.244f, 0.388f, 0.244f, 0.062f};

    size_t image_width = image.width();
    size_t image_height = image.height();
    for(size_t y = 0; y < image_height; y++){
        for(size_t x = 0; x < image_width; x++){
            float sum[3] = {0,0,0};
            float weights = 0.0;
            for(size_t i = 0; i < 5; i++){
                if (x + i < 2 || x + i >= image_width + 2){
                    continue;
                }
                size_t sx = x + i - 2;
            
                uint32_t p = image.pixel(sx, y);
                if (is_transparent(p)){
                    continue;
                }

                weights += filter[i];

                for(int ch = 0; ch < 3; ch++){
                    int shift = 16 - ch * 8;
                    int c = (uint32_t(0xff) & p >> shift);
                    sum[ch] += filter[i] * c;
                }
            }
            if (weights == 0){
                continue;
            }

            char c[3];
            for(int ch = 0; ch < 3; ch++){
                sum[ch] /= weights;
                int v = std::min(std::max(int(sum[ch] + 0.5f), 0), 255);
                c[ch] = (char)v;
            }
            result.pixel(x, y) = combine_rgb(c[0], c[1], c[2]);
        }
    }

    ImageRGB32 result2(image.width(), image.height());
    result2.fill(0);

    for(size_t y = 0; y < image_height; y++){
        for(size_t x = 0; x < image_width; x++){
            float sum[3] = {0,0,0};
            float weights = 0.0;
            for(size_t i = 0; i < 5; i++){
                if (y + i < 2 || y + i - 2 >= image_height){
                    continue;
                }
                size_t sy = y + i - 2;

                uint32_t p = result.pixel(x, sy);
                if (is_transparent(p)){
                    continue;
                }

                weights += filter[i];

                for(int ch = 0; ch < 3; ch++){
                    int shift = 16 - ch * 8;
                    int c = (uint32_t(0xff) & (p >> shift));
                    sum[ch] += filter[i] * c;
                }
            }
            if (weights == 0){
                continue;
            }

            char c[3];
            for(int ch = 0; ch < 3; ch++){
                sum[ch] /= weights;
                int v = std::min(std::max(int(sum[ch] + 0.5f), 0), 255);
                c[ch] = (char)v;
            }
            result2.pixel(x, y) = combine_rgb(c[0], c[1], c[2]);
        }
    }

    // {
    //     result_ref.save("./test_smooth_middle_" + std::to_string(count) + ".png");
    //     result_ref2.save("./test_smooth_after_" + std::to_string(count) + ".png");
    //     count++;
    // }
    // exit(0);

    return result2;
}


ImageRGB32 compute_image_gradient(const ImageViewRGB32& image){
    ImageRGB32 result(image.width(), image.height());
    result.fill(0);

    run_Sobel_gradient_filter(image, [&](size_t x, size_t y, int sum_x[3], int sum_y[3]){
        int gx = (sum_x[0] + sum_x[1] + sum_x[2] + 1) / 3;
        int gy = (sum_y[0] + sum_y[1] + sum_y[2] + 1) / 3;

        uint8_t gxc = (uint8_t)std::min(std::abs(gx), 255);
        uint8_t gyc = (uint8_t)std::min(std::abs(gy), 255);

        result.pixel(x, y) = combine_rgb(gxc, gyc, 0);
    });

    return result;
}

FeatureVector compute_gradient_histogram(const ImageViewRGB32& image){
    const int num_angle_divisions = 8;
    double division_angle = 2. * M_PI / num_angle_divisions;
    double inverse_division_angle = 1.0 / division_angle;

    std::array<int, num_angle_divisions> bin = {0};

    int num_grad = 0;

    run_Sobel_gradient_filter(image, [&](size_t x, size_t y, int sum_x[3], int sum_y[3]){
        int gx = sum_x[0] + sum_x[1] + sum_x[2];
        int gy = sum_y[0] + sum_y[1] + sum_y[2];
        if (gx*gx + gy*gy <= 2000){
            return;
        }
        num_grad++;

        // if (count == 0){
        //     // cout << "gxy " << sum_x[0] << " " << sum_x[1] << " " << sum_x[2] << ", " <<
        //     //  sum_y[0] << " " << sum_y[1] << " " << sum_y[2] << endl;
        //     int gxc = std::min(std::abs(gx), 255);
        //     int gyc = std::min(std::abs(gy), 255);

        //     output_image->setPixelColor(x, y, QColor(gxc, gyc, 0));
        // }

        double angle = std::atan2(gy, gx); // range in -pi, pi
        int bin_idx = int((angle + M_PI) * inverse_division_angle);
        // clamp bin to [0, 11]
        bin_idx = std::min(std::max(bin_idx, 0), num_angle_divisions-1);
        bin[bin_idx]++;
    });

    FeatureVector result(num_angle_divisions);
    for(size_t i = 0; i < num_angle_divisions; i++){
        result[i] = bin[i] / (FeatureType)num_grad;
    }

    return result;
}

} // end anonymous namespace

FeatureVector compute_feature(const ImageViewRGB32& input_image){
    ImageRGB32 image = input_image.copy();
    size_t width = image.width();
    size_t height = image.height();

    // Set pixel outside the sprite circle to transparent:
    double r = (width + height) / 4.0;
    double center_x  = (width-1) / 2.0f;
    double center_y = (height-1) / 2.0f;
    double r2 = r * r;
    for (size_t y = 0; y < height; y++){
        for (size_t x = 0; x < width; x++){
            if ((x-center_x)*(x-center_x) + (y-center_y)*(y-center_y) >= r2){
                image.pixel(x, y) = 0;
            }
        }
    }    

    // Divide the image into 4 areas, compute average color on each.
    // Note: we skip the upper right area because that area may overlap with
    // the berry or star (bonus wave) symbol.
    const int num_divisions = 2;
    const double portion = 1.0 / (double)num_divisions;
    FeatureVector result;
    for(int i = 0; i < num_divisions; i++){
        for(int j = 0; j < num_divisions; j++){
            if (i == 1 && j == 0){
                continue; // skip the berry / bonus wave overlapping area
            }
            ImageFloatBox box{i*portion, j*portion, portion, portion};
            auto sub_image = extract_box_reference(image, box);

            ImageStats stats = image_stats(sub_image);

            result.push_back(stats.average.r);
            result.push_back(stats.average.g);
            result.push_back(stats.average.b);
        }
    }

    return result;
}

void load_and_visit_MMO_sprite(std::function<void(const std::string& slug, const ImageViewRGB32& sprite)> visit_sprit){
    static const SpriteDatabase database("PokemonLA/MMOSprites.png", "PokemonLA/MMOSprites.json");
    for (const auto& item : database){
        // cout << "sprite " << count << endl;
        const std::string& slug = item.first;
        const auto& sprite = item.second.sprite;
        if (sprite.width() != 128 || sprite.height() != 128){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Wrong size of Loaded MMO Sprite sprite: " + slug);
        }
        visit_sprit(slug, item.second.sprite.sub_image(12, 12, 104, 104).scale_to(50, 50));
    }
}

MMOSpriteMatchingMap build_MMO_sprite_matching_data(){

    MMOSpriteMatchingMap sprite_map;

    load_and_visit_MMO_sprite([&](const std::string& slug, const ImageViewRGB32& sprite){

        PerSpriteMatchingData per_sprite_data;

        per_sprite_data.rgb_stats = image_stats(sprite);
        per_sprite_data.hsv_image = ImageHSV32(sprite);

        ImageRGB32 smoothed_sprite = smooth_image(sprite);
        per_sprite_data.gradient_image = compute_image_gradient(smoothed_sprite);
        per_sprite_data.feature = compute_feature(smoothed_sprite);

        sprite_map.emplace(slug, std::move(per_sprite_data));
    });

    return sprite_map;
}

const MMOSpriteMatchingMap& MMO_SPRITE_MATCHING_DATA(){
    const static auto& sprite_matching_data = build_MMO_sprite_matching_data();

    return sprite_matching_data;
}


std::multimap<double, std::string> match_pokemon_map_sprite_feature(const ImageViewRGB32& image, MapRegion region){
    const FeatureVector& image_feature = compute_feature(image);

    const MMOSpriteMatchingMap& sprite_map = MMO_SPRITE_MATCHING_DATA();

    const std::array<std::vector<std::string>, 5>& region_available_sprites = MMO_FIRST_WAVE_REGION_SPRITE_SLUGS();
    int region_index = 0;
    switch(region){
    case MapRegion::FIELDLANDS:
        region_index = 0;
        break;
    case MapRegion::MIRELANDS:
        region_index = 1;
        break;
    case MapRegion::COASTLANDS:
        region_index = 2;
        break;
    case MapRegion::HIGHLANDS:
        region_index = 3;
        break;
    case MapRegion::ICELANDS:
        region_index = 4;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid region.");
//        return {};
    }

    // cout << "input image feature: " << feature_to_str(image_feature) << endl;

    // FeatureType closest_dist = FLT_MAX;
    // std::string closest_slug = "";

    std::multimap<double, std::string> result;

    for(const auto& slug : region_available_sprites[region_index]){
        auto it = sprite_map.find(slug);
        if (it == sprite_map.end()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Inconsistent sprite slug definitions in resource: " + slug);
        }

        const FeatureVector& feature = it->second.feature;
        const FeatureType dist = feature_distance(image_feature, feature);
        result.emplace(dist, slug);
    }

    // cout << "Closest feature distance " << closest_dist << ", slug " << closest_slug << endl;
    // cout << feature_to_str(features.find(closest_slug)->second) << endl;

    return result;
}


ImageHSV32 compute_MMO_sprite_color_hsv(const ImageViewRGB32& image_rgb){
    // Convert the image to HSV during ImageHSV32 class construction
    ImageHSV32 result = [&](){
        if (image_rgb.width() == EXTENDED_IMAGE_SIZE || image_rgb.height() == EXTENDED_IMAGE_SIZE){
            return ImageHSV32(image_rgb);
        }
        // First scale the image
        return ImageHSV32(image_rgb.scale_to(EXTENDED_IMAGE_SIZE, EXTENDED_IMAGE_SIZE));
    }();
    
    const size_t width = result.width();
    const size_t height = result.height();
    
    // Set all the pixels outside the sprite area transparent to avoid matching the template to background colors.
    double r = (width + height) / 4.0;
    double center_x  = (width-1) / 2.0f;
    double center_y = (height-1) / 2.0f;
    // -r/12 to remove some boundary areas
    double dist2_th = (r - r/12) * (r - r/12);
    for (size_t y = 0; y < height; y++){
        for (size_t x = 0; x < width; x++){
            if ((x-center_x)*(x-center_x) + (y-center_y)*(y-center_y) >= dist2_th){
                // color outside of the sprite circle is set to zero transparency
                result.pixel(x, y) = uint32_t(0);
            }
            // else if (x > center_x && y < center_y){
            //     // Upper-right part of the sprite is set to zero transparency to avoid matching the 
            //     // berry or star symbol
            //     result.pixel(x, y) = uint32_t(0);
            // }
        }
    }
    return result;
}


// For a sprite on the screenshot, create gradient image of it
ImageRGB32 compute_MMO_sprite_gradient(const ImageViewRGB32& image){

    ImageRGB32 result = [&](){
        if (image.width() == IMAGE_TEMPLATE_SIZE || image.height() == IMAGE_TEMPLATE_SIZE){
            return smooth_image(image);
        }
        // First scale the image
        return smooth_image(image.scale_to(IMAGE_TEMPLATE_SIZE, IMAGE_TEMPLATE_SIZE));
    }();
    result = compute_image_gradient(result);
    
    size_t width = image.width();
    size_t height = image.height();
    if (width == 0 || height == 0){
        return result;
    }

    // Remove gradients outside of the image area
    double r = (width + height) / 4.0;
    double center_x  = (width-1) / 2.0f;
    double center_y = (height-1) / 2.0f;
    // -r/8 to remove some boundary areas
    double dist2_th = (r - r/8) * (r - r/8);
    for (size_t y = 0; y < height; y++){
        for (size_t x = 0; x < width; x++){
            if ((x-center_x)*(x-center_x) + (y-center_y)*(y-center_y) >= dist2_th){
                // gradients outside of the sprite circle is set to zero
                result.pixel(x, y) = combine_argb(0,0,0,0);
            }
            // else if (x > center_x && y < center_y){
            //     // Upper-right part of the sprite is set to zero transparency to avoid matching the 
            //     // berry or star symbol
            //     result.pixel(x, y) = uint32_t(0);
            // }
        }
    }
    return result;
}











double compute_MMO_sprite_gradient_distance(const ImageViewRGB32& gradient_template, const ImageViewRGB32& gradient){
    int tempt_width = (int)gradient_template.width();
    int tempt_height = (int)gradient_template.height();

    double score = 0.0f;
    int max_offset = 2;

    auto compute_pixel_dist = [](uint32_t t_g, uint32_t g){
        int gx = uint32_t(0xff) & (g >> 16);
        int gy = uint32_t(0xff) & (g >> 8);
        int t_gx = uint32_t(0xff) & (t_g >> 16);
        int t_gy = uint32_t(0xff) & (t_g >> 8);
        double pixel_score = std::max(t_gx, gx) * (gx - t_gx) * (gx - t_gx) + std::max(t_gy, gy) * (gy - t_gy) * (gy - t_gy);
        pixel_score /= 255;
        return pixel_score;
    };


// #define USE_IMAGE_LEVEL_TRANSLATION
// #define USE_PIXEL_LEVEL_TRANSLATION
#define USE_BLOCK_LEVEL_TRANSLATION

#ifdef USE_IMAGE_LEVEL_TRANSLATION
    score = FLT_MAX;
    for(int oy = -max_offset; oy <= max_offset; oy++){ // offset_y
        for(int ox = -max_offset; ox <= max_offset; ox++){ // offset_x

            float match_score = 0.0;
            int num_gradients = 0;
            for(size_t y = 0; y < gradient.height(); y++){
                for(size_t x = 0; x < gradient.width(); x++){
                    uint32_t g = gradient.pixel(x, y);
                    if (is_transparent(g)){
                        continue;
                    }
                    int my = (int)(y + oy); // moved y
                    int mx = (int)(x + ox); // moved x
                    if (mx < 0 || mx >= tempt_width || my < 0 || my >= tempt_height){
                        continue;
                    }

                    uint32_t t_g = gradient_template.pixel(mx, my);
                    if (is_transparent(t_g)){
                        continue;
                    }

                    num_gradients++;

                    float pixel_score = compute_pixel_dist(t_g, g);
                    match_score += pixel_score;

                    // output.setPixelColor(x, y, QColor(
                    //     std::min((int)std::sqrt(gx*gx+gy*gy),255),
                    //     std::min((int)std::sqrt(t_gx*t_gx+t_gy*t_gy), 255),
                    //     0
                    // ));
                }
            }

            match_score = std::sqrt(match_score / num_gradients);
            if (match_score < score){
                score = match_score;
            }
        }
    }
    score = std::sqrt(score);
#endif

#ifdef USE_PIXEL_LEVEL_TRANSLATION
    score = 0;
    int num_gradients = 0;
    for(size_t y = 0; y < gradient.height(); y++){
        for(size_t x = 0; x < gradient.width(); x++){
            uint32_t g = gradient.pixel(x, y);
            uint32_t gx = uint32_t(0xff) & (g >> 16);
            uint32_t gy = uint32_t(0xff) & (g >> 8);
            uint32_t alpha = g >> 24;
            if (alpha < 128){
                continue;
            }

            float min_pixel_score = FLT_MAX;
            for(int oy = -max_offset; oy <= max_offset; oy++){ // offset_y
                for(int ox = -max_offset; ox <= max_offset; ox++){ // offset_x
                    int my = (int)(y + oy); // moved y
                    int mx = (int)(x + ox); // moved x
                    if (mx < 0 || mx >= tempt_width || my < 0 || my >= tempt_height){
                        continue;
                    }
                    // int dist_x = std::abs(ox);
                    // int dist_y = std::abs(oy);
                    // int dist2 = dist_x * dist_x + dist_y * dist_y;
                    uint32_t t_g = scaled_template.pixel(mx, my);
                    uint32_t t_a = t_g >> 24;
                    if (t_a < 128){
                        continue;
                    }

                    uint32_t t_gx = uint32_t(0xff) & (t_g >> 16);
                    uint32_t t_gy = uint32_t(0xff) & (t_g >> 8);
                    float pixel_score = std::max(t_gx, gx) * (gx - t_gx) * (gx - t_gx) + std::max(t_gy, gy) * (gy - t_gy) * (gy - t_gy);
                    pixel_score /= 255;
                    if (pixel_score < min_pixel_score){
                        min_pixel_score = pixel_score;
                    }

                    if (ox == 0 && oy == 0){
                        output.setPixelColor(x, y, QColor(
                            std::min((int)std::sqrt(gx*gx+gy*gy),255),
                            std::min((int)std::sqrt(t_gx*t_gx+t_gy*t_gy), 255),
                            0
                        ));
                    }
                }
            } // end offset

            if (min_pixel_score < FLT_MAX){
                score += min_pixel_score;
                num_gradients++;
            }
        }
    }
    score = std::sqrt(score / num_gradients);
#endif

#ifdef USE_BLOCK_LEVEL_TRANSLATION
    int block_radius = 5;

    score = 0;
    int num_gradients = 0;

    for(int y = 0; y < (int)gradient.height(); y++){
        for(int x = 0; x < (int)gradient.width(); x++){
            uint32_t g = gradient.pixel(x, y);
            if (is_transparent(g)){
                continue;
            }

            double min_block_score = FLT_MAX;
            for(int oy = -max_offset; oy <= max_offset; oy++){ // offset_y
                for(int ox = -max_offset; ox <= max_offset; ox++){ // offset_x

                    double block_score = 0.0;
                    int block_size = 0;
                    for(int by = y - block_radius; by <= y + block_radius; by++){
                        for(int bx = x - block_radius; bx <= x + block_radius; bx++){
                            if (bx < 0 || bx >= (int)gradient.width() || by < 0 || by >= (int)gradient.height()){
                                continue;
                            }

                            uint32_t bg = gradient.pixel(bx, by);
                            if (is_transparent(bg)){
                                continue;
                            }

                            int ty = by + oy; // template y
                            int tx = bx + ox; // template x
                            if (tx < 0 || tx >= tempt_width || ty < 0 || ty >= tempt_height){
                                continue;
                            }
                            uint32_t t_bg = gradient_template.pixel(tx, ty);
                            if (is_transparent(t_bg)){
                                continue;
                            }

                            block_score += compute_pixel_dist(t_bg, bg);
                            block_size++;
                        }
                    }
                    block_score = block_score / block_size;
                    min_block_score = std::min(min_block_score, block_score);
                }
            } // end offset

            if (min_block_score < FLT_MAX){
                score += min_block_score;
                num_gradients++;
            }
        }
    }
    score = std::sqrt(score / num_gradients);
#endif

    // output.save("test_distance_alignment_" + QString::number(count) + ".png");
    // count++;

    return score;
}

double compute_hsv_dist2(uint32_t template_color, uint32_t color){
    int t_h = (uint32_t(0xff) & (template_color >> 16));
    int t_s = (uint32_t(0xff) & (template_color >> 8));
    int t_v = (uint32_t(0xff) & template_color);

    int h = (uint32_t(0xff) & (color >> 16));
    int s = (uint32_t(0xff) & (color >> 8));
    int v = (uint32_t(0xff) & color);

    int h_dif = std::abs(t_h - h);
    if (h_dif > 255 - h_dif){
        h_dif = 256 - h_dif;
    }

    return h_dif * h_dif + (t_s - s) * (t_s - s) + 0.5 * (t_v - v) * (t_v - v);
}

double compute_MMO_sprite_hsv_distance(const ImageViewHSV32& image_template, const ImageViewHSV32& query_image){
    size_t tempt_width = image_template.width();
    size_t tempt_height = image_template.height();

    // cout << tempt_width << " " << tempt_height << " " << query_image.width() << " " << query_image.height() << endl;
    double score = 0.0;
    int num_pixels = 0;
    for (size_t y = 0; y < query_image.height(); y++){
        for (size_t x = 0; x < query_image.width(); x++){
            uint32_t p = query_image.pixel(x, y);
            if (is_transparent(p)){
                // cout << "Skip query pixel " << x << " " << y << endl;
                continue;
            }
            if (x >= tempt_width || y >= tempt_height){
                continue;
            }

            uint32_t t_p = image_template.pixel(x, y);
            if (is_transparent(t_p)){
                // cout << "Skip template pixel " << x << " " << y << endl;
                continue;
            }

            num_pixels++;

            double pixel_score = compute_hsv_dist2(t_p, p);
            score += pixel_score;
        }
    }

    score = std::sqrt(score / num_pixels);
    // cout << "score " << score << " " << num_pixels << endl;
    // exit(0);
    return score;
}


MapSpriteMatchResult match_sprite_on_map(Logger& logger, const ImageViewRGB32& screen, const ImagePixelBox& box, MapRegion region, bool debug_mode){
    const static auto& sprite_map = MMO_SPRITE_MATCHING_DATA();

    auto save_debug_image_if_required = [&](const MapSpriteMatchResult& result){
        if (debug_mode == false){
            return;
        }
        const std::string debug_path = std::string("PokemonLA/PokemonMapSpriteReader/") + std::string(WILD_REGION_SHORT_NAMES[(int)region-2])
             + "/" + result.slug;

        dump_debug_image(logger, debug_path, result.slug, extract_box_reference(screen, box.expand_as(5)));
    };

    // configs for the algorithm:
    const size_t num_feature_candidates = 10;
    const double color_difference_threshold = 10.0;
    const double gradient_confidence_threshold = 2.0;
    const double color_to_gradient_confidence_scale = 2.0;

    MapSpriteMatchResult result;
    logger.log("Start map MMO sprite matching:");

    // This map has the match score for all sprites.
    // This map must be not empty for subsequent computation.
    const auto& feature_map = match_pokemon_map_sprite_feature(extract_box_reference(screen, box), region);
    
    for (const auto& p : feature_map){
        result.candidates.push_back(p.second);
        if (result.candidates.size() >= num_feature_candidates){
            break;
        }
    }

    {
        std::ostringstream os;
        os << "  Candidates: ";
        for(size_t i = 0; i < result.candidates.size(); i++){
            if (i > 0){
                os << ", ";
            }
            os << result.candidates[i];
        }
        logger.log(os.str());
    }

    // Should not happen if we correctly loads the sprite data.
    if (result.candidates.size() == 0){
        return result;
    }

    logger.log("Color matching...");
    {
        const ImagePixelBox expanded_box = box.expand_as(2);
        const ImageHSV32 sprite_hsv = compute_MMO_sprite_color_hsv(extract_box_reference(screen, expanded_box));
        
        for(const auto& slug: result.candidates){
            const ImageHSV32& candidate_template = sprite_map.find(slug)->second.hsv_image;
            double score = FLT_MAX;
            for(size_t ox = 0; ox <= 4; ox++){
                for(size_t oy = 0; oy <= 4; oy++){
                    ImagePixelBox shifted_box(ox, oy, box.width(), box.height());
                    double match_score = compute_MMO_sprite_hsv_distance(
                        candidate_template,
                        extract_box_reference(sprite_hsv, shifted_box)
                    );
                    score = std::min(match_score, score);
                }
            }

            result.color_match_results.emplace(score, slug);
        }
    }

    // Build a map from sprite to their color score and output scores for debugging
    std::map<std::string, double> color_match_sprite_scores;
    int result_count = 0;
    for(const auto& p : result.color_match_results){
        const auto& slug = p.second;
        color_match_sprite_scores.emplace(slug, p.first);
        if (result_count < 5){
            const auto& stats = sprite_map.find(slug)->second.rgb_stats;
            std::ostringstream os;
            os << p.first << " - " << slug << " " << stats.stddev.sum();
            logger.log(os.str());
        }
        if (result_count == 5){
            size_t num_rest_results = result.color_match_results.size() - 5;
            std::ostringstream os;
            os << "Skip " << num_rest_results << " more result" << (num_rest_results > 1 ? "s" : "");
            logger.log(os.str());
        }
        
        result_count++;
    }

    auto color_top = result.color_match_results.begin();
    const std::string& color_top_slug = color_top->second;
    auto color_second = color_top;
    color_second++;

    result.slug = color_top_slug;
    result.color_score = color_top->first;

    if (color_second == result.color_match_results.end()){
        // we only have one color match result.
        result.color_lead = DBL_MAX;
        logger.log("Single color match result: " + color_top_slug);
        save_debug_image_if_required(result);
        return result;
    }

    // We have some sprites which have closer scores to the top color match:
    // Find the difference between the top and second match score
    result.color_lead = color_second->first - result.color_score;
    {
        std::ostringstream os;
        os << "Top color score: " << result.color_score << " diff to second: " << result.color_lead;
        logger.log(os.str());
    }

    if (result.color_lead >= color_difference_threshold){
        logger.log("Color difference large enough. Good match: " + result.slug);
        result.second_slug = color_second->second;
        save_debug_image_if_required(result);
        return result;
    }

    logger.log("Gradient matching...");
    ImageRGB32 gradient_image = compute_MMO_sprite_gradient(extract_box_reference(screen, box));
    
    // std::ostringstream os;
    // os << "test_sprite_gradient" << count << "_" << std::setfill('0') << std::setw(2) << i << ".png";
    // std::string sprite_filename = os.str();
    // gradient_image.save(sprite_filename);

    for(const auto& p : result.color_match_results){
        const auto& slug = p.second;
        ImageViewRGB32 template_gradient = sprite_map.find(slug)->second.gradient_image;
        double score = compute_MMO_sprite_gradient_distance(template_gradient, gradient_image);
        result.gradient_match_results.emplace(score, slug);
    }

    result_count = 0;
    for(const auto& p : result.gradient_match_results){
        if (result_count == 5){
            size_t num_rest_results = result.gradient_match_results.size() - 5;
            std::ostringstream os;
            os << "Skip " << num_rest_results << " more result" << (num_rest_results > 1 ? "s" : "");
            logger.log(os.str());
            break;
        }
        std::ostringstream os;
        os << p.first << " - " << p.second;
        logger.log(os.str());
        result_count++;
    }

    // In case the gradient match is not confident, rely again on the color match:
    auto gradient_top = result.gradient_match_results.begin();
    const auto& gradient_top_slug = gradient_top->second;
    auto gradient_second = gradient_top;
    gradient_second++;
    const auto& gradient_second_slug = gradient_second->second;

    result.slug = gradient_top_slug;
    result.gradient_score = gradient_top->first;
    result.gradient_lead = gradient_second->first - gradient_top->first;
    result.second_slug = gradient_second_slug;

    {
        std::ostringstream os;
        os << "Top gradient score: " << result.gradient_score << " diff to second: " << result.gradient_lead;
        logger.log(os.str());
    }

    if (result.gradient_lead >= gradient_confidence_threshold){
        logger.log("Gradient difference large enough. Good match: " + result.slug);
        save_debug_image_if_required(result);
        return result;
    }

    // The diff between top and second gradient match is close
    // Let's check their color score:
    result.graident_top_color_score = color_match_sprite_scores[gradient_top_slug];
    result.gradient_second_color_score = color_match_sprite_scores[gradient_second_slug];
    double color_diff = result.gradient_second_color_score - result.graident_top_color_score;

    {
        std::ostringstream os;
        os << "Gradient difference not large enough. Check color difference: " << 
            result.graident_top_color_score << " vs " << result.gradient_second_color_score << ", diff: " <<
            color_diff;
        logger.log(os.str());
    }
    
    // If color score is more confident in telling those two sprites apart
    if (std::fabs(color_diff) > result.gradient_lead * color_to_gradient_confidence_scale){ 
        if (color_diff < 0){ // second gradient sprite is better in color matching than the first graident sprite:
            result.pick_gradient_second = true;
            std::swap(result.slug, result.second_slug);
            logger.log("Switch to more confident color result: " + result.slug);
        }
    }else{
        logger.log("Low confidence match: " + result.slug);
    }

    save_debug_image_if_required(result);
    return result;
}





}
}
}
