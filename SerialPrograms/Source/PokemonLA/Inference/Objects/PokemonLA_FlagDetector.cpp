/*  Flag Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonLA_FlagDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels;
using namespace Kernels::Waterfill;


void FlagMatcher_make_template(){
    QImage image("Flag-Original.png");
    image = image.scaled(image.width() / 2, image.height() / 2);
    image = image.convertToFormat(QImage::Format::Format_ARGB32);
    uint32_t* ptr = (uint32_t*)image.bits();
    size_t words = image.bytesPerLine() / sizeof(uint32_t);
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            uint32_t& pixel = ptr[r * words + c];
            uint32_t red = qRed(pixel);
            uint32_t green = qGreen(pixel);
            uint32_t blue = qBlue(pixel);
//            if (red < 128 && green < 128 && blue < 128){
//                pixel = 0x00000000;
//            }
            if (red < 128 || green < 128 || blue < 128){
                pixel = 0x00000000;
            }
        }
    }
    image.save("Flag-Template0.png");
}




class FlagMatcher : public ImageMatch::SubObjectTemplateMatcher{
public:
    FlagMatcher(bool left)
        : SubObjectTemplateMatcher("PokemonLA/Flag-Template.png", 100)
    {
        PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(
            m_object,
            128, 255,
            128, 255,
            128, 255
        );
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20, false);
        if (objects.size() != 2){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Failed to find exactly 2 objects in resource.",
                m_path.toStdString()
            );
        }

        if (left == (objects[0].min_x < objects[1].min_x)){
            set_subobject(objects[0]);
        }else{
            set_subobject(objects[1]);
        }
    }

    static const FlagMatcher& left(){
        static FlagMatcher matcher(true);
        return matcher;
    }
    static const FlagMatcher& right(){
        static FlagMatcher matcher(false);
        return matcher;
    }
};




FlagDetector::FlagDetector()
    : WhiteObjectDetector(
        COLOR_CYAN,
        {
            Color(0xff808080),
            Color(0xff909090),
            Color(0xffa0a0a0),
            Color(0xffb0b0b0),
            Color(0xffc0c0c0),
            Color(0xffd0d0d0),
        }
    )
{}
void FlagDetector::process_object(const QImage& image, const WaterfillObject& object){
    if (object.area < 100){
        return;
    }
    if (object.height() > 0.04 * image.height()){
        return;
    }
    if (object.height() < 0.01 * image.height()){
        return;
    }
    if (object.width() > 0.03 * image.width()){
        return;
    }
    ImagePixelBox object_box;
    if (FlagMatcher::left().matches(object_box, image, object)){
        m_left.emplace_back(object_box);
    }
    if (FlagMatcher::right().matches(object_box, image, object)){
        m_right.emplace_back(object_box);
    }
}
void FlagDetector::finish(){
//    cout << "left  = " << m_left.size() << endl;
//    cout << "right = " << m_right.size() << endl;

    //  Merge left/right parts.
    for (auto iter0 = m_left.begin(); iter0 != m_left.end();){
        double height = (double)iter0->height();
        double width = (double)iter0->width();
        bool removed = false;
        for (auto iter1 = m_right.begin(); iter1 != m_right.end(); ++iter1){
            double height_ratio = height / iter1->height();
            if (height_ratio < 0.8 || height_ratio > 1.2){
                continue;
            }
            double width_ratio = width / iter1->width();
            if (width_ratio < 0.8 || width_ratio > 1.2){
                continue;
            }

            double horizontal_offset = std::abs((iter0->min_x - iter1->min_x) / width);
            if (horizontal_offset > 0.1){
                continue;
            }

            double vertical_offset = std::abs((iter0->min_y - iter1->min_y) / height);
            if (vertical_offset > 0.1){
                continue;
            }

            m_detections.emplace_back(
                iter0->min_x,
                std::min(iter0->min_y, iter1->min_y),
                iter1->max_x,
                std::max(iter0->max_y, iter1->max_y)
            );
            iter0 = m_left.erase(iter0);
            m_right.erase(iter1);
            removed = true;
            break;
        }
        if (!removed){
            ++iter0;
        }
    }
    m_left.clear();
    m_right.clear();
    merge_heavily_overlapping(0.3);
}







class DigitMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DigitMatcher(const char* path)
        : WaterfillTemplateMatcher(path, Color(0xffa0a0a0), Color(0xffffffff), 30)
    {}
};

std::vector<std::pair<int, DigitMatcher>> make_digit_matchers(){
    std::vector<std::pair<int, DigitMatcher>> matchers;
    matchers.emplace_back(0, "PokemonLA/Digits/Digit-0-Template.png");
    matchers.emplace_back(1, "PokemonLA/Digits/Digit-1-Template.png");
    matchers.emplace_back(2, "PokemonLA/Digits/Digit-2-Template.png");
    matchers.emplace_back(3, "PokemonLA/Digits/Digit-3-Template.png");
    matchers.emplace_back(4, "PokemonLA/Digits/Digit-4-Template.png");
    matchers.emplace_back(5, "PokemonLA/Digits/Digit-5-Template.png");
    matchers.emplace_back(6, "PokemonLA/Digits/Digit-6-Template.png");
    matchers.emplace_back(7, "PokemonLA/Digits/Digit-7-Template.png");
    matchers.emplace_back(8, "PokemonLA/Digits/Digit-8-Template.png");
    matchers.emplace_back(9, "PokemonLA/Digits/Digit-9-Template.png");
    return matchers;
}


std::pair<double, int> read_digit(const QImage& image, const WaterfillObject& object){
    static const std::vector<std::pair<int, DigitMatcher>> MATCHERS = make_digit_matchers();
    double best_rmsd = 99999;
    int best_digit = -1;
    for (const auto& item : MATCHERS){
//        cout << item.first << " : " <<  << endl;
        double rmsd = item.second.rmsd_original(image, object);
        if (best_rmsd > rmsd){
            best_rmsd = rmsd;
            best_digit = item.first;
        }
    }
    if (best_rmsd > 80){
        best_digit = -1;
    }
    return {best_rmsd, best_digit};
}



int read_flag_distance(const QImage& screen, double flag_x, double flag_y){
    ImageFloatBox box(flag_x - 0.025, flag_y - 0.055, 0.045, 0.025);
    QImage image = extract_box(screen, box);
//    image.save("test.png");

    PackedBinaryMatrix2 matrix[6];
    compress4_rgb32_to_binary_range(
        image,
        matrix[0], 0xff808080, 0xffffffff,
        matrix[1], 0xff909090, 0xffffffff,
        matrix[2], 0xffa0a0a0, 0xffffffff,
        matrix[3], 0xffb0b0b0, 0xffffffff
    );
    compress2_rgb32_to_binary_range(
        image,
        matrix[4], 0xffc0c0c0, 0xffffffff,
        matrix[5], 0xffd0d0d0, 0xffffffff
    );

    size_t width = matrix[0].width();
    size_t height = matrix[0].height();
    double inv_width = 0.5 / width;

    struct Hit{
        size_t min_x;
        size_t max_x;
        double mid_x;
        double rmsd;
        int digit;
    };


    //  Detect all the digits.
    std::multimap<size_t, Hit> hits;
    for (size_t c = 0; c < 6; c++){
        auto finder = make_WaterfillIterator(matrix[c], 30);
        WaterfillObject object;
        while (finder->find_next(object)){
            //  Skip anything that touches the edge.
            if (object.min_x == 0 || object.min_y == 0 ||
                object.max_x + 1 == width || object.max_y + 1 == height
            ){
                continue;
            }
    //        extract_box(image, object).save("image-" + QString::number(c++) + ".png");
            std::pair<double, int> digit = read_digit(image, object);
            if (digit.second >= 0){
                hits.emplace(
                    object.min_x,
                    Hit{
                        object.min_x,
                        object.max_x,
                        (object.min_x + object.max_x) * inv_width,
                        digit.first,
                        digit.second
                    }
                );
            }
        }
    }
    if (hits.empty()){
        return -1;
    }

//    for (const auto& item : hits){
//        cout << item.first << " : " << item.second.second << " - " << item.second.first << endl;
//    }



    //  Remove overlapping detections by picking the one with strongest detection on each overlap.
    std::vector<Hit> digits;
    auto best = hits.begin();
    auto iter = best;
    ++iter;
    for (; iter != hits.end(); ++iter){
        //  Next digit
        if (best->second.max_x < iter->second.min_x){
            digits.emplace_back(best->second);
            best = iter;
        }

        //  Overlapping. Pick better score.
        if (best->second.rmsd > iter->second.rmsd){
            best = iter;
        }
    }
    digits.emplace_back(best->second);



    //  Now we use the position of the digits to correct for errors.

    int even_buckets[4] = {-1, -1, -1, -1};
    int odd_buckets[3] = {-1, -1, -1};
    size_t even_count = 0;
    size_t odd_count = 0;

    //  Put every digit into one of 7 buckets.
    for (const Hit& digit : digits){
//        cout << "[" << digit.mid_x << " : " << digit.digit << "]";
        int bucket = (int)(digit.mid_x * 9.55557 - 1.4);
        bucket = std::max(bucket, 0);
        bucket = std::min(bucket, 6);
        if (bucket % 2){
            odd_buckets[bucket / 2] = digit.digit;
            odd_count++;
        }else{
            even_buckets[bucket / 2] = digit.digit;
            even_count++;
        }
    }
//    cout << endl;

//    cout << "even = " << even_count << endl;
//    cout << "odd  = " << odd_count << endl;


    //  All the digits must call into either odd or even buckets.
    //  Anything else is a misread and we must return undetected.
    if ((even_count != 0) == (odd_count != 0)){
        return -1;
    }

//    cout << odd_buckets[0] << odd_buckets[1] << odd_buckets[2] << endl;

    if (odd_count != 0){
        //  1 digit only.
        if (odd_buckets[0] < 0 && odd_buckets[2] < 0){
            //  Return unconditionally. If it's one digit, then it matters so don't try to assume anything if it can't be read.
            return odd_buckets[1];
        }

        //  Now we know it's 3 digits for sure.

        //  If we can't read the first 2 digits, we're stuck.
        if (odd_buckets[0] < 0 || odd_buckets[1] < 0){
            return -1;
        }

        //  If we can't read the 3rd digit, then assume it's 5.
        if (odd_buckets[2] < 0){
            odd_buckets[2] = 5;
        }

        return odd_buckets[0] * 100 + odd_buckets[1] * 10 + odd_buckets[2];
    }else{
        //  2 digits only.
        if (even_buckets[0] < 0 && even_buckets[3] < 0){
            //  If we can't read the first digit, we're stuck.
            if (even_buckets[1] < 0){
                return -1;
            }
            //  If we can't read the 2nd digit, then assume it's 5.
            if (even_buckets[2] < 0){
                even_buckets[2] = 5;
            }
            return even_buckets[1] * 10 + even_buckets[2];
        }

        //  Now we know it's 4 digits for sure.

        //  If we can't read either of the first 2 digits, assume they are 1.
        if (even_buckets[0] < 0){
            even_buckets[0] = 1;
        }
        if (even_buckets[1] < 0){
            even_buckets[1] = 1;
        }

        //  If we can't read either of the last digits, assume they are 5.
        if (even_buckets[2] < 0){
            even_buckets[2] = 5;
        }
        if (even_buckets[3] < 0){
            even_buckets[3] = 5;
        }

        return even_buckets[0] * 1000 + even_buckets[1] * 100 + even_buckets[2] * 10 + even_buckets[3];
    }

//    return ret;
}





}
}
}
