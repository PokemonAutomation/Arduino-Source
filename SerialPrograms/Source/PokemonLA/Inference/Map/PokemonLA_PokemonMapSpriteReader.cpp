/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "PokemonLA_PokemonMapSpriteReader.h"
#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"
#include "PokemonLA/Resources/PokemonLA_PokemonIcons.h"

#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/ImageOpener.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/Globals.h"

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

using FeatureType = double;
using FeatureVector = std::vector<FeatureType>;

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

FeatureVector compute_gradient_histogram(const ConstImageRef& image){
    int width = image.width();
    int height = image.height();
    // Kernel for computing gradient along x axis
    int kx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1},
    };
    // kernel for gradient along y axis
    int ky[3][3] = {
        { 1,  2,  1},
        { 0,  0,  0},
        {-1, -2, -1},
    };
    int ksz = 3; // kernel size
    int x_end = width - ksz + 1;
    int y_end = height - ksz + 1;

    const int num_angle_divisions = 8;
    double division_angle = 2. * M_PI / num_angle_divisions;
    double inverse_division_angle = 1.0 / division_angle;

    std::array<int, num_angle_divisions> bin = {0};

    static int count = 0;

    std::unique_ptr<QImage> output_image;
    if (count == 0){
        output_image = std::make_unique<QImage>(image.to_qimage());
        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                output_image->setPixelColor(x, y, QColor(0,0,0,0));
            }
        }
    }

    int num_grad = 0;
    for(int y = 0; y < y_end; y++){
        for(int x = 0; x < x_end; x++){
            int sum_x[3] = {0, 0, 0};
            int sum_y[3] = {0, 0, 0};
            bool has_alpha_pixel = false;
            for(int sy = 0; sy < 3; sy++){
                for(int sx = 0; sx < 3; sx++){
                    uint32_t p = image.pixel(x + sx, y + sy);
                    if (count == 0 && x == 6 && y == 6){
                        Color pc(p);
                        cout << "(6,6) " << sx << " " << sy << ", " << (int)pc.a() << " " << (int)pc.r() << " " << (int)pc.g() << " " << (int)pc.b() << endl;
                    }
                    int alpha = p >> 24;
                    if (alpha < 128){
                        has_alpha_pixel = true;
                        break;
                    }
                    for(int ch = 0; ch < 3; ch++){ // rgb channel
                        int shift = ch * 8;
                        int c = (uint32_t(0xff) & p >> shift);
                        // if (count == 0){
                        //     cout << "c = " << c << endl;
                        // }
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
            
            num_grad++;

            int gx = sum_x[0] + sum_x[1] + sum_x[2];
            int gy = sum_y[0] + sum_y[1] + sum_y[2];

            if (gx*gx + gy*gy <= 2000){
                continue;
            }

            if (count == 0){
                // cout << "gxy " << sum_x[0] << " " << sum_x[1] << " " << sum_x[2] << ", " <<
                //  sum_y[0] << " " << sum_y[1] << " " << sum_y[2] << endl;
                
                int gxc = std::min(std::abs(gx), 255);
                int gyc = std::min(std::abs(gy), 255);

                output_image->setPixelColor(x, y, QColor(gxc, gyc, 0));
            }

            double angle = std::atan2(gy, gx); // range in -pi, pi
            int bin_idx = int((angle + M_PI) * inverse_division_angle);
            // clamp bin to [0, 11]
            bin_idx = std::min(std::max(bin_idx, 0), num_angle_divisions-1);
            bin[bin_idx]++;
        }
    }

    FeatureVector result(num_angle_divisions);
    for(size_t i = 0; i < num_angle_divisions; i++){
        result[i] = bin[i] / (FeatureType)num_grad;
    }

    if (count == 0){
        cout << "GRadient histogram " << feature_to_str(result) << endl; 
        output_image->save("test_gradient_histogram.png");
    }
    count++;

    return result;
}

FeatureVector compute_feature(const QImage& input_image){
    static int count = 0;

    // int width = image.width();
    // int height = image.height();
    QImage image = input_image.convertedTo(QImage::Format::Format_ARGB32);
    int width = image.width();
    int height = image.height();
    float r = (width + height) / 4.0;
    float center_x  = (width-1) / 2.0f;
    float center_y = (height-1) / 2.0f;
    float r2 = r * r;
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            if ((x-center_x)*(x-center_x) + (y-center_y)*(y-center_y) >= r2){
                image.setPixelColor(x, y, QColor(0, 0, 0, 0));
            }
        }
    }    

    const int num_divisions = 3;
    const float portion = 1.0 / (float)num_divisions;
    FeatureVector result;
    for(int i = 0; i < num_divisions; i++){
        for(int j = 0; j < num_divisions; j++){
            ImageFloatBox box{i*portion, j*portion, portion, portion};
            auto sub_image = extract_box_reference(image, box);

            ImageStats stats = image_stats(sub_image);

            // const double average = stats.average.sum();
            // const FloatPixel actual = (std::isnan(average) || average == 0.0 ? FloatPixel() : stats.average / average);
            // result.push_back(actual.r);
            // result.push_back(actual.g);
            // result.push_back(actual.b);

            FeatureType scale = 1.0 / 255;
            result.push_back(stats.average.r * scale);
            result.push_back(stats.average.g * scale);
            result.push_back(stats.average.b * scale);

            auto sub_result = compute_gradient_histogram(sub_image);
            result.insert(result.end(), sub_result.begin(), sub_result.end());
        }
    }
    // auto gradient_result = compute_gradient_histogram(image);
    // result.insert(result.end(), gradient_result.begin(), gradient_result.end());
    // ImageStats stats = image_stats(image);


    
    if (count == 0){
        image.save("test_compute_feature.png");
    }
    count++;

    return result;
}

std::map<std::string, FeatureVector> compute_MMO_sprite_features(){
    // cout << "compute_MMO_sprite_features" << endl;

    std::map<std::string, FeatureVector> features;
    
    const char* sprite_path = "PokemonLA/MMOSprites.png";
    const char* json_path = "PokemonLA/MMOSprites.json";

    QImage sprites = open_image(RESOURCE_PATH() + sprite_path);
    QJsonObject json = read_json_file(
        RESOURCE_PATH() + json_path
    ).object();

    int width = json.find("spriteWidth")->toInt();
    int height = json.find("spriteHeight")->toInt();
    if (width <= 0){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid width.", (RESOURCE_PATH() + json_path).toStdString());
    }
    if (height <= 0){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid height.", (RESOURCE_PATH() + json_path).toStdString());
    }

    int count = 0;

    QJsonObject locations = json.find("spriteLocations")->toObject();
    for (auto iter = locations.begin(); iter != locations.end(); ++iter){
        // cout << "sprite " << count << endl;

        std::string slug = iter.key().toStdString();
        QJsonObject obj = iter.value().toObject();
        int y = obj.find("top")->toInt();
        int x = obj.find("left")->toInt();

        QImage sprite = sprites.copy(x, y, width, height);

        int width = sprite.width();
        int height = sprite.height();
        float center_x  = (width-1) / 2.0f;
        float center_y = (height-1) / 2.0f;
        int r = width/2 - 3;
        int r2 = r * r;
        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                if ((x-center_x)*(x-center_x) + (y-center_y)*(y-center_y) >= r2){
                    sprite.setPixelColor(x, y, QColor(0, 0, 0, 0));
                }
            }
        }

        sprite = sprite.scaled(52, 52);

        if (count == 0){
            sprite.save("test_sprite.png");
        }

        count++;

        features.emplace(slug, compute_feature(sprite));
    }

    // check feature self distances:
    FeatureType closest_dist = FLT_MAX;
    std::pair<std::string, std::string> closest_pair;
    for(auto it = features.begin(); it != features.end(); it++){
        auto it2 = it;
        it2++;
        for(; it2 != features.end(); it2++){
            auto dist = feature_distance(it->second, it2->second);
            if (dist < closest_dist){
                closest_dist = dist;
                closest_pair = std::make_pair(it->first, it2->first);
            }
        }
    }

    cout << "Closest distance " << closest_dist << " at " << endl;
    cout << closest_pair.first << ": " << feature_to_str(features.find(closest_pair.first)->second) << endl;
    cout << closest_pair.second << ": " << feature_to_str(features.find(closest_pair.second)->second) << endl;
    cout << endl;
    
    return features;
}

const std::map<std::string, FeatureVector>& MMO_SPRITE_FEATUES(){
    const static std::map<std::string, FeatureVector> features = compute_MMO_sprite_features();
    return features;
}



std::string match_pokemon_map_sprite(const QImage& image){
    const FeatureVector& image_feature = compute_feature(image);


    const std::map<std::string, FeatureVector>& features = MMO_SPRITE_FEATUES();

    cout << "input image feature: " << feature_to_str(image_feature) << endl;

    FeatureType closest_dist = FLT_MAX;
    std::string closest_slug = "";

    for(const auto& p : features){
        const std::string& slug = p.first;
        const FeatureVector& feature = p.second;
        const FeatureType dist = feature_distance(image_feature, feature);
        if (dist < closest_dist){
            closest_dist = dist;
            closest_slug = slug;
        }
    }

    cout << "Closest feature distance " << closest_dist << ", slug " << closest_slug << endl;
    cout << feature_to_str(features.find(closest_slug)->second) << endl;

    return closest_slug;
}



}
}
}
