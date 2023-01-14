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

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


char read_5S(const ImageViewRGB32& image, char OCR_result){
    //  5 and S are commonly misread as each other. Here we try an extra step to
    //  distinguish them.

//    static int count = 0;
//    image.save("testB-" + std::to_string(count++) + ".png");

    if (image.height() < 2){
        return OCR_result;
    }

    size_t width = image.width();

    size_t top_black_pixels = 0;
    for (size_t c = 0; c < width; c++){
        uint32_t pixel = image.pixel(c, 0);
        uint32_t sum = pixel & 0xff;
        sum += (pixel >> 16) & 0xff;
        sum += (pixel >> 8) & 0xff;
        if (sum < 400){
            top_black_pixels++;
        }
    }
    double ratio = (double)top_black_pixels / width;
//    cout << "top_black_pixels = " << top_black_pixels << ", " << ratio << endl;
    if (ratio > 0.70){
        return '5';
    }

#if 0
    top_black_pixels = 0;
    for (size_t c = 0; c < width; c++){
        uint32_t pixel = image.pixel(c, 1);
        uint32_t sum = pixel & 0xff;
        sum += (pixel >> 16) & 0xff;
        sum += (pixel >> 8) & 0xff;
        if (sum < 400){
            top_black_pixels++;
        }
    }
    ratio = (double)top_black_pixels / width;
//    cout << "top_black_pixels = " << top_black_pixels << ", " << ratio << endl;
    if (ratio > 0.70){
        return '5';
    }
#endif

    return 'S';
}


struct WaterfillOCRResult{
    ImagePixelBox box;
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

//    uint32_t THRESHOLD = 0xff5f5f5f;
//    uint32_t THRESHOLD = 0xff7f7f7f;

    ImageRGB32 filtered = to_blackwhite_rgb32_range(image, 0xff000000, threshold, true);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff000000, threshold);

    std::map<size_t, ImagePixelBox> boxes;
    {
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(20);
        WaterfillObject object;
        while (iter->find_next(object, false)){
            boxes.emplace(object.min_x, object);
        }
    }

    std::vector<WaterfillOCRResult> ret;
    for (auto& item : boxes){
        ret.emplace_back(WaterfillOCRResult{item.second, std::string()});
    }

#if 0
    for (WaterfillOCRResult& item : ret){
        ImageViewRGB32 cropped = extract_box_reference(filtered, item.box);
        ImageRGB32 padded = pad_image(cropped, cropped.width(), 0xffffffff);
        item.ocr = OCR::ocr_read(Language::English, padded);
    }
#else
    dispatcher.run_in_parallel(
        0, ret.size(),
        [&](size_t index){
            ImageViewRGB32 cropped = extract_box_reference(filtered, ret[index].box);
            ImageRGB32 padded = pad_image(cropped, cropped.width(), 0xffffffff);
            ret[index].ocr = OCR::ocr_read(Language::English, padded);
        }
    );
#endif

#if 0
    size_t c = 0;
    for (auto& item : ret){
        cout << item.ocr << endl;
        ImageViewRGB32 cropped = extract_box_reference(filtered, ret[c].box);
        cropped.save("letter-" + std::to_string(c) + ".png");
        c++;
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
                ch = read_5S(extract_box_reference(image, item.box), ch);
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
