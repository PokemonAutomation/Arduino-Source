/*  Tera Code Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageManip.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "PokemonSV_TeraCodeReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


char read_5S(const PackedBinaryMatrix& matrix, char OCR_result){
    //  5 and S are commonly misread as each other. Here we try an extra step to
    //  distinguish them.

//    static int count = 0;
//    image.save("testB-" + std::to_string(count++) + ".png");

    size_t width = matrix.width();
    size_t height = matrix.height();

    for (size_t row = 0; row < height; row++){
        size_t top_black_pixels = 0;
        for (size_t c = 0; c < width; c++){
            if (matrix.get(c, 0)){
                top_black_pixels++;
            }
        }
        double ratio = (double)top_black_pixels / width;
    //    cout << "top_black_pixels = " << top_black_pixels << ", " << ratio << endl;
        if (ratio > 0.70){
            return '5';
        }
        if (ratio > 0.10){
            return 'S';
        }
    }

    return OCR_result;
}


struct WaterfillOCRResult{
    PackedBinaryMatrix matrix;
    std::string ocr;
};


std::vector<WaterfillOCRResult> waterfill_OCR(
    AsyncDispatcher& dispatcher,
    const ImageViewRGB32& image,
    uint32_t threshold
){
    using namespace Kernels::Waterfill;

    //  Direct OCR is unreliable. Instead, we will waterfill each character
    //  to isolate them, then OCR them individually.

    ImageRGB32 filtered = to_blackwhite_rgb32_range(image, 0xff000000, threshold, true);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff000000, threshold);

    std::map<size_t, WaterfillObject> map;
    {
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(20);
        WaterfillObject object;
        while (map.size() < 16 && iter->find_next(object, true)){
            map.emplace(object.min_x, std::move(object));
        }
    }

    std::vector<WaterfillObject> objects;
    for (auto& item : map){
        objects.emplace_back(std::move(item.second));
    }

    std::vector<WaterfillOCRResult> ret(objects.size());
    dispatcher.run_in_parallel(
        0, objects.size(),
        [&](size_t index){
            WaterfillObject& object = objects[index];
            ImageRGB32 cropped = extract_box_reference(filtered, object).copy();
            PackedBinaryMatrix tmp(object.packed_matrix());
            filter_by_mask(tmp, cropped, Color(0xffffffff), true);
            ImageRGB32 padded = pad_image(cropped, cropped.width(), 0xffffffff);
            ret[index].matrix = std::move(tmp);
            ret[index].ocr = OCR::ocr_read(Language::English, padded);
        }
    );

#if 0
    static size_t count = 0;
    for (size_t c = 0; c < objects.size(); c++){
//        cout << item.ocr << endl;
        ImageRGB32 cropped = extract_box_reference(filtered, objects[c]).copy();
        filter_by_mask(ret[c].matrix, cropped, Color(0xffffffff), true);
        ImageRGB32 padded = pad_image(cropped, cropped.width(), 0xffffffff);
        padded.save("letter-" + std::to_string(count) + ".png");
        count++;
    }
#endif

    return ret;
}


int16_t read_raid_timer(Logger& logger, AsyncDispatcher& dispatcher, const ImageViewRGB32& image){
    std::vector<WaterfillOCRResult> characters = waterfill_OCR(dispatcher, image, 0xff7f7f7f);

//    cout << "map.size() = " << map.size() << endl;
//    for (auto& item : map){
//        cout << item.second << endl;
//    }

    static const std::map<char, char> SUBSTITUTIONS{
        {'I', '1'},
        {'i', '1'},
        {'l', '1'},
        {'O', '0'},
        {'S', '5'},
        {'s', '5'},
        {'/', '7'},
        {']', '1'},
    };

    std::string raw;
    std::string normalized;
    for (const auto& item : characters){
        const std::string& ocr = item.ocr;
        if (ocr.empty()){
            continue;
        }
        if ((uint8_t)ocr.back() < (uint8_t)32){
            raw += ocr.substr(0, ocr.size() - 1);
        }else{
            raw += ocr;
        }

        char ch = ocr[0];

        //  Character substitution.
        auto iter = SUBSTITUTIONS.find(ch);
        if (iter != SUBSTITUTIONS.end()){
            ch = iter->second;
        }

        if ('0' <= ch && ch <= '9'){
            normalized += ch;
        }
    }

    std::string log = "Timer OCR: \"" + raw + "\" -> \"" + normalized + "\"";
    if (normalized.size() != 3){
        logger.log(log, COLOR_RED);
        return -1;
    }

    logger.log(log, COLOR_BLUE);

    int16_t ret = 60 * (normalized[0] - '0');
    ret += 10 * (normalized[1] - '0');
    ret += normalized[2] - '0';

    return ret;
}


std::string read_raid_code(Logger& logger, AsyncDispatcher& dispatcher, const ImageViewRGB32& image){
    std::vector<uint32_t> filters{
        0xff5f5f5f,
        0xff7f7f7f,
    };

    for (uint32_t filter : filters){
        std::vector<WaterfillOCRResult> characters = waterfill_OCR(dispatcher, image, filter);

        static const std::map<char, char> SUBSTITUTIONS{
            {'I', '1'},
            {'i', '1'},
            {'l', '1'},
            {'O', '0'},
            {'Z', 'S'},
            {'\\', 'V'},
            {'/', '7'},
            {']', '1'},
            {')', 'J'},
            {'(', 'K'},
        };

        bool contains_letters = false;

        std::string raw;
        std::string normalized;
        for (const auto& item : characters){
            const std::string& ocr = item.ocr;
            if (ocr.empty()){
                continue;
            }
            if ((uint8_t)ocr.back() < (uint8_t)32){
                raw += ocr.substr(0, ocr.size() - 1);
            }else{
                raw += ocr;
            }

            char ch = ocr[0];

            //  Upper case.
            if ('a' <= ch && ch <= 'z'){
                ch -= 'a' - 'A';
            }

            //  Character substitution.
            auto iter = SUBSTITUTIONS.find(ch);
            if (iter != SUBSTITUTIONS.end()){
                ch = iter->second;
            }

            contains_letters |= 'A' <= ch && ch <= 'Z';

            //  Distinguish 5 and S.
            switch (ch){
            case '5':
            case 'S':
                ch = read_5S(item.matrix, ch);
            }

            normalized += ch;
        }

        std::string log = "Code OCR: \"" + raw + "\" -> \"" + normalized + "\"";
        size_t length = normalized.size();
        if ((contains_letters && length == 6) ||
            (!contains_letters && (length == 4 || length == 8))
        ){
            logger.log(log, COLOR_BLUE);
            return normalized;
        }
        logger.log(log, COLOR_RED);
    }

    return "";
}



}
}
}
