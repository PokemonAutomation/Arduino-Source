/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/ImageOpener.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/Resources/SpriteDatabase.h"
#include "PokemonLA_PokemonMapSpriteReader.h"
#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"
#include "PokemonLA/Resources/PokemonLA_PokemonSprites.h"

#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/ImageOpener.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/Globals.h"

#include <set>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <cfloat>
#include <cmath>
#include <array>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

namespace {

using FeatureType = double;
using FeatureVector = std::vector<FeatureType>;

using ImageMatch::ExactImageDictionaryMatcher;

// defined locally stored data for matching MMO sprites
struct MMOSpriteMatchingData {
    ExactImageDictionaryMatcher color_matcher;
    ExactImageDictionaryMatcher color_matcher_hsv;
    ExactImageDictionaryMatcher gradient_matcher;
    // sprite slug -> features
    std::map<std::string, FeatureVector> features;

    MMOSpriteMatchingData(
        ExactImageDictionaryMatcher c_matcher,
        ExactImageDictionaryMatcher c_matcher_hsv,
        ExactImageDictionaryMatcher g_matcher,
        std::map<std::string, FeatureVector> f
    )
        : color_matcher(std::move(c_matcher))
        , color_matcher_hsv(std::move(c_matcher_hsv))
        , gradient_matcher(std::move(g_matcher))
        , features(f) {}
};

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

void run_Sobel_gradient_filter(const ImageViewRGB32& image, std::function<void(int x, int y, int sum_x[3], int sum_y[3])> proces_gradient){
    const int width = (int)image.width();
    const int height = (int)image.height();
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
    const int x_end = width - ksz + 1;
    const int y_end = height - ksz + 1;

    for(int y = 0; y < y_end; y++){
        for(int x = 0; x < x_end; x++){
            int sum_x[3] = {0, 0, 0};
            int sum_y[3] = {0, 0, 0};
            bool has_alpha_pixel = false;
            for(int sy = 0; sy < 3; sy++){
                for(int sx = 0; sx < 3; sx++){
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

            proces_gradient(x+1, y+1, sum_x, sum_y);
        }
    }
}

QImage smooth_image(const ImageViewRGB32& image){
    // static int count = 0;
    // {
    //     image.save("./test_smooth_before_" + QString::number(count) + ".png");
    // }

    QImage result((int)image.width(), (int)image.height(), QImage::Format::Format_ARGB32);
    result.fill(QColor(0,0,0,0));
    ImageRef result_ref(result);

    const float filter[5] = {0.062f, 0.244f, 0.388f, 0.244f, 0.062f};

    int image_width = (int)image.width();
    int image_height = (int)image.height();
    for(int y = 0; y < image_height; y++){
        for(int x = 0; x < image_width; x++){
            float sum[3] = {0,0,0};
            float weights = 0.0;
            for(int i = 0; i < 5; i++){
                int sx = x + i - 2;
                if (sx < 0 || sx >= image_width){
                    continue;
                }
            
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
                c[ch] = v;
            }
            result_ref.pixel(x, y) = combine_rgb(c[0], c[1], c[2]);
        }
    }

    QImage result2((int)image.width(), (int)image.height(), QImage::Format::Format_ARGB32);
    result2.fill(QColor(0,0,0,0));
    ImageRef result_ref2(result2);

    for(int y = 0; y < image_height; y++){
        for(int x = 0; x < image_width; x++){
            float sum[3] = {0,0,0};
            float weights = 0.0;
            for(int i = 0; i < 5; i++){
                int sy = y + i - 2;
                
                if (sy < 0 || sy >= image_height){
                    continue;
                }

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
                c[ch] = v;
            }
            result_ref2.pixel(x, y) = combine_rgb(c[0], c[1], c[2]);
        }
    }

    // {
    //     result_ref.save("./test_smooth_middle_" + QString::number(count) + ".png");
    //     result_ref2.save("./test_smooth_after_" + QString::number(count) + ".png");
    //     count++;
    // }
    // exit(0);

    return result2;
}

QImage convert_to_hsv(const ImageViewRGB32& image){
    QImage result((int)image.width(), (int)image.height(), QImage::Format::Format_ARGB32);
    result.fill(QColor(0,0,0,0));
    ImageRef result_ref(result);

    for(int y = 0; y < (int)image.height(); y++){
        for(int x = 0; x < (int)image.width(); x++){
            uint32_t p = image.pixel(x, y);
            if (is_transparent(p)){
                continue;
            }

            int r = (uint32_t(0xff) & (p >> 16));
            int g = (uint32_t(0xff) & (p >> 8));
            int b = (uint32_t(0xff) & p);

            int M = std::max(std::max(r, g), b);
            int m = std::min(std::min(r, g), b);

            int S = 0;
            if (M > 0){
                S = std::min(std::max(255 - (m*255 + M/2)/M, 0), 255);
            }

            double cosH = std::sqrt(r*r + g*g + b*b - r*g - r*b - g*b) * (r - 0.5*g - 0.5*b);
            double Hf = std::acos(std::min(std::max(cosH, -1.0), 1.0)) * (180 / M_PI);
            if (b > g){
                Hf = 360 - Hf;
            }
            // Convert Hf range from [0, 360) to [0, 256)
            int H = std::max(int(Hf * 256.0 / 360.0 + 0.5) % 256, 0);

            result_ref.pixel(x, y) = combine_rgb(H, S, M);
        }
    }

    return result;
}


QImage compute_image_gradient(const ImageViewRGB32& image){
    QImage result((int)image.width(), (int)image.height(), QImage::Format::Format_ARGB32);
    result.fill(QColor(0,0,0,0));
    ImageRef result_ref(result);

    run_Sobel_gradient_filter(image, [&](int x, int y, int sum_x[3], int sum_y[3]){
        int gx = (sum_x[0] + sum_x[1] + sum_x[2] + 1) / 3;
        int gy = (sum_y[0] + sum_y[1] + sum_y[2] + 1) / 3;

        int gxc = std::min(std::abs(gx), 255);
        int gyc = std::min(std::abs(gy), 255);

        result_ref.pixel(x, y) = combine_rgb(gxc, gyc, 0);
    });

    return result;
}

FeatureVector compute_gradient_histogram(const ImageViewRGB32& image){
    const int num_angle_divisions = 8;
    double division_angle = 2. * M_PI / num_angle_divisions;
    double inverse_division_angle = 1.0 / division_angle;

    std::array<int, num_angle_divisions> bin = {0};

    int num_grad = 0;

    run_Sobel_gradient_filter(image, [&](int x, int y, int sum_x[3], int sum_y[3]){
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
    QImage image = input_image.to_QImage_owning().convertToFormat(QImage::Format::Format_ARGB32);
    ImageRef image_ref(image);
    int width = image.width();
    int height = image.height();

    // Set pixel outside the sprite circle to transparent:
    float r = (width + height) / 4.0;
    float center_x  = (width-1) / 2.0f;
    float center_y = (height-1) / 2.0f;
    float r2 = r * r;
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            if ((x-center_x)*(x-center_x) + (y-center_y)*(y-center_y) >= r2){
                image_ref.pixel(x, y) = 0;
            }
        }
    }    

    // Divide the image into 4 areas, compute average color on each.
    // Note: we skip the upper right area because that area may overlap with
    // the berry or bonus wave symbol.
    const int num_divisions = 2;
    const float portion = 1.0 / (float)num_divisions;
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

void load_and_visit_MMO_sprite(std::function<void(const std::string& slug, const QImage& sprite)> visit_sprit){
    static const SpriteDatabase database("PokemonLA/MMOSprites.png", "PokemonLA/MMOSprites.json");
    for (const auto& item : database){
        // cout << "sprite " << count << endl;
        const std::string& slug = item.first;
        visit_sprit(slug, item.second.sprite.scaled_to_QImage(50, 50));
    }
}

MMOSpriteMatchingData build_MMO_sprite_matching_data(){
    ImageMatch::WeightedExactImageMatcher::InverseStddevWeight color_stddev_weight;
    color_stddev_weight.stddev_coefficient = 0.004;
    // stddev_weight.stddev_coefficient = 0.1;
    color_stddev_weight.offset = 1.0;
    ExactImageDictionaryMatcher color_matcher(color_stddev_weight);

    ImageMatch::WeightedExactImageMatcher::InverseStddevWeight gradient_stddev_weight;
    gradient_stddev_weight.stddev_coefficient = 0.000;
    // stddev_weight.stddev_coefficient = 0.1;
    gradient_stddev_weight.offset = 1.0;
    ExactImageDictionaryMatcher gradient_matcher(gradient_stddev_weight);
    ExactImageDictionaryMatcher color_matcher_hsv(color_stddev_weight);

    std::map<std::string, FeatureVector> features;

    load_and_visit_MMO_sprite([&](const std::string& slug, const QImage& sprite){
        color_matcher.add(slug, sprite);
        QImage sprite_hsv = convert_to_hsv(sprite);
        color_matcher_hsv.add(slug, sprite_hsv);
        QImage smoothed_sprite = smooth_image(sprite);
        QImage sprite_gradient = compute_image_gradient(smoothed_sprite);
        gradient_matcher.add(slug, sprite_gradient);

        features.emplace(slug, compute_feature(smoothed_sprite));
    });

    return MMOSpriteMatchingData(
        std::move(color_matcher),
        std::move(color_matcher_hsv),
        std::move(gradient_matcher),
        std::move(features)
    );
}

const MMOSpriteMatchingData& MMO_SPRITE_MATCHING_DATA(){
    const static auto& sprite_matching_data = build_MMO_sprite_matching_data();

    return sprite_matching_data;
}


std::multimap<double, std::string> match_pokemon_map_sprite_feature(const ImageViewRGB32& image, MapRegion region){
    const FeatureVector& image_feature = compute_feature(image);

    const std::map<std::string, FeatureVector>& features = MMO_SPRITE_MATCHING_DATA().features;

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
        auto it = features.find(slug);
        if (it == features.end()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Inconsistent sprite slug definitions in resource: " + slug);
        }

        const FeatureVector& feature = it->second;
        const FeatureType dist = feature_distance(image_feature, feature);
        result.emplace(dist, slug);
    }

    // cout << "Closest feature distance " << closest_dist << ", slug " << closest_slug << endl;
    // cout << feature_to_str(features.find(closest_slug)->second) << endl;

    return result;
}


// For a sprite on the screenshot, create gradient image of it
QImage compute_MMO_sprite_gradient(const ImageViewRGB32& image){

    QImage result = smooth_image(image);
    result = compute_image_gradient(result);
    ImageRef result_ref(result);
    
    int width = (int)image.width();
    int height = (int)image.height();
    float r = (width + height) / 4.0;
    float center_x  = (width-1) / 2.0f;
    float center_y = (height-1) / 2.0f;
    // -r/8 to remove some boundary areas
    float dist2_th = (r - r/8) * (r - r/8);
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            if ((x-center_x)*(x-center_x) + (y-center_y)*(y-center_y) >= dist2_th){
                // gradients outside of the sprite circle is set to zero
                result_ref.pixel(x, y) = combine_argb(0,0,0,0);
            }
        }
    }    
    return result;
}











float compute_MMO_sprite_gradient_distance(const ImageViewRGB32& gradient_template, const ImageViewRGB32& gradient){
    int tempt_width = (int)gradient_template.width();
    int tempt_height = (int)gradient_template.height();

    // static int count = 0;
    // QImage output(gradient.width(), gradient.height(), QImage::Format::Format_ARGB32);
    // output.fill(QColor(0,0,0,0));

    // cout << "Size check " << tempt_width << " x " << tempt_height << ",  " <<
    // gradient.width() << " x " << gradient.height() << endl;

    float score = 0.0f;
    int max_offset = 2;

    auto compute_pixel_dist = [](uint32_t t_g, uint32_t g){
        int gx = uint32_t(0xff) & (g >> 16);
        int gy = uint32_t(0xff) & (g >> 8);
        int t_gx = uint32_t(0xff) & (t_g >> 16);
        int t_gy = uint32_t(0xff) & (t_g >> 8);
        float pixel_score = std::max(t_gx, gx) * (gx - t_gx) * (gx - t_gx) + std::max(t_gy, gy) * (gy - t_gy) * (gy - t_gy);
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
            for(int y = 0; y < gradient.height(); y++){
                for(int x = 0; x < gradient.width(); x++){
                    uint32_t g = gradient.pixel(x, y);
                    if (is_transparent(g)){
                        continue;
                    }
                    int my = y + oy; // moved y
                    int mx = x + ox; // moved x
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
    for(int y = 0; y < gradient.height(); y++){
        for(int x = 0; x < gradient.width(); x++){
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
                    int my = y + oy; // moved y
                    int mx = x + ox; // moved x
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

            float min_block_score = FLT_MAX;
            for(int oy = -max_offset; oy <= max_offset; oy++){ // offset_y
                for(int ox = -max_offset; ox <= max_offset; ox++){ // offset_x

                    float block_score = 0.0;
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

float compute_hsv_dist2(uint32_t template_color, uint32_t color){
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

float compute_MMO_sprite_hsv_distance(const ImageViewRGB32& image_template, const ImageViewRGB32& query_image){
    int tempt_width = (int)image_template.width();
    int tempt_height = (int)image_template.height();

    // cout << tempt_width << " " << tempt_height << " " << query_image.width() << " " << query_image.height() << endl;
    float score = 0.0;
    int num_pixels = 0;
    for(int y = 0; y < (int)query_image.height(); y++){
        for(int x = 0; x < (int)query_image.width(); x++){
            uint32_t p = query_image.pixel(x, y);
            if (is_transparent(p)){
                // cout << "Skip query pixel " << x << " " << y << endl;
                continue;
            }
            int mx = x;
            int my = y;
            if (mx < 0 || mx >= tempt_width || my < 0 || my >= tempt_height){
                continue;
            }

            uint32_t t_p = image_template.pixel(mx, my);
            if (is_transparent(t_p)){
                // cout << "Skip template pixel " << x << " " << y << endl;
                continue;
            }

            num_pixels++;

            float pixel_score = compute_hsv_dist2(t_p, p);
            score += pixel_score;
        }
    }

    score = std::sqrt(score / num_pixels);
    // cout << "score " << score << " " << num_pixels << endl;
    // exit(0);
    return score;
}


MapSpriteMatchResult match_sprite_on_map(const ImageViewRGB32& screen, const ImagePixelBox& box, MapRegion region){
    const static auto& sprite_matching_data = MMO_SPRITE_MATCHING_DATA();
    const ExactImageDictionaryMatcher& color_matcher = sprite_matching_data.color_matcher;
    const ExactImageDictionaryMatcher& color_matcher_hsv = sprite_matching_data.color_matcher_hsv;
    const ExactImageDictionaryMatcher& gradient_matcher = sprite_matching_data.gradient_matcher;

    // configs for the algorithm:
    const size_t num_feature_candidates = 10;
    const double color_difference_threshold = 10.0;
    const double gradient_confidence_threshold = 2.0;
    const double color_to_gradient_confidence_scale = 2.0;

    MapSpriteMatchResult result;
    cout << "Map sprite matching:" << endl;

    // This map has the match score for all sprites.
    // This map must be not empty for subsequent computation.
    const auto& feature_map = match_pokemon_map_sprite_feature(extract_box_reference(screen, box), region);
    
    for(const auto& p : feature_map){
        result.candidates.push_back(p.second);
        if (result.candidates.size() >= num_feature_candidates){
            break;
        }
    }
    cout << "  Candidates: ";
    for(size_t i = 0; i < result.candidates.size(); i++){
        if (i > 0){
            cout << ", ";
        }
        cout << result.candidates[i];
    }
    cout << endl;

    // Should not happen if we correctly loads the sprite data.
    if (result.candidates.size() == 0){
        return result;
    }

    cout << "Color matching..." << endl;
    {
        // auto color_match_results = color_matcher.subset_match(result.candidates, screen,
        //     pixelbox_to_floatbox(screen, box), 1, 10);
        // result.color_match_results = std::move(color_match_results.results);
        ImagePixelBox expanded_box(box.min_x - 2, box.min_y - 2, box.max_x + 2, box.max_y + 2);
        QImage sprite_hsv = convert_to_hsv(extract_box_reference(screen, expanded_box));
        for(const auto& slug: result.candidates){
            ImageViewRGB32 candidate_template = color_matcher_hsv.image_template(slug);
            float score = FLT_MAX;
            for(int ox = -2; ox <= 2; ox++){
                for(int oy = -2; oy <= 2; oy++){
                    ImagePixelBox shifted_box(ox+2, oy+2, box.width(), box.height());
                    float match_score = compute_MMO_sprite_hsv_distance(
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
            const auto& stats = color_matcher.image_matcher(slug).stats();
            cout << p.first << " - " << slug << " " << stats.stddev.sum() << endl;
        }
        if (result_count == 5){
            size_t num_rest_results = result.color_match_results.size() - 5;
            cout << "Skip " << num_rest_results << " more result" << (num_rest_results > 1 ? "s" : "") << endl;
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
        cout << "Single color match result: " << color_top_slug << endl;
        return result;
    }

    // We have some sprites which have closer scores to the top color match:
    // Find the difference between the top and second match score
    result.color_lead = color_second->first - result.color_score;

    cout << "Top color score: " << result.color_score << " diff to second: " << result.color_lead << endl;

    if (result.color_lead >= color_difference_threshold){
        cout << "Color difference large enough. Good match: " << result.slug << endl;
        result.second_slug = color_second->second;
        return result;
    }

    cout << "Gradient matching..." << endl;
    QImage gradient_image = compute_MMO_sprite_gradient(extract_box_reference(screen, box));
    
    // std::ostringstream os;
    // os << "test_sprite_gradient" << count << "_" << std::setfill('0') << std::setw(2) << i << ".png";
    // std::string sprite_filename = os.str();
    // gradient_image.save(QString::fromStdString(sprite_filename));

    for(const auto& p : result.color_match_results){
        const auto& slug = p.second;
        ImageViewRGB32 template_gradient = gradient_matcher.image_template(slug);
        double score = compute_MMO_sprite_gradient_distance(template_gradient, gradient_image);
        result.gradient_match_results.emplace(score, slug);
    }

    result_count = 0;
    for(const auto& p : result.gradient_match_results){
        if (result_count == 5){
            size_t num_rest_results = result.gradient_match_results.size() - 5;
            cout << "Skip " << num_rest_results << " more result" << (num_rest_results > 1 ? "s" : "") << endl;
            break;
        }
        cout << p.first << " - " << p.second << endl;
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

    cout << "Top gradient score: " << result.gradient_score << " diff to second: " << result.gradient_lead << endl;

    if (result.gradient_lead >= gradient_confidence_threshold){
        cout << "Gradient difference large enough. Good match: " << result.slug << endl;
        return result;
    }

    // The diff between top and second gradient match is close
    // Let's check their color score:
    result.graident_top_color_score = color_match_sprite_scores[gradient_top_slug];
    result.gradient_second_color_score = color_match_sprite_scores[gradient_second_slug];
    double color_diff = result.gradient_second_color_score - result.graident_top_color_score;

    cout << "Gradient difference not large enough. Check color difference: " << 
        result.graident_top_color_score << " vs " << result.gradient_second_color_score << ", diff: " <<
        color_diff << endl;
    
    // If color score is more confident in telling those two sprites apart
    if (std::fabs(color_diff) > result.gradient_lead * color_to_gradient_confidence_scale){ 
        if (color_diff < 0){ // second gradient sprite is better in color matching than the first graident sprite:
            result.pick_gradient_second = true;
            std::swap(result.slug, result.second_slug);
            cout << "Switch to more confident color result: " << result.slug << endl;
        }
    }
    else{
        cout << "Low confidence match: " << result.slug << endl;
    }

    return result;
}





}
}
}
