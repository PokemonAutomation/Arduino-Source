/*  Flag Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonLA_FlagDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels::Waterfill;


void FlagMatcher_make_template(){
    ImageRGB32 image("Flag-Original.png");
    image = image.scale_to(image.width() / 2, image.height() / 2);
    uint32_t* ptr = image.data();
    size_t words = image.bytes_per_row() / sizeof(uint32_t);
    for (size_t r = 0; r < image.height(); r++){
        for (size_t c = 0; c < image.width(); c++){
            uint32_t& pixel = ptr[r * words + c];
            Color color(pixel);
            uint32_t red = color.red();
            uint32_t green = color.green();
            uint32_t blue = color.blue();
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
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            m_matcher.image_template(),
            128, 255,
            128, 255,
            128, 255
        );
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);
        if (objects.size() != 2){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Failed to find exactly 2 objects in resource.",
                m_path
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
            Color(0xffe0e0e0),
            Color(0xfff0f0f0),
        }
    )
{}
void FlagDetector::process_object(const ImageViewRGB32& image, const WaterfillObject& object){
//    cout << "FlagDetector::process_object()" << endl;
    if (object.area < 50){
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

//    static int count = 0;
//    extract_box_reference(image, object).save("test-" + std::to_string(count++) + ".png");

//    cout << "left" << endl;
    if (FlagMatcher::left().matches(object_box, image, object)){
//        static int count = 0;
//        extract_box_reference(image, object_box).save("test-left-" + std::to_string(count++) + ".png");
        m_left.emplace_back(object_box);
    }
//    cout << "right" << endl;
    if (FlagMatcher::right().matches(object_box, image, object)){
//        static int count = 0;
//        extract_box_reference(image, object_box).save("test-right-" + std::to_string(count++) + ".png");
        m_right.emplace_back(object_box);
    }

//    cout << "left  = " << m_left.size() << endl;
//    cout << "right = " << m_right.size() << endl;
}
void FlagDetector::finish(const ImageViewRGB32& image){
#if 0
    cout << "left  = " << m_left.size() << endl;
    cout << "right = " << m_right.size() << endl;

    for (const ImagePixelBox& box : m_left){
        static int count = 0;
        extract_box_reference(image, box).save("test-left-" + std::to_string(count++) + ".png");
    }
    for (const ImagePixelBox& box : m_right){
        static int count = 0;
        extract_box_reference(image, box).save("test-right-" + std::to_string(count++) + ".png");
    }
#endif

    //  Merge left/right parts.
    for (auto iter0 = m_left.begin(); iter0 != m_left.end();){
        double height = (double)iter0->height();
        double width = (double)iter0->width();
        bool removed = false;
        for (auto iter1 = m_right.begin(); iter1 != m_right.end(); ++iter1){
            double height_ratio = height / iter1->height();
            if (height_ratio < 0.8 || height_ratio > 1.2){
//                cout << "bad height ratio: " << height_ratio << endl;
                continue;
            }
            double width_ratio = width / iter1->width();
            if (width_ratio < 0.8 || width_ratio > 1.2){
//                cout << "bad width ratio: " << width_ratio << endl;
                continue;
            }

            double horizontal_offset = ((ptrdiff_t)iter0->min_x - (ptrdiff_t)iter1->min_x) / width;
            if (std::abs(horizontal_offset) > 0.1){
//                cout << "bad horizontal offset: " << horizontal_offset << endl;
                continue;
            }

            double vertical_offset = ((ptrdiff_t)iter0->min_y - (ptrdiff_t)iter1->min_y) / height;
            if (std::abs(vertical_offset) > 0.1){
//                cout << "bad vertical offset: " << vertical_offset << endl;
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
//    cout << "m_detections = " << m_detections.size() << endl;
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


std::pair<double, int> read_digit(const ImageViewRGB32& image, const WaterfillObject& object){
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
//    cout << best_rmsd << endl;
    if (best_rmsd > 100){
        best_digit = -1;
    }
    return {best_rmsd, best_digit};
}



int read_flag_distance(const ImageViewRGB32& screen, double flag_x, double flag_y){
    ImageFloatBox box(flag_x - 0.025, flag_y - 0.055, 0.045, 0.025);
    ImageViewRGB32 image = extract_box_reference(screen, box);
//    image.save("test.png");

    size_t width = image.width();
    size_t height = image.height();

    //  Detect all the digits.
    struct Hit{
        size_t min_x;
        size_t max_x;
        double mid_x;
        double rmsd;
        int digit;
    };
    std::multimap<size_t, Hit> hits;

    {
        std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
            image,
            {
                {0xff808080, 0xffffffff},
                {0xff909090, 0xffffffff},
                {0xffa0a0a0, 0xffffffff},
                {0xffb0b0b0, 0xffffffff},
                {0xffc0c0c0, 0xffffffff},
                {0xffd0d0d0, 0xffffffff},
                {0xffe0e0e0, 0xffffffff},
                {0xfff0f0f0, 0xffffffff},
            }
        );

        double inv_width = 0.5 / width;

        std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
        for (PackedBinaryMatrix& matrix : matrices){
//            cout << (int)filters[c].matrix.type() << endl;
            session->set_source(matrix);
            auto finder = session->make_iterator(30);
            WaterfillObject object;
            while (finder->find_next(object, false)){
                //  Skip anything that touches the edge.
                if (object.min_x == 0 || object.min_y == 0 ||
                    object.max_x + 1 == width || object.max_y + 1 == height
                ){
                    continue;
                }

//                static int c = 0;
//                extract_box_reference(image, object).save("image-" + std::to_string(c++) + ".png");

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
    }
    if (hits.empty()){
        return -1;
    }

#if 0
    for (const auto& item : hits){
        cout << item.first << " : " << item.second.min_x << " - " << item.second.max_x << " : " << item.second.digit << endl;
    }
#endif


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
